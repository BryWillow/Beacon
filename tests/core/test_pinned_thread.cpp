// =============================================================================
// Project:      Beacon Tests
// File:         test_pinned_thread.cpp
// Purpose:      GoogleTest suite for pinned threads
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/concurrency/pinned_thread.h"
#include <atomic>

using namespace beacon::hft::concurrency;

TEST(PinnedThreadTest, BasicExecution) {
    std::atomic<int> counter{0};
    
    {
        PinnedThread thread([&](std::atomic<bool>& stop) {
            counter++;
        }, 0);
        // Destructor will join automatically when scope closes
    }
    
    EXPECT_EQ(counter, 1);
}

TEST(PinnedThreadTest, MultipleThreadsOnDifferentCores) {
    std::atomic<int> sum{0};
    
    {
        PinnedThread t1([&](std::atomic<bool>& stop) { 
            sum += 10; 
        }, 0);
        
        PinnedThread t2([&](std::atomic<bool>& stop) { 
            sum += 20; 
        }, 1);
        
        // Destructors will join automatically
    }
    
    EXPECT_EQ(sum, 30);
}
