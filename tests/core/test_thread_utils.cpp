// =============================================================================
// Project:      Beacon Tests
// File:         test_thread_utils.cpp
// Purpose:      GoogleTest suite for thread utilities
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/concurrency/thread_utils.h"
#include <thread>

using namespace hft::concurrency;

TEST(ThreadUtilsTest, SetThreadAffinity) {
    std::thread t([&]() {
        EXPECT_TRUE(set_thread_affinity(0));
    });
    t.join();
}

TEST(ThreadUtilsTest, InvalidCoreAffinity) {
    std::thread t([&]() {
        // Core 999 doesn't exist
        EXPECT_FALSE(set_thread_affinity(999));
    });
    t.join();
}

TEST(ThreadUtilsTest, SetThreadPriority) {
    std::thread t([&]() {
        EXPECT_TRUE(set_realtime_priority());
    });
    t.join();
}
