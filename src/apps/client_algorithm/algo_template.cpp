/*
 * =============================================================================
 * Project:      Beacon
 * Application:  client_algorithm
 * File:         algo_template.cpp
 * Strategy:     TEMPLATE - Copy this to create your own algorithm
 * 
 * Description:
 *   Minimal template showing how to plug in a new trading algorithm using
 *   Beacon's HFT infrastructure. This demonstrates the essential pattern:
 *   
 *   1. Include HFT headers
 *   2. Define message structures
 *   3. Implement 3 thread functions (market data, exec reports, trading logic)
 *   4. Wire everything together in main()
 *   
 * Usage:
 *   1. Copy this file: cp algo_template.cpp algo_your_strategy.cpp
 *   2. Implement your trading logic in tradingLogicThread()
 *   3. Update CMakeLists.txt to build your algorithm
 *   4. Build and run!
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
#include <sys/socket.h>

// Beacon HFT Infrastructure (header-only, zero overhead)
#include <hft/ringbuffer/spsc_ringbuffer.h>
#include <hft/concurrency/pinned_thread.h>
#include <hft/core/cpu_pause.h>
#include <hft/networking/udp_receiver.h>
#include <hft/networking/tcp_client.h>
#include <hft/profiling/latency_tracker.h>

using namespace beacon::hft;

// =============================================================================
// STEP 1: Define Your Message Structures
// =============================================================================

// Market data message (32 bytes) - matches what md_playback sends
struct MarketDataMessage {
    uint32_t msgType;
    uint32_t symbolId;
    uint32_t price;      // Price in 1/10000 dollars
    uint32_t size;
    char symbol[8];
    uint32_t timestamp;
    uint32_t padding;
};
static_assert(sizeof(MarketDataMessage) == 32);

// OUCH order (64 bytes) - what we send to matching engine
struct OuchEnterOrder {
    uint64_t clientOrderId;  // 8 bytes
    char symbol[8];          // 8 bytes
    uint32_t shares;         // 4 bytes
    uint32_t price;          // 4 bytes
    char side;               // 1 byte: 'B'=buy, 'S'=sell
    char timeInForce;        // 1 byte: '0'=Day, '3'=IOC
    char orderType;          // 1 byte: 'O'=OUCH order
    char capacity;           // 1 byte: 'A'=agency
    uint16_t reserved;       // 2 bytes
    char _padding[34];       // 34 bytes padding
};
static_assert(sizeof(OuchEnterOrder) == 64);

// Execution report (32 bytes) - what matching engine sends back
struct ExecutionReport {
    uint32_t msgType;    // 3=Execution
    uint32_t orderId;
    char symbol[8];
    uint32_t execQty;
    uint32_t execPrice;
    uint32_t status;     // 0=New, 1=PartialFill, 2=Filled
    uint32_t padding;
};
static_assert(sizeof(ExecutionReport) == 32);

// =============================================================================
// STEP 2: Define Your Algorithm State
// =============================================================================

// Global state (use atomics for thread-safe counters)
std::atomic<bool> g_running{true};
std::atomic<uint64_t> g_mdReceived{0};
std::atomic<uint64_t> g_ordersSent{0};
std::atomic<uint64_t> g_fillsReceived{0};

// Latency tracking (tick-to-trade)
profiling::LatencyTracker<1000000> g_tickToTradeLatency;  // Track up to 1M samples

// Your algorithm-specific state here
struct AlgoState {
    // Example: Simple counter-based logic
    uint32_t orderInterval = 100;  // Send order every N market data messages
    uint64_t nextOrderId = 1;
    
    // Add your state here (positions, signals, indicators, etc.)
};

AlgoState g_algoState;

// Signal handler for clean shutdown
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[SIGNAL] Received signal " << signal << " - shutting down NOW!\n" << std::flush;
        g_running.store(false, std::memory_order_release);
        // Force immediate exit
        std::exit(0);
    }
}

// =============================================================================
// STEP 3: Implement Thread Functions
// =============================================================================

/*
 * Core 0: Market Data Receiver Thread
 * 
 * Job: Receive UDP packets and push to queue (MINIMAL LATENCY)
 * Pattern: recv() -> tryPush() -> done
 */
