/*
 * =============================================================================
 * Project:      Beacon
 * Application:  test_pillar
 * File:         test_pillar.cpp
 * Purpose:      Standalone test client for NYSE Pillar Gateway v3.2 protocol.
 *               
 *               Functionality:
 *               - Sends 64-byte NYSE Pillar order entry messages
 *               - Tests with NYSE-listed equity symbols (AAPL, MSFT, GOOGL, etc.)
 *               - Synchronous request-response pattern: send order → wait for fill
 *               - Random order generation: side, quantity (10-100 shares), price
 *               - Validates protocol integration with multi-protocol matching engine
 *               
 *               Message Format:
 *               - Order Type: 'L' (Limit orders)
 *               - Time In Force: '0' (Day orders)
 *               - Capacity: 'A' (Agency)
 *               - Price encoding: 1/10000 dollars (e.g., 1,502,500 = $150.25)
 *               
 *               Usage:
 *               ./test_pillar <host> <port> [num_orders]
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

// NYSE Pillar Gateway v3.2 Order Entry Message (64 bytes)
struct PillarOrderEntryMessage {
    uint64_t clientOrderId;
    char symbol[8];
    uint32_t shares;
    uint32_t price;
    char side;
    char orderType;
    char tif;
    char reserved1;
    uint16_t reserved2;
    char _padding[34];
};

static_assert(sizeof(PillarOrderEntryMessage) == 64, "PillarOrderEntryMessage must be 64 bytes");

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
    std::cout << "              NYSE PILLAR PROTOCOL TEST CLIENT                 \n";
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
    std::uniform_int_distribution<> qtyDist(10, 100);
    std::uniform_int_distribution<> priceDist(15000000, 16000000);

    const char* symbols[] = {"AAPL    ", "MSFT    ", "GOOGL   ", "AMZN    ", "TSLA    "};
    std::uniform_int_distribution<> symbolDist(0, 4);

    // Send orders
    for (int i = 1; i <= numOrders; i++) {
        PillarOrderEntryMessage order;
        std::memset(&order, 0, sizeof(order));

        order.clientOrderId = i;
        std::memcpy(order.symbol, symbols[symbolDist(gen)], 8);
        order.shares = qtyDist(gen);
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

        std::cout << "[PILLAR ORDER #" << i << "] ";
        std::cout << sideStr << " " << order.shares << " " << symbol;
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
