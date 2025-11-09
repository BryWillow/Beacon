// Example: Measuring Tick-to-Trade Latency with LatencyTracker
// 
// This shows how to use the lightweight latency tracker in your algorithm
// to measure microsecond-level tick-to-trade latency WITHOUT impacting
// the hot path performance.

#include "../../../include/hft/profiling/latency_tracker.h"
#include <iostream>
#include <iomanip>

using namespace beacon::hft::profiling;

// Market data message (simplified)
struct MarketDataMessage {
    uint64_t timestamp;  // Timestamp from exchange (or capture time)
    uint32_t price;
    uint32_t size;
    // ... other fields
};

// Global latency tracker (or per-strategy instance)
LatencyTracker<1000000> g_tickToTradeLatency;  // Track up to 1M samples

// ============================================================================
// EXAMPLE 1: Manual Measurement
// ============================================================================

void processMarketData_Manual(const MarketDataMessage& md) {
    // Capture timestamp as soon as we receive the message
    auto receive_ts = HighResTimer::now();
    
    // ===== YOUR TRADING LOGIC HERE =====
    // Check signals, make decision, etc.
    bool shouldTrade = (md.price % 100 == 0);  // Example: trade on round prices
    // ===================================
    
    if (shouldTrade) {
        // Send order here...
        auto send_ts = HighResTimer::now();
        
        // Record tick-to-trade latency (HOT PATH - just one array write)
        g_tickToTradeLatency.record(receive_ts, send_ts);
    }
}

// ============================================================================
// EXAMPLE 2: Using Market Data Timestamp
// ============================================================================

void processMarketData_WithMDTimestamp(const MarketDataMessage& md) {
    // If MD has exchange timestamp, use it for true tick-to-trade
    auto md_timestamp = md.timestamp;  // From exchange
    
    // Your trading logic
    bool shouldTrade = true;
    
    if (shouldTrade) {
        auto send_ts = HighResTimer::now();
        
        // Record delta: now - exchange timestamp
        g_tickToTradeLatency.recordDelta(send_ts - md_timestamp);
    }
}

// ============================================================================
// EXAMPLE 3: Scoped Measurement (RAII)
// ============================================================================

void processMarketData_Scoped(const MarketDataMessage& md) {
    // Automatically measures from construction to destruction
    ScopedLatency measure(g_tickToTradeLatency);
    
    // Your trading logic
    // ... when this function returns, latency is automatically recorded
}

// ============================================================================
// EXAMPLE 4: Real Trading Thread Pattern
// ============================================================================

void tradingLogicThread() {
    LatencyTracker<1000000> tickToTrade;  // Thread-local tracker
    
    // ... receive market data from queue
    
    while (true) {
        MarketDataMessage md;
        // if (queue.tryPop(md)) {
            auto t0 = HighResTimer::now();  // ← HOT PATH: ~20 CPU cycles
            
            // ===== YOUR ALGORITHM =====
            bool signal = (md.price > 100000);  // Example
            // ==========================
            
            if (signal) {
                // sendOrder(...);
                auto t1 = HighResTimer::now();  // ← HOT PATH: ~20 CPU cycles
                
                tickToTrade.record(t0, t1);     // ← HOT PATH: ~5 CPU cycles (array write)
            }
        // }
    }
}

// ============================================================================
// Print Statistics (OFFLINE - not in hot path)
// ============================================================================

void printLatencyStats(const LatencyTracker<>::Stats& stats) {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════\n";
    std::cout << "           TICK-TO-TRADE LATENCY STATISTICS            \n";
    std::cout << "═══════════════════════════════════════════════════════\n";
    std::cout << "  Total Events:   " << stats.count << "\n";
    std::cout << "  Samples:        " << stats.samples_recorded << "\n";
    std::cout << "───────────────────────────────────────────────────────\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Min:            " << stats.min_us << " μs\n";
    std::cout << "  Mean:           " << stats.mean_us << " μs\n";
    std::cout << "  Median (p50):   " << stats.median_us << " μs\n";
    std::cout << "  p95:            " << stats.p95_us << " μs\n";
    std::cout << "  p99:            " << stats.p99_us << " μs\n";
    std::cout << "  p99.9:          " << stats.p999_us << " μs\n";
    std::cout << "  Max:            " << stats.max_us << " μs\n";
    std::cout << "═══════════════════════════════════════════════════════\n";
    std::cout << "\n";
}

// ============================================================================
// MAIN - Demo Usage
// ============================================================================

int main() {
    std::cout << "Latency Tracker Demo\n";
    std::cout << "====================\n\n";
    
    // Simulate some market data processing
    for (int i = 0; i < 10000; ++i) {
        MarketDataMessage md{};
        md.timestamp = HighResTimer::now();
        md.price = 100000 + (i % 100);
        md.size = 100;
        
        processMarketData_Manual(md);
        
        // Simulate some work between messages (a few microseconds)
        for (volatile int j = 0; j < 1000; ++j) {}
    }
    
    // Get and print statistics (OFFLINE)
    auto stats = g_tickToTradeLatency.getStats();
    printLatencyStats(stats);
    
    // Performance characteristics
    std::cout << "Performance Characteristics:\n";
    std::cout << "  - HighResTimer::now():  ~20 CPU cycles (x86 RDTSC)\n";
    std::cout << "  - record():             ~5 CPU cycles (array write)\n";
    std::cout << "  - Total overhead:       ~25-30 CPU cycles per measurement\n";
    std::cout << "  - At 3 GHz CPU:         ~10 nanoseconds overhead\n";
    std::cout << "\n";
    std::cout << "This is LOW enough for HFT tick-to-trade measurement!\n";
    std::cout << "\n";
    
    return 0;
}
