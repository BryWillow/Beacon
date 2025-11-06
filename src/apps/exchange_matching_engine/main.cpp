/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_matching_engine
 * File:         main.cpp
 * Purpose:      Multi-protocol electronic matching engine supporting:
 *               - NASDAQ OUCH v5.0 order entry
 *               - NYSE Pillar Gateway v3.2 order entry
 *               - CME iLink 3 order entry (simplified)
 *               
 *               Features:
 *               - Auto-detection of protocol from message content
 *               - Explicit protocol mode configuration
 *               - Protocol-agnostic order normalization via adapters
 *               - Immediate execution model (simplified, no order book)
 *               - TCP server for client connections
 *               - 32-byte execution report responses
 *               
 *               Usage:
 *               ./exchange_matching_engine <port> [protocol]
 *               protocol: "auto", "ouch", "pillar", or "cme"
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <iostream>
#include <iomanip>
#include <cstring>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include "protocol_adapters.h"

// Execution report to client (32 bytes)
struct ExecutionReport {
    uint32_t msgType;      // 3=Execution Report
    uint32_t orderId;
    char symbol[8];
    uint32_t execQty;
    uint32_t execPrice;
    uint32_t status;       // 0=New, 1=PartialFill, 2=Filled, 3=Cancelled
    uint32_t padding[2];
};

class MatchingEngine {
public:
    MatchingEngine(uint16_t port, const std::string& protocol = "auto") 
        : _port(port), _protocol(protocol), _running(false), _orderCount(0), _fillCount(0) {}
    
    void start() {
        // Create TCP socket
        _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (_listenSocket < 0) {
            std::cerr << "ERROR: Failed to create socket: " << strerror(errno) << "\n";
            return;
        }
        
        // Allow address reuse
        int reuse = 1;
        setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        
        // Bind to port
        sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(_port);
        
        if (bind(_listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
            std::cerr << "ERROR: Failed to bind: " << strerror(errno) << "\n";
            close(_listenSocket);
            return;
        }
        
        if (listen(_listenSocket, 5) < 0) {
            std::cerr << "ERROR: Failed to listen: " << strerror(errno) << "\n";
            close(_listenSocket);
            return;
        }
        
        std::cout << "═══════════════════════════════════════════════════════════════\n";
        std::cout << "  [1] <OUCH> Exchange Matching Engine Starting...              \n";
        std::cout << "═══════════════════════════════════════════════════════════════\n";
        std::cout << "      Port:     " << _port << "\n";
        std::cout << "      Protocol: " << _protocol << " (OUCH/Pillar/CME)\n";
        std::cout << "═══════════════════════════════════════════════════════════════\n\n";
        
        _running = true;
        
        std::cout << "═══════════════════════════════════════════════════════════════\n";
        std::cout << "  [2] <OUCH> Matching Engine Started                           \n";
        std::cout << "═══════════════════════════════════════════════════════════════\n";
        std::cout << "      Waiting for client connections...\n\n";
        
        while (_running) {
            sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientSocket = accept(_listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
            
            if (clientSocket < 0) {
                if (_running) {
                    std::cerr << "ERROR: Accept failed: " << strerror(errno) << "\n";
                }
                continue;
            }
            
            char clientIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
            std::cout << "✓ Client connected from " << clientIp << ":" << ntohs(clientAddr.sin_port) << "\n\n";
            
            // Disable Nagle's algorithm for low latency
            int noDelay = 1;
            setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));
            
            // Handle this client in a separate thread
            std::thread clientThread(&MatchingEngine::handleClient, this, clientSocket);
            clientThread.detach();
        }
        
        close(_listenSocket);
    }
    
    void stop() {
        _running = false;
        if (_listenSocket >= 0) {
            close(_listenSocket);
        }
    }
    
private:
    void handleClient(int clientSocket) {
        char buffer[64];  // All protocols use 64-byte messages
        
        while (_running) {
            ssize_t bytesRead = recv(clientSocket, buffer, 64, 0);
            
            if (bytesRead == 0) {
                std::cout << "Client disconnected\n";
                break;
            }
            
            if (bytesRead < 0) {
                std::cerr << "ERROR: Receive failed: " << strerror(errno) << "\n";
                break;
            }
            
            if (bytesRead != 64) {
                std::cerr << "ERROR: Expected 64 bytes, got " << bytesRead << " bytes\n";
                continue;
            }
            
            // Decode based on protocol mode
            NormalizedOrder order = decodeOrder(buffer);
            
            // Process normalized order
            _orderCount++;
            processOrder(order, clientSocket);
        }
        
        close(clientSocket);
    }
    
