/*
 * =============================================================================
 * Project:      Beacon
 * Application:  client_algorithm
 * File:         algo_twap.cpp
 * Strategy:     TWAP (Time-Weighted Average Price)
 * 
 * Description:
 *   Industry-standard execution algorithm that splits a large parent order
 *   into smaller child orders executed evenly over a specified time window.
 *   
 *   Goal: Minimize market impact by spreading execution over time
 *   Use Case: Executing large institutional orders without moving the market
 *   
 * Algorithm Logic:
 *   1. Divide time window into N equal slices (e.g., 10 minutes → 60 slices)
 *   2. Calculate shares per slice: totalShares / numSlices
 *   3. Every slice interval, send an order at current market price
 *   4. Track fills and adjust remaining quantity
 *   5. Handle partial fills and rejects gracefully
 *   
 * Threading Model:
 *   - Core 0: Market data receiver (UDP multicast)
 *   - Core 1: TWAP logic (time-slicing, order generation)
 *   - Core 2: Execution report receiver (TCP)
 *   
 * Performance Features:
 *   - Lock-free SPSC queues
 *   - Thread pinning (Linux)
 *   - Microsecond-precision timing
 *   - Zero-copy market data path
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
#include <cmath>

// Beacon HFT Infrastructure
#include <hft/ringbuffer/spsc_ringbuffer.h>
#include <hft/concurrency/pinned_thread.h>
#include <hft/core/cpu_pause.h>
#include <hft/networking/udp_receiver.h>
#include <hft/networking/tcp_client.h>

using namespace beacon::hft;
using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Seconds = std::chrono::seconds;
using Milliseconds = std::chrono::milliseconds;

// =============================================================================
// Message Structures
// =============================================================================

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

struct OuchEnterOrder {
    uint64_t clientOrderId;
    char symbol[8];
    uint32_t shares;
    uint32_t price;
    char side;           // 'B'=buy, 'S'=sell
    char timeInForce;    // '0'=Day, '3'=IOC
    char orderType;      // 'O'=order
    char capacity;       // 'A'=agency
    uint16_t reserved;
    char _padding[34];
};
static_assert(sizeof(OuchEnterOrder) == 64);

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
// TWAP Configuration
// =============================================================================

struct TWAPConfig {
    char symbol[8];
    uint32_t totalShares;        // Parent order size
    uint32_t durationSeconds;    // Execution window
    char side;                   // 'B' or 'S'
    uint32_t sliceIntervalMs;    // Time between orders (milliseconds)
    uint32_t maxSliceSize;       // Max shares per child order
    
    void print() const {
        std::cout << "=============================================================================\n";
        std::cout << "  TWAP ALGORITHM CONFIGURATION\n";
        std::cout << "=============================================================================\n";
        std::cout << "Symbol:           " << std::string(symbol, 8) << "\n";
        std::cout << "Side:             " << (side == 'B' ? "BUY" : "SELL") << "\n";
        std::cout << "Total Shares:     " << totalShares << "\n";
        std::cout << "Duration:         " << durationSeconds << " seconds\n";
        std::cout << "Slice Interval:   " << sliceIntervalMs << " ms\n";
        std::cout << "Max Slice Size:   " << maxSliceSize << " shares\n";
        
        uint32_t numSlices = (durationSeconds * 1000) / sliceIntervalMs;
        uint32_t sharesPerSlice = totalShares / numSlices;
        std::cout << "Calculated:\n";
        std::cout << "  Number of slices:     " << numSlices << "\n";
        std::cout << "  Shares per slice:     " << sharesPerSlice << "\n";
        std::cout << "  Orders per second:    " << (1000.0 / sliceIntervalMs) << "\n";
        std::cout << "=============================================================================\n\n";
    }
};

// =============================================================================
// TWAP State
// =============================================================================

struct TWAPState {
    std::atomic<uint32_t> sharesExecuted{0};
    std::atomic<uint32_t> sharesOutstanding{0};  // Pending in market
    std::atomic<uint32_t> ordersSent{0};
    std::atomic<uint32_t> fillsReceived{0};
    std::atomic<uint64_t> totalFillValue{0};     // Sum of (qty * price)
    std::atomic<uint32_t> rejects{0};
    
    TimePoint startTime;
    TimePoint endTime;
    
    double getAvgFillPrice() const {
        uint32_t executed = sharesExecuted.load();
        if (executed == 0) return 0.0;
        return static_cast<double>(totalFillValue.load()) / (executed * 10000.0);
    }
    
    double getProgressPercent(uint32_t totalShares) const {
        return (static_cast<double>(sharesExecuted.load()) / totalShares) * 100.0;
    }
    
    void printSummary(uint32_t totalShares) const {
        std::cout << "\n=============================================================================\n";
        std::cout << "                        TWAP EXECUTION SUMMARY\n";
        std::cout << "=============================================================================\n";
        std::cout << "Target Shares:     " << totalShares << "\n";
        std::cout << "Shares Executed:   " << sharesExecuted.load() << " ("
                  << std::fixed << std::setprecision(2) << getProgressPercent(totalShares) << "%)\n";
        std::cout << "Shares Outstanding:" << sharesOutstanding.load() << "\n";
        std::cout << "Orders Sent:       " << ordersSent.load() << "\n";
        std::cout << "Fills Received:    " << fillsReceived.load() << "\n";
        std::cout << "Order Rejects:     " << rejects.load() << "\n";
        std::cout << "Avg Fill Price:    $" << std::fixed << std::setprecision(4) 
                  << getAvgFillPrice() << "\n";
        
        auto duration = std::chrono::duration_cast<Milliseconds>(endTime - startTime);
        std::cout << "Execution Time:    " << duration.count() << " ms\n";
        std::cout << "=============================================================================\n";
    }
};

// =============================================================================
// Global State
// =============================================================================

std::atomic<bool> g_running{true};
std::atomic<uint32_t> g_currentMarketPrice{1000000};  // $100.00 default
TWAPState g_state;

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[SIGNAL] Shutting down TWAP algorithm...\n";
        g_running.store(false, std::memory_order_relaxed);
    }
}

// =============================================================================
// Core 0: Market Data Receiver Thread
// =============================================================================

void marketDataReceiverThread(
    std::atomic<bool>& stopFlag,
    networking::UdpMulticastReceiver& udpReceiver,
    ringbuffer::SpScRingBuffer<MarketDataMessage, 32768>& mdQueue)
{
    std::cout << "[Core 0] Market data receiver started\n";
    
    MarketDataMessage buffer;
    
    while (!stopFlag.load(std::memory_order_relaxed)) {
        ssize_t n = udpReceiver.recv(&buffer, sizeof(buffer));
        
        if (n == sizeof(MarketDataMessage)) {
            // Update current market price (TWAP needs this for order pricing)
            g_currentMarketPrice.store(buffer.price, std::memory_order_relaxed);
            
            // Try to push to queue (best effort for market data)
            mdQueue.tryPush(buffer);
        }
    }
    
    std::cout << "[Core 0] Market data receiver stopped\n";
}

// =============================================================================
// Core 2: Execution Report Receiver Thread
// =============================================================================

void executionReportReceiverThread(
    std::atomic<bool>& stopFlag,
    networking::TcpClient& tcpClient,
    ringbuffer::SpScRingBuffer<ExecutionReport, 16384>& execQueue)
{
    std::cout << "[Core 2] Execution report receiver started\n";
    
    ExecutionReport buffer;
    
    while (!stopFlag.load(std::memory_order_relaxed)) {
        ssize_t n = tcpClient.recv(&buffer, sizeof(buffer));
        
        if (n == 0) {
            std::cout << "[Core 2] Exchange closed connection\n";
            break;
        }
        
        if (n < 0) {
            if (stopFlag.load(std::memory_order_relaxed)) break;
            std::cerr << "[Core 2 ERROR] TCP recv failed\n";
            break;
        }
        
        if (n == sizeof(ExecutionReport)) {
            // Blocking push - execution reports CANNOT be dropped
            execQueue.push(buffer);
        }
    }
    
    std::cout << "[Core 2] Execution report receiver stopped\n";
}

// =============================================================================
// Core 1: TWAP Trading Logic Thread
// =============================================================================

void twapLogicThread(
    std::atomic<bool>& stopFlag,
    const TWAPConfig& config,
    ringbuffer::SpScRingBuffer<MarketDataMessage, 32768>& mdQueue,
    ringbuffer::SpScRingBuffer<ExecutionReport, 16384>& execQueue,
    networking::TcpClient& orderClient)
{ 
    std::cout << "[Core 1] TWAP logic thread started\n";
    
    g_state.startTime = Clock::now();
    TimePoint endTime = g_state.startTime + Seconds(config.durationSeconds);
    TimePoint nextSliceTime = g_state.startTime;
    
    uint64_t nextOrderId = 1;
    uint32_t numSlices = (config.durationSeconds * 1000) / config.sliceIntervalMs;
    uint32_t sharesPerSlice = config.totalShares / numSlices;
    
    // Ensure at least 1 share per slice
    if (sharesPerSlice == 0) sharesPerSlice = 1;
    
    std::cout << "[Core 1] TWAP starting execution:\n";
    std::cout << "         " << numSlices << " slices of " << sharesPerSlice << " shares each\n";
    std::cout << "         Slice every " << config.sliceIntervalMs << " ms\n\n";
    
    MarketDataMessage md;
    ExecutionReport exec;
    
    while (!stopFlag.load(std::memory_order_relaxed)) {
        auto now = Clock::now();
        
        // PRIORITY 1: Process execution reports
        while (execQueue.tryPop(exec)) {
            if (exec.msgType == 3) {  // Execution report
                uint32_t fillQty = exec.execQty;
                uint32_t fillPrice = exec.execPrice;
                
                // Update state
                g_state.sharesExecuted.fetch_add(fillQty, std::memory_order_relaxed);
                g_state.sharesOutstanding.fetch_sub(fillQty, std::memory_order_relaxed);
                g_state.fillsReceived.fetch_add(1, std::memory_order_relaxed);
                g_state.totalFillValue.fetch_add(fillQty * fillPrice, std::memory_order_relaxed);
                
                if (exec.status == 2) {  // Filled
                    std::cout << "[FILL] Qty=" << fillQty 
                              << " Price=$" << std::fixed << std::setprecision(4) 
                              << (fillPrice / 10000.0)
                              << " Total=" << g_state.sharesExecuted.load()
                              << "/" << config.totalShares << "\n";
                }
            }
        }
        
        // PRIORITY 2: Process market data (consume queue to prevent backup)
        while (mdQueue.tryPop(md)) {
            // Market data processed in receiver thread (updates price)
        }
        
        // PRIORITY 3: Time-slice logic - send orders on schedule
        if (now >= nextSliceTime && now < endTime) {
            uint32_t executed = g_state.sharesExecuted.load();
            uint32_t outstanding = g_state.sharesOutstanding.load();
            uint32_t remaining = config.totalShares - executed - outstanding;
            
            if (remaining > 0) {
                // Calculate shares for this slice
                uint32_t sliceSize = std::min(remaining, sharesPerSlice);
                sliceSize = std::min(sliceSize, config.maxSliceSize);
                
                // Get current market price
                uint32_t price = g_currentMarketPrice.load(std::memory_order_relaxed);
                
                // Build OUCH order
                OuchEnterOrder order{};
                order.clientOrderId = nextOrderId++;
                std::memcpy(order.symbol, config.symbol, 8);
                order.shares = sliceSize;
                order.price = price;
                order.side = config.side;
                order.timeInForce = '3';  // IOC (Immediate-or-Cancel)
                order.orderType = 'O';
                order.capacity = 'A';
                
                // Send order
                if (orderClient.sendAll(&order, sizeof(order))) {
                    g_state.ordersSent.fetch_add(1, std::memory_order_relaxed);
                    g_state.sharesOutstanding.fetch_add(sliceSize, std::memory_order_relaxed);
                } else {
                    std::cerr << "[ERROR] Failed to send TWAP order\n";
                    break;
                }
            }
            
            // Advance to next slice time
            nextSliceTime += Milliseconds(config.sliceIntervalMs);
        }
        
        // Check if TWAP execution window complete
        if (now >= endTime) {
            std::cout << "\n[TWAP] Execution window complete\n";
            break;
        }
        
        // Check if target reached
        if (g_state.sharesExecuted.load() >= config.totalShares) {
            std::cout << "\n[TWAP] Target shares executed!\n";
            break;
        }
        
        // Efficient spin
        core::cpu_pause();
    }
    
    g_state.endTime = Clock::now();
    std::cout << "[Core 1] TWAP logic thread stopped\n";
}

// =============================================================================
// Main Function
// =============================================================================

int main(int argc, char* argv[]) {
    if (argc != 10) {
        std::cout << "Usage: " << argv[0] 
                  << " <symbol> <side> <shares> <duration_sec> <slice_ms>"
                  << " <mcast_addr> <md_port> <ex_host> <ex_port>\n";
        std::cout << "\nExample (Buy 10,000 AAPL over 60 seconds, slice every 1000ms):\n";
        std::cout << "  " << argv[0] << " AAPL B 10000 60 1000 239.255.0.1 12345 127.0.0.1 9000\n";
        return 1;
    }
    
    // Parse TWAP config
    TWAPConfig config{};
    std::string symbolStr = argv[1];
    std::memcpy(config.symbol, symbolStr.c_str(), std::min(symbolStr.size(), size_t(8)));
    config.side = argv[2][0];
    config.totalShares = std::stoul(argv[3]);
    config.durationSeconds = std::stoul(argv[4]);
    config.sliceIntervalMs = std::stoul(argv[5]);
    config.maxSliceSize = 1000;  // Default max
    
    const char* mcastAddr = argv[6];
    uint16_t mdPort = static_cast<uint16_t>(std::stoi(argv[7]));
    const char* exHost = argv[8];
    uint16_t exPort = static_cast<uint16_t>(std::stoi(argv[9]));
    
    // Install signal handler
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    config.print();
    
    try {
        // Initialize networking
        std::cout << "[INIT] Connecting to market data " << mcastAddr << ":" << mdPort << "...\n";
        networking::UdpMulticastReceiver udpReceiver(mcastAddr, mdPort);
        std::cout << "[INIT] ✓ Market data connected\n";
        
        std::cout << "[INIT] Connecting to exchange " << exHost << ":" << exPort << "...\n";
        networking::TcpClient orderClient(exHost, exPort);
        std::cout << "[INIT] ✓ Exchange connected\n\n";
        
        // Create queues
        ringbuffer::SpScRingBuffer<MarketDataMessage, 32768> mdQueue;
        ringbuffer::SpScRingBuffer<ExecutionReport, 16384> execQueue;
        
        // Start threads
        concurrency::PinnedThread mdThread(
            [&](std::atomic<bool>& stop) {
                marketDataReceiverThread(stop, udpReceiver, mdQueue);
            }, 0);
        
        concurrency::PinnedThread execThread(
            [&](std::atomic<bool>& stop) {
                executionReportReceiverThread(stop, orderClient, execQueue);
            }, 2);
        
        concurrency::PinnedThread twapThread(
            [&](std::atomic<bool>& stop) {
                twapLogicThread(stop, config, mdQueue, execQueue, orderClient);
            }, 1);
        
        // Wait for TWAP to complete or SIGINT
        while (g_running.load()) {
            std::this_thread::sleep_for(Milliseconds(100));
            
            // Check if execution complete
            if (g_state.sharesExecuted.load() >= config.totalShares) {
                break;
            }
        }
        
        // Give threads a moment to finish
        std::this_thread::sleep_for(Milliseconds(500));
        g_running.store(false);
        
        // Print summary
        g_state.printSummary(config.totalShares);
        
    } catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
