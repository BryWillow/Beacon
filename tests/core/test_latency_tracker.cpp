// =============================================================================
// Project:      Beacon Tests
// File:         test_latency_tracker.cpp
// Purpose:      GoogleTest suite for latency tracking
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/profiling/latency_tracker.h"
#include <thread>
#include <chrono>

using namespace hft::profiling;

TEST(LatencyTrackerTest, BasicRecording) {
    LatencyTracker tracker;
    
    tracker.record(100);
    tracker.record(200);
    tracker.record(300);
    
    auto stats = tracker.get_statistics();
    
    EXPECT_EQ(stats.count, 3);
    EXPECT_EQ(stats.min, 100);
    EXPECT_EQ(stats.max, 300);
    EXPECT_DOUBLE_EQ(stats.mean, 200.0);
}

TEST(LatencyTrackerTest, Percentiles) {
    LatencyTracker tracker;
    
    // Record 1-100
    for (int i = 1; i <= 100; ++i) {
        tracker.record(i);
    }
    
    auto stats = tracker.get_statistics();
    
    EXPECT_EQ(stats.count, 100);
    EXPECT_EQ(stats.min, 1);
    EXPECT_EQ(stats.max, 100);
    EXPECT_NEAR(stats.p50, 50, 5);
    EXPECT_NEAR(stats.p95, 95, 5);
    EXPECT_NEAR(stats.p99, 99, 2);
}

TEST(LatencyTrackerTest, MicrosecondPrecision) {
    LatencyTracker tracker;
    
    auto start = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    auto end = std::chrono::high_resolution_clock::now();
    
    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
        end - start
    ).count();
    
    tracker.record(latency);
    
    auto stats = tracker.get_statistics();
    
    EXPECT_GE(stats.min, 90);   // Should be close to 100μs
    EXPECT_LE(stats.max, 150);  // Allow some jitter
}

TEST(LatencyTrackerTest, Reset) {
    LatencyTracker tracker;
    
    tracker.record(100);
    tracker.record(200);
    
    tracker.reset();
    
    auto stats = tracker.get_statistics();
    EXPECT_EQ(stats.count, 0);
}

TEST(LatencyTrackerTest, LargeDataset) {
    LatencyTracker tracker;
    
    // Record 10,000 samples
    for (int i = 0; i < 10000; ++i) {
        tracker.record(i % 1000);  // 0-999 pattern
    }
    
    auto stats = tracker.get_statistics();
    
    EXPECT_EQ(stats.count, 10000);
    EXPECT_GE(stats.min, 0);
    EXPECT_LE(stats.max, 999);
}
