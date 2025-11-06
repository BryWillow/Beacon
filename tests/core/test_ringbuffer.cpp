// =============================================================================
// Project:      Beacon Tests
// File:         test_ringbuffer.cpp
// Purpose:      GoogleTest suite for SPSC ringbuffer
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/ringbuffer/spsc_ringbuffer.h"
#include <thread>
#include <vector>

using namespace beacon::hft::ringbuffer;

// =============================================================================
// Basic Functionality Tests
// =============================================================================

TEST(RingbufferTest, Construction) {
    SpScRingBuffer<int, 1024> rb;
    // If we get here, construction succeeded
    EXPECT_TRUE(true);
}

TEST(RingbufferTest, SingleProducerSingleConsumer) {
    SpScRingBuffer<int, 8> rb;
    
    // Push elements
    EXPECT_TRUE(rb.tryPush(1));
    EXPECT_TRUE(rb.tryPush(2));
    EXPECT_TRUE(rb.tryPush(3));
    
    // Pop elements
    int val;
    EXPECT_TRUE(rb.tryPop(val));
    EXPECT_EQ(val, 1);
    
    EXPECT_TRUE(rb.tryPop(val));
    EXPECT_EQ(val, 2);
    
    EXPECT_TRUE(rb.tryPop(val));
    EXPECT_EQ(val, 3);
    
    // Should be empty now
    EXPECT_FALSE(rb.tryPop(val));
}

TEST(RingbufferTest, FillCapacity) {
    SpScRingBuffer<int, 8> rb;
    
    // Fill to capacity (size - 1 due to implementation)
    for (int i = 0; i < 7; ++i) {
        EXPECT_TRUE(rb.tryPush(i)) << "Failed at index " << i;
    }
    
    // Should be full now
    EXPECT_FALSE(rb.tryPush(999));
    
    // Drain one
    int val;
    EXPECT_TRUE(rb.tryPop(val));
    EXPECT_EQ(val, 0);
    
    // Should be able to push one more
    EXPECT_TRUE(rb.tryPush(777));
}

TEST(RingbufferTest, WrapAround) {
    SpScRingBuffer<int, 8> rb;
    
    // Fill and drain multiple times to test wrap-around
    for (int cycle = 0; cycle < 10; ++cycle) {
        for (int i = 0; i < 5; ++i) {
            EXPECT_TRUE(rb.tryPush(cycle * 100 + i));
        }
        
        for (int i = 0; i < 5; ++i) {
            int val;
            EXPECT_TRUE(rb.tryPop(val));
            EXPECT_EQ(val, cycle * 100 + i);
        }
    }
}

// =============================================================================
// Thread Safety Tests
// =============================================================================

TEST(RingbufferTest, ConcurrentProducerConsumer) {
    SpScRingBuffer<int, 1024> rb;
    constexpr int NUM_MESSAGES = 100000;
    std::atomic<bool> producer_done{false};
    std::atomic<int> messages_consumed{0};
    
    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < NUM_MESSAGES; ++i) {
            while (!rb.tryPush(i)) {
                // Spin until space available
                std::this_thread::yield();
            }
        }
        producer_done = true;
    });
    
    // Consumer thread
    std::thread consumer([&]() {
        int expected = 0;
        while (expected < NUM_MESSAGES) {
            int val;
            if (rb.tryPop(val)) {
                EXPECT_EQ(val, expected) << "Out of order message";
                expected++;
                messages_consumed++;
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_EQ(messages_consumed, NUM_MESSAGES);
}

// =============================================================================
// Performance Tests
// =============================================================================

TEST(RingbufferTest, HighThroughput) {
    SpScRingBuffer<uint64_t, 4096> rb;
    constexpr int NUM_MESSAGES = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread producer([&]() {
        for (uint64_t i = 0; i < NUM_MESSAGES; ++i) {
            while (!rb.tryPush(i)) {
                // Spin
            }
        }
    });
    
    std::thread consumer([&]() {
        uint64_t received = 0;
        while (received < NUM_MESSAGES) {
            uint64_t val;
            if (rb.tryPop(val)) {
                received++;
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double throughput = (NUM_MESSAGES * 1000000.0) / duration.count();
    
    std::cout << "Ringbuffer throughput: " 
              << throughput << " msgs/sec" << std::endl;
    std::cout << "Average latency: " 
              << (duration.count() / (double)NUM_MESSAGES) << " μs" << std::endl;
    
    // Should handle at least 1M msgs/sec
    EXPECT_GT(throughput, 1000000);
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST(RingbufferTest, PopFromEmpty) {
    SpScRingBuffer<int, 8> rb;
    int val;
    EXPECT_FALSE(rb.tryPop(val));
}

TEST(RingbufferTest, PushToFull) {
    SpScRingBuffer<int, 4> rb;
    
    // Fill it up (size - 1)
    EXPECT_TRUE(rb.tryPush(1));
    EXPECT_TRUE(rb.tryPush(2));
    EXPECT_TRUE(rb.tryPush(3));
    
    // Should fail
    EXPECT_FALSE(rb.tryPush(4));
}

TEST(RingbufferTest, AlternatingPushPop) {
    SpScRingBuffer<int, 8> rb;
    
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(rb.tryPush(i));
        
        int val;
        EXPECT_TRUE(rb.tryPop(val));
        EXPECT_EQ(val, i);
    }
}

// =============================================================================
// Complex Data Types
// =============================================================================

struct ComplexMessage {
    uint64_t timestamp;
    double price;
    int quantity;
    char symbol[8];
    
    bool operator==(const ComplexMessage& other) const {
        return timestamp == other.timestamp &&
               price == other.price &&
               quantity == other.quantity &&
               std::string(symbol) == std::string(other.symbol);
    }
};

TEST(RingbufferTest, ComplexDataTypes) {
    SpScRingBuffer<ComplexMessage, 16> rb;
    
    ComplexMessage msg1{123456789, 150.25, 100, "AAPL"};
    ComplexMessage msg2{987654321, 275.50, 200, "MSFT"};
    
    EXPECT_TRUE(rb.tryPush(msg1));
    EXPECT_TRUE(rb.tryPush(msg2));
    
    ComplexMessage recv1, recv2;
    EXPECT_TRUE(rb.tryPop(recv1));
    EXPECT_TRUE(rb.tryPop(recv2));
    
    EXPECT_EQ(recv1, msg1);
    EXPECT_EQ(recv2, msg2);
}
