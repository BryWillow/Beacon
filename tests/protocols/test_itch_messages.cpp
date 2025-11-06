// =============================================================================
// Project:      Beacon Tests
// File:         test_itch_messages.cpp
// Purpose:      GoogleTest suite for ITCH message structures
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "src/libs/exchange_protocols/nsdq/market_data/itch/v5/itch_message_types.h"
#include <cstring>

using namespace beacon::nsdq::market_data::itch;

TEST(ITCHMessagesTest, AddOrderMessageSize) {
    // Verify packed structure size
    EXPECT_EQ(sizeof(AddOrderMessage), 33);
}

TEST(ITCHMessagesTest, AddOrderConstruction) {
    AddOrderMessage msg{};
    msg.sequenceNumber = 12345;
    msg.orderRefNum = 98765;
    std::strncpy(msg.stock, "AAPL", sizeof(msg.stock));
    msg.shares = 100;
    msg.price = 1502500;  // $150.25 in 1/10000 dollars
    msg.side = 'B';
    
    EXPECT_EQ(msg.sequenceNumber, 12345);
    EXPECT_EQ(msg.orderRefNum, 98765);
    EXPECT_STREQ(msg.stock, "AAPL");
    EXPECT_EQ(msg.shares, 100);
    EXPECT_EQ(msg.price, 1502500);
    EXPECT_EQ(msg.side, 'B');
}

TEST(ITCHMessagesTest, TradeMessageSize) {
    EXPECT_EQ(sizeof(TradeMessage), 33);
}

TEST(ITCHMessagesTest, MessageTypeCodes) {
    EXPECT_EQ(static_cast<uint8_t>(MessageType::AddOrder), 'A');
    EXPECT_EQ(static_cast<uint8_t>(MessageType::Trade), 'P');
    EXPECT_EQ(static_cast<uint8_t>(MessageType::OrderExecuted), 'E');
    EXPECT_EQ(static_cast<uint8_t>(MessageType::OrderCancel), 'X');
    EXPECT_EQ(static_cast<uint8_t>(MessageType::OrderDelete), 'D');
}

TEST(ITCHMessagesTest, PriceConversion) {
    // Test price representation: $150.2534 -> 1502534
    double price_dollars = 150.2534;
    uint32_t price_itch = static_cast<uint32_t>(price_dollars * 10000);
    
    EXPECT_EQ(price_itch, 1502534);
    
    // Convert back
    double converted_back = price_itch / 10000.0;
    EXPECT_NEAR(converted_back, price_dollars, 0.0001);
}

TEST(ITCHMessagesTest, StockSymbolPadding) {
    AddOrderMessage msg{};
    
    // Test short symbol
    std::strncpy(msg.stock, "AAPL", sizeof(msg.stock));
    EXPECT_EQ(std::strlen(msg.stock), 4);
    
    // Test 8-character symbol
    std::strncpy(msg.stock, "ABCDEFGH", sizeof(msg.stock));
    EXPECT_EQ(std::strlen(msg.stock), 8);
}