    NormalizedOrder decodeOrder(const char* buffer) {
        // Auto-detect protocol or use configured protocol
        if (_protocol == "auto") {
            // Simple heuristic: Check orderType field (byte 22)
            // OUCH: 'O', Pillar: varies, CME: 'L'/'M'
            char orderType = buffer[22];
            
            if (orderType == 'O') {
                // OUCH protocol
                const OuchEnterOrderMessage* ouch = reinterpret_cast<const OuchEnterOrderMessage*>(buffer);
                return ProtocolAdapter::decodeOuch(*ouch);
            }
            else if (orderType == 'L' || orderType == 'M') {
                // Could be CME or Pillar - check symbol for futures patterns
                char symbol[9] = {0};
                std::memcpy(symbol, buffer + 8, 8);
                
                // CME futures typically have month codes (ESZ4, NQZ4, etc.)
                if (symbol[2] >= 'F' && symbol[2] <= 'Z' && symbol[3] >= '0' && symbol[3] <= '9') {
                    const CmeOrderEntryMessage* cme = reinterpret_cast<const CmeOrderEntryMessage*>(buffer);
                    return ProtocolAdapter::decodeCME(*cme);
                }
                else {
                    const PillarOrderEntryMessage* pillar = reinterpret_cast<const PillarOrderEntryMessage*>(buffer);
                    return ProtocolAdapter::decodePillar(*pillar);
                }
            }
            else {
                // Default to Pillar
                const PillarOrderEntryMessage* pillar = reinterpret_cast<const PillarOrderEntryMessage*>(buffer);
                return ProtocolAdapter::decodePillar(*pillar);
            }
        }
        else if (_protocol == "ouch") {
            const OuchEnterOrderMessage* ouch = reinterpret_cast<const OuchEnterOrderMessage*>(buffer);
            return ProtocolAdapter::decodeOuch(*ouch);
        }
        else if (_protocol == "pillar") {
            const PillarOrderEntryMessage* pillar = reinterpret_cast<const PillarOrderEntryMessage*>(buffer);
            return ProtocolAdapter::decodePillar(*pillar);
        }
        else if (_protocol == "cme") {
            const CmeOrderEntryMessage* cme = reinterpret_cast<const CmeOrderEntryMessage*>(buffer);
            return ProtocolAdapter::decodeCME(*cme);
        }
        
        // Fallback to OUCH
        const OuchEnterOrderMessage* ouch = reinterpret_cast<const OuchEnterOrderMessage*>(buffer);
        return ProtocolAdapter::decodeOuch(*ouch);
    }
    
    void processOrder(const NormalizedOrder& order, int clientSocket) {
        // Extract symbol (null-terminated)
        char symbol[9] = {0};
        std::memcpy(symbol, order.symbol, 8);
        
        const char* sideStr = (order.side == 'B') ? "BUY" : "SELL";
        const char* protocolStr = ProtocolAdapter::getProtocolName(order.protocol);
        const char* tifStr = ProtocolAdapter::getTifString(order.timeInForce);
        double price = order.price / 10000.0;
        
        std::cout << "[" << protocolStr << " ORDER #" << _orderCount << "] ";
        std::cout << sideStr << " " << order.quantity << " " << symbol;
        std::cout << " @ $" << std::fixed << std::setprecision(2) << price;
        std::cout << " [" << tifStr << ", " << order.capacity << ", " << order.orderType << "]";
        std::cout << " (OrderID: " << order.orderId << ")\n";
        
        // Simulate matching - immediately fill the order
        ExecutionReport exec;
        std::memset(&exec, 0, sizeof(exec));
        
        exec.msgType = 3;  // Execution Report
        exec.orderId = static_cast<uint32_t>(order.orderId);
        std::memcpy(exec.symbol, order.symbol, 8);
        exec.execQty = order.quantity;
        exec.execPrice = order.price;
        exec.status = 2;  // Filled
        
        // Send execution report back to client
        ssize_t sent = send(clientSocket, &exec, sizeof(exec), 0);
        if (sent == sizeof(exec)) {
            _fillCount++;
            std::cout << "  ✓ FILLED " << order.quantity << " @ $" << price << "\n\n";
        } else {
            std::cerr << "  ✗ Failed to send execution report\n\n";
        }
    }
    
    uint16_t _port;
    std::string _protocol;
    int _listenSocket = -1;
    std::atomic<bool> _running;
    std::atomic<uint64_t> _orderCount;
    std::atomic<uint64_t> _fillCount;
};

int main(int argc, char* argv[]) {
    uint16_t port = 54321;
    std::string protocol = "auto";
    
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    if (argc > 2) {
        protocol = argv[2];
        // Validate protocol
        if (protocol != "auto" && protocol != "ouch" && protocol != "pillar" && protocol != "cme") {
            std::cerr << "ERROR: Invalid protocol '" << protocol << "'\n";
            std::cerr << "Valid options: auto, ouch, pillar, cme\n\n";
            std::cout << "Usage: " << argv[0] << " [port] [protocol]\n";
            std::cout << "  port     - Port to listen on (default: 54321)\n";
            std::cout << "  protocol - Protocol mode: auto|ouch|pillar|cme (default: auto)\n\n";
            std::cout << "Examples:\n";
            std::cout << "  " << argv[0] << " 54321 auto     # Auto-detect protocol\n";
            std::cout << "  " << argv[0] << " 54321 ouch     # NASDAQ OUCH only\n";
            std::cout << "  " << argv[0] << " 54321 pillar   # NYSE Pillar only\n";
            std::cout << "  " << argv[0] << " 54321 cme      # CME only\n";
            return 1;
        }
    }
    
    MatchingEngine engine(port, protocol);
    
    // Handle Ctrl+C gracefully
    signal(SIGINT, [](int) {
        std::cout << "\n\nShutting down...\n";
        exit(0);
    });
    
    engine.start();
    
    return 0;
}
