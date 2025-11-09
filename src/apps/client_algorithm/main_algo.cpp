/*
 * =============================================================================
 * Project:      Beacon
 * Application:  client_algorithm
 * File:         main_algo.cpp
 * Purpose:      Trading algorithm with NASDAQ OUCH v5.0 order entry protocol.
 *               
 *               Functionality:
 *               - Receives 32-byte market data messages from market data server
 *               - Extracts symbol and price information from market data
 *               - Generates trading signals based on market data flow
 *               - Sends 64-byte OUCH v5.0 order entry messages to matching engine
 *               - Receives 32-byte execution reports in dedicated listener thread
 *               
 *               Threading Model:
 *               - Main thread: Market data reception + order transmission
 *               - Execution thread: Execution report reception
 *               - Atomic counters for thread-safe statistics
 *               
 *               Usage:
 *               ./client_algo algo <md_host> <md_port> <ex_host> <ex_port> <num_messages>
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <iostream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <random>

// NASDAQ OUCH v5.0 Enter Order Message (64 bytes)
struct OuchEnterOrderMessage {
    uint64_t clientOrderId;      // 8 bytes: unique client order ID
    char symbol[8];              // 8 bytes: padded stock symbol
    uint32_t shares;             // 4 bytes: number of shares
    uint32_t price;              // 4 bytes: price in 1/10000 dollars
    char side;                   // 1 byte: 'B'=buy, 'S'=sell
    char timeInForce;            // 1 byte: '0'=Day, '3'=IOC, '4'=FOK
    char orderType;              // 1 byte: 'O'=order
    char capacity;               // 1 byte: 'A'=agency, 'P'=principal
    uint16_t reserved;           // 2 bytes: padding/future use
    char _padding[34];           // 34 bytes: padding to 64 bytes
};

static_assert(sizeof(OuchEnterOrderMessage) == 64, "OuchEnterOrderMessage must be 64 bytes");

// Execution report from exchange (32 bytes) - for now, keeping simple format
struct ExecutionReport {
    uint32_t msgType;      // 3=Execution Report
    uint32_t orderId;
    char symbol[8];
    uint32_t execQty;
    uint32_t execPrice;
    uint32_t status;       // 0=New, 1=PartialFill, 2=Filled
    uint32_t padding[2];
};

std::atomic<bool> g_running(true);
std::atomic<uint64_t> g_marketDataCount(0);
std::atomic<uint64_t> g_ordersSent(0);
std::atomic<uint64_t> g_fillsReceived(0);

// Connect to matching engine for order entry
int connectToExchange(const char* host, uint16_t port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }
    
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    
    if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    
    // Enable TCP_NODELAY for low latency
    int noDelay = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));
    
    return sock;
}

// Thread: Listen for execution reports from exchange
void listenForExecutions(int exchangeSocket) {
    ExecutionReport exec;
    
    while (g_running) {
        ssize_t bytesRead = recv(exchangeSocket, &exec, sizeof(exec), 0);
        
        if (bytesRead == 0) {
            std::cout << "\n[EXCHANGE] Connection closed\n";
            break;
        }
        
        if (bytesRead < 0) {
            if (g_running) {
                std::cerr << "\n[EXCHANGE ERROR] Receive failed\n";
            }
            break;
        }
        
        if (bytesRead == sizeof(exec) && exec.msgType == 3) {
            g_fillsReceived++;
            
            char symbol[9] = {0};
            std::memcpy(symbol, exec.symbol, 8);
            double price = exec.execPrice / 10000.0;
            
            std::cout << "[FILL #" << g_fillsReceived << "] ";
            std::cout << "OrderID " << exec.orderId << ": ";
            std::cout << exec.execQty << " " << symbol;
            std::cout << " @ $" << std::fixed << std::setprecision(2) << price << "\n";
        }
    }
}

// Simple algo: Send random orders every N market data messages
void runAlgoWithMarketData(const char* mdHost, int mdPort, 
                           const char* exHost, int exPort,
                           int orderFrequency) {
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "          CLIENT ALGORITHM - MARKET DATA + ORDERS              \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "Market Data: " << mdHost << ":" << mdPort << "\n";
    std::cout << "Exchange:    " << exHost << ":" << exPort << "\n";
    std::cout << "Strategy:    Send order every " << orderFrequency << " market data messages\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";
    
    // Connect to market data feed (TCP)
    int mdSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in mdAddr;
    std::memset(&mdAddr, 0, sizeof(mdAddr));
    mdAddr.sin_family = AF_INET;
    mdAddr.sin_port = htons(mdPort);
    inet_pton(AF_INET, mdHost, &mdAddr.sin_addr);
    
    std::cout << "Connecting to market data feed...\n";
    if (connect(mdSocket, reinterpret_cast<sockaddr*>(&mdAddr), sizeof(mdAddr)) < 0) {
        std::cerr << "ERROR: Failed to connect to market data: " << strerror(errno) << "\n";
        return;
    }
    std::cout << "✓ Connected to market data\n";
    
    // Connect to exchange for order entry
    std::cout << "Connecting to exchange...\n";
    int exSocket = connectToExchange(exHost, exPort);
    if (exSocket < 0) {
        std::cerr << "ERROR: Failed to connect to exchange: " << strerror(errno) << "\n";
        close(mdSocket);
        return;
    }
    std::cout << "✓ Connected to exchange\n\n";
    std::cout << "Running algorithm (Press Ctrl+C to stop)...\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";
    
    // Start thread to listen for execution reports
    std::thread execThread(listenForExecutions, exSocket);
    
    // Random number generator for orders
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sideDist(1, 2);  // 1=Buy, 2=Sell
    std::uniform_int_distribution<> qtyDist(10, 100);
    std::uniform_int_distribution<> priceDist(15000000, 16000000);  // $1500-$1600
    
    constexpr size_t MSG_SIZE = 32;
    char mdBuffer[MSG_SIZE];
    uint64_t nextOrderId = 1;
    
    // Read market data and send orders
    while (g_running) {
        ssize_t bytesRead = recv(mdSocket, mdBuffer, MSG_SIZE, 0);
        
        if (bytesRead == 0) {
            std::cout << "\n✓ Market data feed closed\n";
            break;
        }
        
        if (bytesRead < 0) {
            std::cerr << "\nERROR: Market data receive failed\n";
            break;
        }
        
        g_marketDataCount++;
        
        // Print progress every 1000 messages
        if (g_marketDataCount % 1000 == 0) {
            std::cout << "[MD] Received " << g_marketDataCount << " messages | ";
            std::cout << "Orders sent: " << g_ordersSent << " | ";
            std::cout << "Fills: " << g_fillsReceived << "\n";
        }
        
        // Send order every N messages
        if (g_marketDataCount % orderFrequency == 0) {
            OuchEnterOrderMessage order;
            std::memset(&order, 0, sizeof(order));
            
            order.clientOrderId = nextOrderId++;
            
            // Extract symbol from market data message (bytes 8-15)
            std::memcpy(order.symbol, mdBuffer + 8, 8);
            
            // Random order parameters
            int sideRand = sideDist(gen);
            order.side = (sideRand == 1) ? 'B' : 'S';
            order.shares = qtyDist(gen);
            order.price = priceDist(gen);
            
            // OUCH protocol fields
            order.timeInForce = '0';  // Day order
            order.orderType = 'O';    // Regular order
            order.capacity = 'A';     // Agency
            order.reserved = 0;
            
            ssize_t sent = send(exSocket, &order, sizeof(order), 0);
            if (sent == sizeof(order)) {
                g_ordersSent++;
                
                char symbol[9] = {0};
                std::memcpy(symbol, order.symbol, 8);
                const char* sideStr = (order.side == 'B') ? "BUY" : "SELL";
                double price = order.price / 10000.0;
                
                std::cout << "[ORDER #" << g_ordersSent << "] ";
                std::cout << sideStr << " " << order.shares << " " << symbol;
                std::cout << " @ $" << std::fixed << std::setprecision(2) << price;
                std::cout << " [OUCH OrderID: " << order.clientOrderId << "]\n";
            }
        }
    }
    
    g_running = false;
    close(mdSocket);
    close(exSocket);
    execThread.join();
    
    std::cout << "\n═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    ALGORITHM COMPLETE                         \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "Market Data Messages: " << g_marketDataCount << "\n";
    std::cout << "Orders Sent:          " << g_ordersSent << "\n";
    std::cout << "Fills Received:       " << g_fillsReceived << "\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
}

void printUsage(const char* progName) {
    std::cout << "Usage:\n";
    std::cout << "  " << progName << " algo <md_host> <md_port> <ex_host> <ex_port> [order_freq]\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  md_host      - Market data playback host\n";
    std::cout << "  md_port      - Market data playback port\n";
    std::cout << "  ex_host      - Exchange matching engine host\n";
    std::cout << "  ex_port      - Exchange matching engine port\n";
    std::cout << "  order_freq   - Send order every N messages (default: 10)\n\n";
    std::cout << "Example:\n";
    std::cout << "  " << progName << " algo 127.0.0.1 12345 127.0.0.1 54321 10\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string mode = argv[1];
    
    if (mode == "algo") {
        if (argc < 6) {
            std::cerr << "ERROR: Not enough arguments for algo mode\n\n";
            printUsage(argv[0]);
            return 1;
        }
        
        const char* mdHost = argv[2];
        int mdPort = std::atoi(argv[3]);
        const char* exHost = argv[4];
        int exPort = std::atoi(argv[5]);
        int orderFreq = (argc > 6) ? std::atoi(argv[6]) : 10;
        
        runAlgoWithMarketData(mdHost, mdPort, exHost, exPort, orderFreq);
    }
    else {
        std::cerr << "ERROR: Invalid mode '" << mode << "'\n\n";
        printUsage(argv[0]);
        return 1;
    }
    
    return 0;
}
