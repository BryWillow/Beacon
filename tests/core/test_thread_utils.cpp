// =============================================================================
// Project:      Beacon Tests
// File:         test_thread_utils.cpp
// Purpose:      GoogleTest suite for thread utilities
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/concurrency/thread_utils.h"
#include <thread>
#include <atomic>
#include <chrono>

using namespace beacon::hft::concurrency;

// Test that NO_CPU_PINNING constant exists
TEST(ThreadUtilsTest, NoCpuPinningConstant) {
    EXPECT_EQ(ThreadUtils::NO_CPU_PINNING, -1);
}

// Test basic pinThreadToCore functionality
// Note: On macOS this is a no-op, but we verify it doesn't crash
TEST(ThreadUtilsTest, PinThreadToCore) {
    std::atomic<bool> threadRan{false};
    
    std::thread t([&threadRan]() {
        threadRan = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
    
    // Should not throw or crash (even if it's a no-op on macOS)
    EXPECT_NO_THROW(ThreadUtils::pinThreadToCore(t, 0));
    
    t.join();
    EXPECT_TRUE(threadRan);
}

// Test with multiple cores
TEST(ThreadUtilsTest, PinToMultipleCores) {
    for (int core = 0; core < 4; ++core) {
        std::atomic<bool> threadRan{false};
        
        std::thread t([&threadRan]() {
            threadRan = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        });
        
        EXPECT_NO_THROW(ThreadUtils::pinThreadToCore(t, core));
        
        t.join();
        EXPECT_TRUE(threadRan);
    }
}

// Test with NO_CPU_PINNING value
TEST(ThreadUtilsTest, NoCpuPinningValue) {
    std::atomic<bool> threadRan{false};
    
    std::thread t([&threadRan]() {
        threadRan = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
    
    // Using NO_CPU_PINNING should also not crash
    EXPECT_NO_THROW(ThreadUtils::pinThreadToCore(t, ThreadUtils::NO_CPU_PINNING));
    
    t.join();
    EXPECT_TRUE(threadRan);
}
