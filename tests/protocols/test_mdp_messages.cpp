// =============================================================================
// Project:      Beacon Tests
// File:         test_mdp_messages.cpp
// Purpose:      GoogleTest suite for CME MDP protocol
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "libs/exchange_protocols/cme/market_data/mdp_mbo/current/messages_active.h"
#include <cstring>

using namespace beacon::cme::market_data;

// Test message sizes - critical for binary protocol parsing
TEST(MDPMessagesTest, MessageSizes) {
    EXPECT_EQ(sizeof(CmeMboAddOrder), 64);
    EXPECT_EQ(sizeof(CmeMboModifyOrder), 64);
    EXPECT_EQ(sizeof(CmeMboDeleteOrder), 32);  // DeleteOrder is smaller
    EXPECT_EQ(sizeof(CmeMboTrade), 64);
    EXPECT_EQ(sizeof(CmeMboBookClear), 64);
}

// Test CmeMboAddOrder structure
TEST(MDPMessagesTest, AddOrderConstruction) {
    CmeMboAddOrder msg{};
    
    msg.sequenceNumber = 12345;
    msg.orderId = 999888;
    std::strncpy(msg.symbol, "ESZ4", sizeof(msg.symbol));
    msg.price = 445000;  // $4450.00
    msg.quantity = 10;
    msg.side = 'B';
    msg.orderType = 'L';  // Limit
    
    EXPECT_EQ(msg.sequenceNumber, 12345);
    EXPECT_EQ(msg.orderId, 999888);
    EXPECT_STREQ(msg.symbol, "ESZ4");
    EXPECT_EQ(msg.price, 445000);
    EXPECT_EQ(msg.quantity, 10);
    EXPECT_EQ(msg.side, 'B');
    EXPECT_EQ(msg.orderType, 'L');
}

// Test CmeMboModifyOrder structure
TEST(MDPMessagesTest, ModifyOrderConstruction) {
    CmeMboModifyOrder msg{};
    
    msg.sequenceNumber = 67890;
    msg.orderId = 999888;
    std::strncpy(msg.symbol, "ESZ4", sizeof(msg.symbol));
    msg.price = 445050;  // $4450.50
    msg.quantity = 15;
    msg.side = 'B';
    msg.orderType = 'L';
    
    EXPECT_EQ(msg.sequenceNumber, 67890);
    EXPECT_EQ(msg.orderId, 999888);
    EXPECT_EQ(msg.price, 445050);
    EXPECT_EQ(msg.quantity, 15);
}

// Test CmeMboDeleteOrder structure
TEST(MDPMessagesTest, DeleteOrderConstruction) {
    CmeMboDeleteOrder msg{};
    
    msg.sequenceNumber = 11111;
    msg.orderId = 999888;
    std::strncpy(msg.symbol, "ESZ4", sizeof(msg.symbol));
    msg.side = 'B';
    
    EXPECT_EQ(msg.sequenceNumber, 11111);
    EXPECT_EQ(msg.orderId, 999888);
    EXPECT_STREQ(msg.symbol, "ESZ4");
    EXPECT_EQ(msg.side, 'B');
}

// Test CmeMboTrade structure
TEST(MDPMessagesTest, TradeConstruction) {
    CmeMboTrade msg{};
    
    msg.sequenceNumber = 22222;
    msg.orderId = 999888;
    std::strncpy(msg.symbol, "ESZ4", sizeof(msg.symbol));
    msg.price = 445000;
    msg.quantity = 5;
    msg.tradeId = 777666555;
    msg.side = 'B';
    
    EXPECT_EQ(msg.sequenceNumber, 22222);
    EXPECT_EQ(msg.orderId, 999888);
    EXPECT_EQ(msg.price, 445000);
    EXPECT_EQ(msg.quantity, 5);
    EXPECT_EQ(msg.tradeId, 777666555);
    EXPECT_EQ(msg.side, 'B');
}

// Test CmeMboBookClear structure
TEST(MDPMessagesTest, BookClearConstruction) {
    CmeMboBookClear msg{};
    
    msg.sequenceNumber = 33333;
    std::strncpy(msg.symbol, "ESZ4", sizeof(msg.symbol));
    
    EXPECT_EQ(msg.sequenceNumber, 33333);
    EXPECT_STREQ(msg.symbol, "ESZ4");
}

// Test symbol field handling (8 chars max)
TEST(MDPMessagesTest, SymbolFieldBoundary) {
    CmeMboAddOrder msg{};
    
    // Test 8-character symbol (boundary case)
    std::strncpy(msg.symbol, "ABCDEFGH", sizeof(msg.symbol));
    EXPECT_EQ(std::strlen(msg.symbol), 8);
    
    // Test shorter symbol with padding
    std::memset(msg.symbol, 0, sizeof(msg.symbol));
    std::strncpy(msg.symbol, "ESZ4", sizeof(msg.symbol));
    EXPECT_EQ(std::strlen(msg.symbol), 4);
}

// Test order ID uniqueness
TEST(MDPMessagesTest, OrderIdHandling) {
    CmeMboAddOrder addMsg{};
    CmeMboModifyOrder modMsg{};
    
    uint32_t orderId = 123456789;
    addMsg.orderId = orderId;
    modMsg.orderId = orderId;
    
    EXPECT_EQ(addMsg.orderId, modMsg.orderId);
    EXPECT_EQ(addMsg.orderId, 123456789);
}

// Test price precision (CME uses different precision per instrument)
TEST(MDPMessagesTest, PricePrecision) {
    CmeMboAddOrder msg{};
    
    // E-mini S&P 500: prices in quarters ($0.25 increments)
    // $4450.25 = 445025 (in cents)
    msg.price = 445025;
    double priceDollars = msg.price / 100.0;
    EXPECT_NEAR(priceDollars, 4450.25, 0.01);
}
