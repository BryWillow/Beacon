#pragma once

#include <cstdint>
#include <chrono>
#include <algorithm>
#include <cstring>

#if defined(__x86_64__) || defined(_M_X64)
#include <x86intrin.h>
#define HAS_RDTSC 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#define HAS_RDTSC 0
#else
#define HAS_RDTSC 0
#endif

namespace beacon {
namespace hft {
namespace profiling {

// ============================================================================
// High-Resolution Timer (TSC on x86, system timer on ARM)
// ============================================================================

class HighResTimer {
public:
    using Timestamp = uint64_t;
    
    // Get current timestamp (CPU cycles on x86, nanoseconds on ARM/fallback)
    static inline Timestamp now() noexcept {
#if HAS_RDTSC
        return __rdtsc();  // ~20 CPU cycles
#else
        // Fallback to chrono (slower but portable)
        return std::chrono::high_resolution_clock::now().time_since_epoch().count();
#endif
    }
    
    // Convert timestamp difference to microseconds
    static inline double toMicroseconds(Timestamp delta) noexcept {
#if HAS_RDTSC
        // Calibrate once at startup (assumes ~3.0 GHz CPU)
        // For production, calibrate dynamically
        static const double CYCLES_PER_US = calibrateTSC();
        return static_cast<double>(delta) / CYCLES_PER_US;
#else
        // Already in nanoseconds
        return static_cast<double>(delta) / 1000.0;
#endif
    }
    
private:
    static double calibrateTSC() noexcept {
#if HAS_RDTSC
        // Simple calibration: measure TSC ticks over 100ms
        auto start_ts = __rdtsc();
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Sleep for 100ms
        auto end_time = start_time + std::chrono::milliseconds(100);
        while (std::chrono::high_resolution_clock::now() < end_time) {
            // Busy wait
        }
        
        auto end_ts = __rdtsc();
        auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now() - start_time).count();
        
        double cycles_per_ns = static_cast<double>(end_ts - start_ts) / elapsed_ns;
        return cycles_per_ns * 1000.0;  // cycles per microsecond
#else
        return 1.0;
#endif
    }
};

// ============================================================================
// Lightweight Latency Tracker
// ============================================================================

template<size_t MAX_SAMPLES = 1000000>
class alignas(64) LatencyTracker {
public:
    using Timestamp = HighResTimer::Timestamp;
    
    LatencyTracker() : _count(0), _index(0) {
        std::memset(_samples, 0, sizeof(_samples));
    }
    
    // ========== HOT PATH ==========
    // Record a latency sample (timestamp difference)
    // INLINE, NO ALLOCATION, NO LOCKS
    inline void record(Timestamp start, Timestamp end) noexcept {
        if (_index < MAX_SAMPLES) {
            _samples[_index++] = end - start;
        }
        _count++;
    }
    
    // Alternative: record delta directly
    inline void recordDelta(Timestamp delta) noexcept {
        if (_index < MAX_SAMPLES) {
            _samples[_index++] = delta;
        }
        _count++;
    }
    // ========== END HOT PATH ==========
    
    // Get statistics (call this OFFLINE, not in hot path)
    struct Stats {
        uint64_t count;
        uint64_t samples_recorded;
        double min_us;
        double max_us;
        double mean_us;
        double median_us;
        double p95_us;
        double p99_us;
        double p999_us;
    };
    
    Stats getStats() const {
        Stats stats{};
        stats.count = _count;
        stats.samples_recorded = std::min(_index, MAX_SAMPLES);
        
        if (stats.samples_recorded == 0) {
            return stats;
        }
        
        // Copy samples for sorting (don't mutate original)
        uint64_t* sorted = new uint64_t[stats.samples_recorded];
        std::memcpy(sorted, _samples, stats.samples_recorded * sizeof(uint64_t));
        std::sort(sorted, sorted + stats.samples_recorded);
        
        // Convert to microseconds
        stats.min_us = HighResTimer::toMicroseconds(sorted[0]);
        stats.max_us = HighResTimer::toMicroseconds(sorted[stats.samples_recorded - 1]);
        
        // Mean
        uint64_t sum = 0;
        for (size_t i = 0; i < stats.samples_recorded; ++i) {
            sum += sorted[i];
        }
        stats.mean_us = HighResTimer::toMicroseconds(sum / stats.samples_recorded);
        
        // Percentiles
        auto percentile = [&](double p) {
            size_t idx = static_cast<size_t>(p * stats.samples_recorded);
            if (idx >= stats.samples_recorded) idx = stats.samples_recorded - 1;
            return HighResTimer::toMicroseconds(sorted[idx]);
        };
        
        stats.median_us = percentile(0.50);
        stats.p95_us = percentile(0.95);
        stats.p99_us = percentile(0.99);
        stats.p999_us = percentile(0.999);
        
        delete[] sorted;
        return stats;
    }
    
    // Reset tracker
    void reset() {
        _count = 0;
        _index = 0;
    }
    
    // Check if buffer is full
    bool isFull() const { return _index >= MAX_SAMPLES; }
    
    // Get current sample count
    uint64_t getCount() const { return _count; }
    
private:
    alignas(64) uint64_t _samples[MAX_SAMPLES];  // Pre-allocated sample buffer
    uint64_t _count;   // Total events (may exceed MAX_SAMPLES)
    size_t _index;     // Current write index
};

// ============================================================================
// Scoped Latency Measurement (RAII)
// ============================================================================

class ScopedLatency {
public:
    explicit ScopedLatency(LatencyTracker<>& tracker) 
        : _tracker(tracker), _start(HighResTimer::now()) {}
    
    ~ScopedLatency() {
        auto end = HighResTimer::now();
        _tracker.record(_start, end);
    }
    
    // Delete copy/move to prevent misuse
    ScopedLatency(const ScopedLatency&) = delete;
    ScopedLatency& operator=(const ScopedLatency&) = delete;
    
private:
    LatencyTracker<>& _tracker;
    HighResTimer::Timestamp _start;
};

} // namespace profiling
} // namespace hft
} // namespace beacon