void marketDataReceiverThread(
    std::atomic<bool>& stopFlag,
    networking::UdpMulticastReceiver& udpReceiver,
    ringbuffer::SpScRingBuffer<MarketDataMessage, 32768>& mdQueue)
{
    std::cout << "[Core 0] Market data receiver started\n";
    std::cout << "[Core 0 DEBUG] Expecting message size: " << sizeof(MarketDataMessage) << " bytes\n";
    std::cout << "[Core 0 DEBUG] Thread sees UDP socket FD=" << udpReceiver.fd() << "\n";
    
    MarketDataMessage buffer;
    size_t recvAttempts = 0;
    size_t timeouts = 0;
    size_t wrongSize = 0;
    
    while (!stopFlag.load(std::memory_order_relaxed)) {
        // HOT PATH: Just recv and push, nothing else
        if (recvAttempts == 0) {
            std::cout << "[Core 0 DEBUG] About to call recv() on FD=" << udpReceiver.fd() << "\n";
        }
        ssize_t n = udpReceiver.recv(&buffer, sizeof(buffer));
        recvAttempts++;
        
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                timeouts++;
                continue;  // Timeout, try again
            }
            // Real error
            if (recvAttempts < 10) {  // Only log first few errors
                std::cerr << "[Core 0 ERROR] recv() failed: " << strerror(errno) << " (errno=" << errno << ")\n";
            }
            continue;
        }
        
        if (n != sizeof(MarketDataMessage)) {
            wrongSize++;
            if (wrongSize < 10) {  // Only log first few
                std::cout << "[Core 0 WARN] Received " << n << " bytes, expected " << sizeof(MarketDataMessage) << "\n";
            }
            continue;
        }
        
        // Got valid message!
        g_mdReceived.fetch_add(1, std::memory_order_relaxed);
        mdQueue.tryPush(buffer);  // Best effort (can drop if queue full)
    }
    
    std::cout << "[Core 0] Market data receiver stopped\n";
    std::cout << "[Core 0 DEBUG] recv() attempts: " << recvAttempts 
              << ", timeouts: " << timeouts 
              << ", wrong size: " << wrongSize << "\n";
}

/*
 * Core 2: Execution Report Receiver Thread
 * 
 * Job: Receive fills/acks from exchange and push to queue (CANNOT DROP)
 * Pattern: recv() -> push() (blocking) -> done
 */
