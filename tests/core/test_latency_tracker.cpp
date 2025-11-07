// =============================================================================
// Project:      Beacon Tests
// File:         test_latency_tracker.cpp
// Purpose:      GoogleTest suite for latency tracking
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/profiling/latency_tracker.h"

using namespace beacon::hft::profiling;

TEST(LatencyTrackerTest, BasicRecording) {
    LatencyTracker<> tracker;
    
    auto start1 = HighResTimer::now();
    auto end1 = start1 + 100;
    tracker.record(start1, end1);
    
    auto start2 = HighResTimer::now();
    auto end2 = start2 + 200;
    tracker.record(start2, end2);
    
    auto start3 = HighResTimer::now();
    auto end3 = start3 + 300;
    tracker.record(start3, end3);
    
    auto stats = tracker.getStats();
    
    EXPECT_EQ(stats.count, 3);
    EXPECT_EQ(stats.samples_recorded, 3);
}

TEST(LatencyTrackerTest, Percentiles) {
    LatencyTracker<> tracker;
    
    // Record 1-100
    for (uint64_t i = 1; i <= 100; i++) {
        auto start = HighResTimer::now();
        auto end = start + (i * 1000);
        tracker.record(start, end);
    }
    
    auto stats = tracker.getStats();
    
    EXPECT_EQ(stats.count, 100);
    EXPECT_GT(stats.max_us, stats.min_us);
    EXPECT_GT(stats.p95_us, stats.median_us);
    EXPECT_GT(stats.p99_us, stats.p95_us);
}

TEST(LatencyTrackerTest, RecordDelta) {
    LatencyTracker<> tracker;
    
    // Use recordDelta for pre-computed latencies
    tracker.recordDelta(100);
    tracker.recordDelta(200);
    tracker.recordDelta(300);
    
    auto stats = tracker.getStats();
    
    EXPECT_EQ(stats.count, 3);
    EXPECT_EQ(stats.samples_recorded, 3);
}

TEST(LatencyTrackerTest, LargeDataset) {
    LatencyTracker<> tracker;
    
    // Record 1,000 samples
    for (uint64_t i = 0; i < 1000; i++) {
        auto start = HighResTimer::now();
        auto end = start + (i % 1000) * 100;
        tracker.record(start, end);
    }
    
    auto stats = tracker.getStats();
    
    EXPECT_EQ(stats.count, 1000);
    EXPECT_EQ(stats.samples_recorded, 1000);
}

