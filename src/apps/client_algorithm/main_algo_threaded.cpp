/*
 * =============================================================================
 * Project:      Beacon
 * Application:  client_algorithm
 * File:         main_algo_threaded.cpp
 * Purpose:      HFT trading algorithm with production threading architecture
 *               
 * Threading Model (5-core architecture):
 *   Core 0: UDP market data receiver (minimal latency: recv → memcpy → push)
 *   Core 1: Trading logic hot path (process market data → generate orders)
 *   Core 2: TCP execution report receiver (recv → parse → push)
 *   Core 3: Reserved for NTP/timestamping (not yet implemented)
 *   Core 4+: OS reserved
 *               
 * Data Flow:
 *   Market Data: UDP → SPSC Queue → Trading Thread
 *   Exec Reports: TCP → SPSC Queue → Trading Thread
 *   Orders: Trading Thread → TCP Client
 *               
 * Performance Features:
 *   - Lock-free SPSC queues for inter-thread communication
 *   - Thread pinning to dedicated cores (Linux only)
 *   - cpu_pause() for efficient spinning
 *   - Zero-copy data path where possible
 *   - TCP_NODELAY for order entry
 *               
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <iostream>
#include <iomanip>
#include <cstring>
#include <atomic>
#include <chrono>
#include <csignal>

// Beacon HFT Infrastructure
#include <hft/ringbuffer/spsc_ringbuffer.h>
#include <hft/concurrency/pinned_thread.h>
#include <hft/core/cpu_pause.h>
#include <hft/networking/udp_receiver.h>
#include <hft/networking/tcp_client.h>

using namespace beacon::hft;

// =============================================================================
// Message Structures
// =============================================================================

// Simple market data message (32 bytes) - protocol-agnostic for now
struct MarketDataMessage {
    uint32_t msgType;
    uint32_t symbolId;
    uint32_t price;      // Price in 1/10000 dollars
    uint32_t size;
    char symbol[8];
    uint32_t timestamp;
    uint32_t padding;
};

static_assert(sizeof(MarketDataMessage) == 32, "MarketDataMessage must be 32 bytes");

// NASDAQ OUCH v5.0 Enter Order (64 bytes)
struct OuchEnterOrder {
    uint64_t clientOrderId;
    char symbol[8];
    uint32_t shares;
    uint32_t price;
    char side;           // 'B'=buy, 'S'=sell
    char timeInForce;    // '0'=Day
    char orderType;      // 'O'=order
    char capacity;       // 'A'=agency
    uint16_t reserved;
    char _padding[34];
};

static_assert(sizeof(OuchEnterOrder) == 64, "OuchEnterOrder must be 64 bytes");

// Execution Report (32 bytes)
struct ExecutionReport {
    uint32_t msgType;    // 3=Execution
    uint32_t orderId;
    char symbol[8];
    uint32_t execQty;
    uint32_t execPrice;
    uint32_t status;     // 0=New, 1=PartialFill, 2=Filled
    uint32_t padding;
};

static_assert(sizeof(ExecutionReport) == 32, "ExecutionReport must be 32 bytes");

// =============================================================================
// Global State (Atomics for thread-safe counters)
// =============================================================================

std::atomic<bool> g_running{true};
std::atomic<uint64_t> g_mdReceived{0};
std::atomic<uint64_t> g_mdProcessed{0};
std::atomic<uint64_t> g_ordersSent{0};
std::atomic<uint64_t> g_execsReceived{0};
std::atomic<uint64_t> g_mdDropped{0};
std::atomic<uint64_t> g_execDropped{0};

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[SIGNAL] Shutting down gracefully...\n";
        g_running.store(false, std::memory_order_relaxed);
    }
}

// =============================================================================
// Core 0: Market Data Receiver Thread (Minimal Latency)
// =============================================================================

void marketDataReceiverThread(
    std::atomic<bool>& stopFlag,
    networking::UdpMulticastReceiver& udpReceiver,
    ringbuffer::SpScRingBuffer<MarketDataMessage, 32768>& mdQueue)
{
    std::cout << "[Core 0] Market data receiver thread started\n";
    
    MarketDataMessage buffer;
    
    while (!stopFlag.load(std::memory_order_relaxed)) {
        // HOT PATH: Minimal code between kernel and queue
        ssize_t n = udpReceiver.recv(&buffer, sizeof(buffer));
        
        if (n == sizeof(MarketDataMessage)) {
            // Fast path: got complete message
            g_mdReceived.fetch_add(1, std::memory_order_relaxed);
            
            // Try to push (non-blocking - if queue full, drop and count)
            if (!mdQueue.tryPush(buffer)) {
                g_mdDropped.fetch_add(1, std::memory_order_relaxed);
            }
        }
        // Ignore partial/invalid packets (UDP can fragment)
    }
    
    std::cout << "[Core 0] Market data receiver thread stopped\n";
}

// =============================================================================
// Core 2: Execution Report Receiver Thread
// =============================================================================

void executionReportReceiverThread(
    std::atomic<bool>& stopFlag,
    networking::TcpClient& tcpClient,
    ringbuffer::SpScRingBuffer<ExecutionReport, 16384>& execQueue)
{
    std::cout << "[Core 2] Execution report receiver thread started\n";
    
    ExecutionReport buffer;
    
    while (!stopFlag.load(std::memory_order_relaxed)) {
        ssize_t n = tcpClient.recv(&buffer, sizeof(buffer));
        
        if (n == 0) {
            // Connection closed by exchange
            std::cout << "[Core 2] Exchange closed connection\n";
            break;
        }
        
        if (n < 0) {
            if (stopFlag.load(std::memory_order_relaxed)) {
                break; // Graceful shutdown
            }
            std::cerr << "[Core 2 ERROR] TCP recv failed\n";
            break;
        }
        
        if (n == sizeof(ExecutionReport)) {
            g_execsReceived.fetch_add(1, std::memory_order_relaxed);
            
            // BLOCKING push - execution reports CANNOT be dropped
            execQueue.push(buffer);
        }
    }
    
    std::cout << "[Core 2] Execution report receiver thread stopped\n";
}

// =============================================================================
// Core 1: Trading Logic Thread (Hot Path)
// =============================================================================

void tradingLogicThread(
    std::atomic<bool>& stopFlag,
    ringbuffer::SpScRingBuffer<MarketDataMessage, 32768>& mdQueue,
    ringbuffer::SpScRingBuffer<ExecutionReport, 16384>& execQueue,
    networking::TcpClient& orderEntryClient)
{
    std::cout << "[Core 1] Trading logic thread started (hot path)\n";
    
    uint64_t nextOrderId = 1;
    uint32_t orderInterval = 100; // Send order every 100 market data messages
    uint32_t mdCount = 0;
    
    MarketDataMessage md;
    ExecutionReport exec;
    
    while (!stopFlag.load(std::memory_order_relaxed)) {
        // PRIORITY 1: Process execution reports (critical for state management)
        while (execQueue.tryPop(exec)) {
            // Update position, working orders, PnL, etc.
            // (Simplified for now - just count)
            if (exec.msgType == 3 && exec.status == 2) {
                // Filled!
                std::cout << "[Core 1] FILL: OrderID=" << exec.orderId 
                          << " Symbol=" << std::string(exec.symbol, 8)
                          << " Qty=" << exec.execQty
                          << " Price=" << (exec.execPrice / 10000.0) << "\n";
            }
        }
        
        // PRIORITY 2: Process market data and generate trading signals
        if (mdQueue.tryPop(md)) {
            g_mdProcessed.fetch_add(1, std::memory_order_relaxed);
            mdCount++;
            
            // Simple trading logic: Send order every N messages
            if (mdCount % orderInterval == 0) {
                OuchEnterOrder order{};
                order.clientOrderId = nextOrderId++;
                std::memcpy(order.symbol, md.symbol, 8);
                order.shares = 100;
                order.price = md.price;
                order.side = (nextOrderId % 2 == 0) ? 'B' : 'S';
                order.timeInForce = '0';
                order.orderType = 'O';
                order.capacity = 'A';
                
                // Send order (blocking send for reliability)
                if (orderEntryClient.sendAll(&order, sizeof(order))) {
                    g_ordersSent.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::cerr << "[Core 1 ERROR] Failed to send order\n";
                    break;
                }
            }
        } else {
            // Queue empty - efficient spin with cpu_pause
            core::cpu_pause();
        }
    }
    
    std::cout << "[Core 1] Trading logic thread stopped\n";
}

// =============================================================================
// Main Function
// =============================================================================

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cout << "Usage: " << argv[0] << " <mcast_addr> <md_port> <ex_host> <ex_port> <duration_sec>\n";
        std::cout << "Example: " << argv[0] << " 239.255.0.1 12345 127.0.0.1 9000 60\n";
        return 1;
    }
    
    const char* mcastAddr = argv[1];
    uint16_t mdPort = static_cast<uint16_t>(std::stoi(argv[2]));
    const char* exHost = argv[3];
    uint16_t exPort = static_cast<uint16_t>(std::stoi(argv[4]));
    int durationSec = std::stoi(argv[5]);
    
    // Install signal handler
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    std::cout << "=============================================================================\n";
    std::cout << "  BEACON HFT CLIENT ALGORITHM - THREADED ARCHITECTURE\n";
    std::cout << "=============================================================================\n";
    std::cout << "Market Data: UDP " << mcastAddr << ":" << mdPort << "\n";
    std::cout << "Order Entry: TCP " << exHost << ":" << exPort << "\n";
    std::cout << "Duration: " << durationSec << " seconds\n";
    std::cout << "=============================================================================\n\n";
    
    try {
        // Initialize networking
        std::cout << "[INIT] Connecting to market data...\n";
        networking::UdpMulticastReceiver udpReceiver(mcastAddr, mdPort);
        std::cout << "[INIT] ✓ Joined multicast group\n";
        
        std::cout << "[INIT] Connecting to exchange...\n";
        networking::TcpClient orderClient(exHost, exPort);
        std::cout << "[INIT] ✓ Connected to exchange\n\n";
        
        // Create SPSC queues
        ringbuffer::SpScRingBuffer<MarketDataMessage, 32768> mdQueue;
        ringbuffer::SpScRingBuffer<ExecutionReport, 16384> execQueue;
        
        // Start threads (with core pinning on Linux)
        concurrency::PinnedThread mdThread(
            [&](std::atomic<bool>& stop) {
                marketDataReceiverThread(stop, udpReceiver, mdQueue);
            },
            0  // Pin to Core 0
        );
        
        concurrency::PinnedThread execThread(
            [&](std::atomic<bool>& stop) {
                executionReportReceiverThread(stop, orderClient, execQueue);
            },
            2  // Pin to Core 2
        );
        
        concurrency::PinnedThread tradingThread(
            [&](std::atomic<bool>& stop) {
                tradingLogicThread(stop, mdQueue, execQueue, orderClient);
            },
            1  // Pin to Core 1 (hot path)
        );
        
        std::cout << "[THREADS] All threads started\n";
        std::cout << "           Core 0: Market data receiver\n";
        std::cout << "           Core 1: Trading logic (hot path)\n";
        std::cout << "           Core 2: Execution reports\n\n";
        
        // Run for specified duration or until SIGINT
        auto startTime = std::chrono::steady_clock::now();
        while (g_running.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - startTime).count();
            
            // Print stats every second
            std::cout << "[" << elapsed << "s] "
                      << "MD recv=" << g_mdReceived.load()
                      << " proc=" << g_mdProcessed.load()
                      << " drop=" << g_mdDropped.load()
                      << " | Orders=" << g_ordersSent.load()
                      << " | Execs=" << g_execsReceived.load()
                      << " | Queue: " << mdQueue.highWaterMark() << "/" << 32768
                      << "\n";
            
            if (elapsed >= durationSec) {
                std::cout << "\n[TIMEOUT] Duration reached, shutting down...\n";
                g_running.store(false, std::memory_order_relaxed);
            }
        }
        
        // Threads will stop and join automatically (RAII)
        std::cout << "\n=============================================================================\n";
        std::cout << "                           FINAL STATISTICS\n";
        std::cout << "=============================================================================\n";
        std::cout << "Market Data Received:  " << g_mdReceived.load() << "\n";
        std::cout << "Market Data Processed: " << g_mdProcessed.load() << "\n";
        std::cout << "Market Data Dropped:   " << g_mdDropped.load() << "\n";
        std::cout << "Orders Sent:           " << g_ordersSent.load() << "\n";
        std::cout << "Executions Received:   " << g_execsReceived.load() << "\n";
        std::cout << "MD Queue High Water:   " << mdQueue.highWaterMark() << " / 32768\n";
        std::cout << "Exec Queue High Water: " << execQueue.highWaterMark() << " / 16384\n";
        std::cout << "=============================================================================\n";
        
    } catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