void executionReportReceiverThread(
    std::atomic<bool>& stopFlag,
    networking::TcpClient& tcpClient,
    ringbuffer::SpScRingBuffer<ExecutionReport, 16384>& execQueue)
{
    std::cout << "[Core 2] Execution report receiver started\n";
    
    ExecutionReport buffer;
    bool warned = false;
    
    while (!stopFlag.load(std::memory_order_relaxed)) {
        ssize_t n = tcpClient.recv(&buffer, sizeof(buffer));
        
        if (n == 0) {
            if (!warned) {
                std::cout << "[Core 2 WARN] Exchange closed connection - continuing without exec reports\n";
                warned = true;
            }
            // Don't break - just wait for stop flag
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        if (n < 0) {
            if (stopFlag.load(std::memory_order_relaxed)) break;
            if (!warned) {
                std::cerr << "[Core 2 WARN] TCP recv failed (errno=" << errno << ") - continuing without exec reports\n";
                warned = true;
            }
            // Don't break - just wait for stop flag
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        if (n == sizeof(ExecutionReport)) {
            // BLOCKING push with timeout - execution reports CANNOT be dropped
            // But we need to detect if trading thread is hung/deadlocked
            if (!execQueue.push(buffer, 5000)) {  // 5 second timeout
                std::cerr << "[Core 2 CRITICAL] Failed to push execution report - trading thread hung for >5s!\n";
                std::cerr << "[Core 2 CRITICAL] This indicates a deadlock or consumer stopped processing.\n";
                std::cerr << "[Core 2 CRITICAL] Dropped=" << execQueue.dropped() 
                          << " HighWater=" << execQueue.highWaterMark() << "\n";
                // Continue trying - don't exit, but alert user
            } else {
                g_fillsReceived.fetch_add(1, std::memory_order_relaxed);
                warned = false;  // Reset warning flag if we start receiving again
            }
        }
    }
    
    std::cout << "[Core 2] Execution report receiver stopped\n";
}

/*
 * Core 1: Trading Logic Thread (HOT PATH)
 * 
 * Job: YOUR ALGORITHM GOES HERE!
 * Pattern: 
 *   1. Process exec reports (update state)
 *   2. Process market data (generate signals)
 *   3. Send orders
 *   4. cpu_pause() if queues empty
 */
void tradingLogicThread(
    std::atomic<bool>& stopFlag,
    ringbuffer::SpScRingBuffer<MarketDataMessage, 32768>& mdQueue,
    ringbuffer::SpScRingBuffer<ExecutionReport, 16384>& execQueue,
    networking::TcpClient& orderClient)
{
    std::cout << "[Core 1] Trading logic started (HOT PATH - YOUR CODE HERE)\n";
    
    MarketDataMessage md;
    ExecutionReport exec;
    uint32_t mdCount = 0;
    
    while (!stopFlag.load(std::memory_order_relaxed)) {
        // =====================================================================
        // PRIORITY 1: Process execution reports (update positions, PnL, etc.)
        // =====================================================================
        while (execQueue.tryPop(exec)) {
            g_fillsReceived.fetch_add(1, std::memory_order_relaxed);
            
            // YOUR CODE: Update positions, track PnL, manage working orders
            if (exec.msgType == 3 && exec.status == 2) {  // Filled
                std::cout << "[FILL] OrderID=" << exec.orderId 
                          << " Qty=" << exec.execQty 
                          << " Price=$" << (exec.execPrice / 10000.0) << "\n";
            }
        }
        
        // =====================================================================
        // PRIORITY 2: Process market data and generate trading signals
        // =====================================================================
        if (mdQueue.tryPop(md)) {
            // START LATENCY MEASUREMENT (tick received)
            auto t0 = profiling::HighResTimer::now();
            
            mdCount++;
            
            // ===== YOUR TRADING LOGIC GOES HERE =====
            // 
            // Examples:
            // - Mean reversion: if (price > ema) sell(); else buy();
            // - Momentum: if (price_change > threshold) buy();
            // - Market making: quote(bid, ask);
            // - TWAP: if (time_for_slice) send_order();
            // - Arbitrage: if (spread > threshold) trade();
            //
            // Simple example: Send order every N messages
            if (mdCount % g_algoState.orderInterval == 0) {
                OuchEnterOrder order{};
                order.clientOrderId = g_algoState.nextOrderId++;
                std::memcpy(order.symbol, md.symbol, 8);
                order.shares = 100;
                order.price = md.price;
                order.side = (g_algoState.nextOrderId % 2 == 0) ? 'B' : 'S';
                order.timeInForce = '3';  // IOC
                order.orderType = 'O';
                order.capacity = 'A';
                
                bool sendSuccess = orderClient.sendAll(&order, sizeof(order));
                
                if (sendSuccess) {
                    // END LATENCY MEASUREMENT (order sent)
                    auto t1 = profiling::HighResTimer::now();
                    g_tickToTradeLatency.record(t0, t1);
                    
                    uint64_t ordersNow = g_ordersSent.fetch_add(1, std::memory_order_relaxed) + 1;
                    
                    // Debug: print first few orders
                    if (ordersNow <= 5) {
                        std::cout << "[Core 1 DEBUG] Sent order #" << ordersNow 
                                  << " - OrderID=" << order.clientOrderId
                                  << " Symbol=" << std::string(order.symbol, 8)
                                  << " Side=" << order.side
                                  << " Shares=" << order.shares
                                  << " Price=" << order.price << "\n";
                    }
                } else {
                    // TCP send failed - exchange may have closed connection
                    std::cerr << "[Core 1 ERROR] Failed to send order - TCP connection lost\n";
                    // Continue processing market data anyway
                }
            }
            // ===== END YOUR TRADING LOGIC =====
            
        } else {
            // Queue empty - efficient spin
            core::cpu_pause();
        }
    }
    
    std::cout << "[Core 1] Trading logic stopped\n";
}

// =============================================================================
// STEP 4: Main Function (Wire Everything Together)
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
    
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "  [3] <AlgoTemplate> Starting...                               \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "      Market Data: UDP " << mcastAddr << ":" << mdPort << "\n";
    std::cout << "      Order Entry: TCP " << exHost << ":" << exPort << "\n";
    std::cout << "      Duration:    " << durationSec << " seconds\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";
    
    try {
        // Initialize networking
        networking::UdpMulticastReceiver udpReceiver(mcastAddr, mdPort);
        std::cout << "[DEBUG] UDP receiver created, socket FD=" << udpReceiver.fd() << "\n";
        
        networking::TcpClient orderClient(exHost, exPort);
        std::cout << "[DEBUG] TCP client created, socket FD=" << orderClient.fd() << "\n";
        
        // Set receive timeout on UDP socket so it can check stop flag
        struct timeval udp_tv;
        udp_tv.tv_sec = 0;
        udp_tv.tv_usec = 100000;  // 100ms timeout
        if (setsockopt(udpReceiver.fd(), SOL_SOCKET, SO_RCVTIMEO, &udp_tv, sizeof(udp_tv)) < 0) {
            std::cerr << "[WARNING] Failed to set UDP timeout: " << strerror(errno) << "\n";
        }
        std::cout << "[DEBUG] After UDP setsockopt, FD=" << udpReceiver.fd() << "\n";
        
        // Set receive timeout on TCP socket so recv() doesn't block forever
        // This allows threads to exit cleanly when g_running becomes false
        struct timeval tv;
        tv.tv_sec = 1;   // 1 second timeout
        tv.tv_usec = 0;
        if (setsockopt(orderClient.fd(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            std::cerr << "[WARNING] Failed to set TCP timeout: " << strerror(errno) << "\n";
        }
        std::cout << "[DEBUG] After TCP setsockopt, FD=" << orderClient.fd() << "\n";
        
        std::cout << "═══════════════════════════════════════════════════════════════\n";
        std::cout << "  [4] <AlgoTemplate> Connected to <OUCH> Matching Engine      \n";
        std::cout << "═══════════════════════════════════════════════════════════════\n";
        std::cout << "      UDP:  " << mcastAddr << ":" << mdPort << " ✓\n";
        std::cout << "      TCP:  " << exHost << ":" << exPort << " ✓\n";
        std::cout << "═══════════════════════════════════════════════════════════════\n\n";
        
        // Create SPSC queues (lock-free inter-thread communication)
        ringbuffer::SpScRingBuffer<MarketDataMessage, 32768> mdQueue;
        ringbuffer::SpScRingBuffer<ExecutionReport, 16384> execQueue;
        
        // Start threads with core pinning
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
        std::cout << "           Core 1: Trading logic (YOUR CODE)\n";
        std::cout << "           Core 2: Execution reports\n\n";
        
        std::cout << "═══════════════════════════════════════════════════════════════\n";
        std::cout << "  System Running - Waiting for market data...                 \n";
        std::cout << "═══════════════════════════════════════════════════════════════\n\n";
        
        // Run for specified duration or until SIGINT
        auto startTime = std::chrono::steady_clock::now();
        while (g_running.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            // Check stop flag more frequently
            if (!g_running.load(std::memory_order_acquire)) {
                break;
            }
            
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - startTime).count();
            
            // Print stats every second
            if (elapsed > 0 && elapsed % 1 == 0) {
                std::cout << "[" << elapsed << "s] "
                          << "MD=" << g_mdReceived.load()
                          << " Orders=" << g_ordersSent.load()
                          << " Fills=" << g_fillsReceived.load()
                          << "\r" << std::flush;
            }
            
            if (elapsed >= durationSec) {
                std::cout << "\n[TIMEOUT] Duration reached\n" << std::flush;
                g_running.store(false, std::memory_order_release);
                break;
            }
        }
        
        std::cout << "\n[SHUTDOWN] Waiting for threads to stop...\n" << std::flush;
        
        // Threads stop automatically (RAII)
        std::cout << "\n=============================================================================\n";
        std::cout << "                         FINAL STATISTICS\n";
        std::cout << "=============================================================================\n";
        std::cout << "Market Data Received:  " << g_mdReceived.load() << "\n";
        std::cout << "Orders Sent:           " << g_ordersSent.load() << "\n";
        std::cout << "Fills Received:        " << g_fillsReceived.load() << "\n";
        std::cout << "=============================================================================\n";
        
        // Print tick-to-trade latency statistics
        if (g_ordersSent.load() > 0) {
            auto stats = g_tickToTradeLatency.getStats();
            
            std::cout << "\n";
            std::cout << "═══════════════════════════════════════════════════════════════════════════\n";
            std::cout << "                    TICK-TO-TRADE LATENCY STATISTICS                       \n";
            std::cout << "═══════════════════════════════════════════════════════════════════════════\n";
            std::cout << "  Samples:        " << stats.samples_recorded << " / " << stats.count << "\n";
            std::cout << "───────────────────────────────────────────────────────────────────────────\n";
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "  Min:            " << stats.min_us << " μs\n";
            std::cout << "  Mean:           " << stats.mean_us << " μs\n";
            std::cout << "  Median (p50):   " << stats.median_us << " μs\n";
            std::cout << "  p95:            " << stats.p95_us << " μs\n";
            std::cout << "  p99:            " << stats.p99_us << " μs\n";
            std::cout << "  p99.9:          " << stats.p999_us << " μs\n";
            std::cout << "  Max:            " << stats.max_us << " μs\n";
            std::cout << "═══════════════════════════════════════════════════════════════════════════\n";
            std::cout << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
