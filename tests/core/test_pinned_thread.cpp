// =============================================================================
// Project:      Beacon Tests
// File:         test_pinned_thread.cpp
// Purpose:      GoogleTest suite for pinned threads
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/concurrency/pinned_thread.h"
#include <atomic>

using namespace hft::concurrency;

TEST(PinnedThreadTest, BasicExecution) {
    std::atomic<int> counter{0};
    
    PinnedThread thread(0, [&]() {
        counter++;
    });
    
    thread.join();
    
    EXPECT_EQ(counter, 1);
}

TEST(PinnedThreadTest, MultipleThreadsOnDifferentCores) {
    std::atomic<int> sum{0};
    
    PinnedThread t1(0, [&]() { sum += 10; });
    PinnedThread t2(1, [&]() { sum += 20; });
    
    t1.join();
    t2.join();
    
    EXPECT_EQ(sum, 30);
}
