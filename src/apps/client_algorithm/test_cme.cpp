/*
 * =============================================================================
 * Project:      Beacon
 * Application:  test_cme
 * File:         test_cme.cpp
 * Purpose:      Standalone test client for CME iLink 3 protocol (simplified).
 *               
 *               Functionality:
 *               - Sends 64-byte CME futures order entry messages
 *               - Tests with CME futures contracts (ES, NQ, YM, RTY, ZN)
 *               - Symbol format: [ROOT][MONTH][YEAR] (e.g., ESZ4, NQH5)
 *               - Month codes: F,G,H,J,K,M,N,Q,U,V,X,Z
 *               - Synchronous request-response: send order → wait for fill
 *               - Futures-appropriate sizing: 1-10 contracts, $4500-$4600 range
 *               
 *               Message Format:
 *               - Order Type: 'L' (Limit orders)
 *               - Time In Force: '0' (Day orders)
 *               - Capacity: 'P' (Principal trading)
 *               - Price encoding: 1/10000 dollars (e.g., 45,500,000 = $4,550.00)
 *               
 *               Usage:
 *               ./test_cme <host> <port> [num_orders]
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <iostream>
#include <iomanip>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <random>

// CME Order Entry Message (64 bytes)
struct CmeOrderEntryMessage {
    uint64_t clientOrderId;      // 8 bytes: unique client order ID
    char symbol[8];              // 8 bytes: padded instrument symbol
    uint32_t quantity;           // 4 bytes: order quantity
    uint32_t price;              // 4 bytes: price in 1/10000 dollars
    char side;                   // 1 byte: 'B'=buy, 'S'=sell
    char orderType;              // 1 byte: 'L'=limit, 'M'=market
    char tif;                    // 1 byte: time in force
    char reserved1;              // 1 byte: reserved/future use
    uint16_t reserved2;          // 2 bytes: padding/future use
    char _padding[34];           // 34 bytes
};

static_assert(sizeof(CmeOrderEntryMessage) == 64, "CmeOrderEntryMessage must be 64 bytes");

// Execution report from exchange (32 bytes)
struct ExecutionReport {
    uint32_t msgType;
    uint32_t orderId;
    char symbol[8];
    uint32_t execQty;
    uint32_t execPrice;
    uint32_t status;
    uint32_t padding[2];
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port> [num_orders]\n";
        std::cerr << "Example: " << argv[0] << " 127.0.0.1 54321 10\n";
        return 1;
    }

    const char* host = argv[1];
    uint16_t port = std::atoi(argv[2]);
    int numOrders = (argc > 3) ? std::atoi(argv[3]) : 10;

    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "                 CME PROTOCOL TEST CLIENT                      \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "Matching Engine: " << host << ":" << port << "\n";
    std::cout << "Orders to send:  " << numOrders << "\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";

    // Connect to matching engine
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "ERROR: Failed to create socket\n";
        return 1;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    std::cout << "Connecting to matching engine...\n";
    if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "ERROR: Failed to connect: " << strerror(errno) << "\n";
        close(sock);
        return 1;
    }

    int noDelay = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));
    std::cout << "✓ Connected\n\n";

    // Random number generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sideDist(0, 1);
    std::uniform_int_distribution<> qtyDist(1, 10);  // Futures contracts
    std::uniform_int_distribution<> priceDist(45000000, 46000000);  // ES futures ~$4500-$4600

    const char* symbols[] = {"ESZ4    ", "NQZ4    ", "YMZ4    ", "RTY4    ", "ZNZ4    "};
    std::uniform_int_distribution<> symbolDist(0, 4);

    // Send orders
    for (int i = 1; i <= numOrders; i++) {
        CmeOrderEntryMessage order;
        std::memset(&order, 0, sizeof(order));

        order.clientOrderId = i;
        std::memcpy(order.symbol, symbols[symbolDist(gen)], 8);
        order.quantity = qtyDist(gen);
        order.price = priceDist(gen);
        order.side = sideDist(gen) ? 'B' : 'S';
        order.orderType = 'L';  // Limit order
        order.tif = '0';        // Day order
        order.reserved1 = 0;
        order.reserved2 = 0;

        ssize_t sent = send(sock, &order, sizeof(order), 0);
        if (sent != sizeof(order)) {
            std::cerr << "ERROR: Failed to send order " << i << "\n";
            continue;
        }

        char symbol[9] = {0};
        std::memcpy(symbol, order.symbol, 8);
        const char* sideStr = (order.side == 'B') ? "BUY" : "SELL";
        double price = order.price / 10000.0;

        std::cout << "[CME ORDER #" << i << "] ";
        std::cout << sideStr << " " << order.quantity << " " << symbol;
        std::cout << " @ $" << std::fixed << std::setprecision(2) << price;
        std::cout << " [DAY, L] (OrderID: " << order.clientOrderId << ")\n";

        // Receive execution report
        ExecutionReport exec;
        ssize_t received = recv(sock, &exec, sizeof(exec), 0);
        if (received == sizeof(exec) && exec.msgType == 3) {
            std::cout << "  ✓ FILLED " << exec.execQty << " @ $" 
                      << (exec.execPrice / 10000.0) << "\n\n";
        } else {
            std::cerr << "  ✗ No execution report received\n\n";
        }

        usleep(100000);  // 100ms delay between orders
    }

    close(sock);

    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "                      TEST COMPLETE                            \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";

    return 0;
}
