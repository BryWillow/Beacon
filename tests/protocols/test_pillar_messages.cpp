// =============================================================================
// Project:      Beacon Tests
// File:         test_pillar_messages.cpp
// Purpose:      GoogleTest suite for NYSE Pillar protocol
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "libs/exchange_protocols/nyse/market_data/pillar_market_data/current/messages_active.h"
#include <cstring>

using namespace beacon::nyse::market_data;

// Test message sizes - critical for binary protocol parsing
TEST(PillarMessagesTest, MessageSizes) {
    EXPECT_EQ(sizeof(NyseQuoteMessage), 64);
    EXPECT_EQ(sizeof(NyseBookUpdateMessage), 64);
    EXPECT_EQ(sizeof(NyseTradeMessage), 64);
}

// Test NyseQuoteMessage structure
TEST(PillarMessagesTest, QuoteMessageConstruction) {
    NyseQuoteMessage msg{};
    
    msg.sequenceNumber = 12345;
    std::strncpy(msg.symbol, "AAPL", sizeof(msg.symbol));
    msg.bidPrice = 15025000;  // $150.25
    msg.bidSize = 100;
    msg.askPrice = 15026000;  // $150.26
    msg.askSize = 200;
    
    EXPECT_EQ(msg.sequenceNumber, 12345);
    EXPECT_STREQ(msg.symbol, "AAPL");
    EXPECT_EQ(msg.bidPrice, 15025000);
    EXPECT_EQ(msg.bidSize, 100);
    EXPECT_EQ(msg.askPrice, 15026000);
    EXPECT_EQ(msg.askSize, 200);
}

// Test NyseBookUpdateMessage structure
TEST(PillarMessagesTest, BookUpdateMessageConstruction) {
    NyseBookUpdateMessage msg{};
    
    msg.sequenceNumber = 67890;
    std::strncpy(msg.symbol, "MSFT", sizeof(msg.symbol));
    msg.price = 30050000;  // $300.50
    msg.size = 500;
    msg.level = 1;
    msg.side = 'B';  // Bid
    msg.updateType = 'A';  // Add
    
    EXPECT_EQ(msg.sequenceNumber, 67890);
    EXPECT_STREQ(msg.symbol, "MSFT");
    EXPECT_EQ(msg.price, 30050000);
    EXPECT_EQ(msg.size, 500);
    EXPECT_EQ(msg.level, 1);
    EXPECT_EQ(msg.side, 'B');
    EXPECT_EQ(msg.updateType, 'A');
}

// Test NyseTradeMessage structure
TEST(PillarMessagesTest, TradeMessageConstruction) {
    NyseTradeMessage msg{};
    
    msg.sequenceNumber = 11111;
    std::strncpy(msg.symbol, "GOOGL", sizeof(msg.symbol));
    msg.price = 14000000;  // $1400.00
    msg.shares = 100;
    msg.tradeId = 999888777;
    msg.side = 'B';
    
    EXPECT_EQ(msg.sequenceNumber, 11111);
    EXPECT_STREQ(msg.symbol, "GOOGL");
    EXPECT_EQ(msg.price, 14000000);
    EXPECT_EQ(msg.shares, 100);
    EXPECT_EQ(msg.tradeId, 999888777);
    EXPECT_EQ(msg.side, 'B');
}

// Test symbol field handling (8 chars max)
TEST(PillarMessagesTest, SymbolFieldBoundary) {
    NyseQuoteMessage msg{};
    
    // Test 8-character symbol (boundary case)
    std::strncpy(msg.symbol, "ABCDEFGH", sizeof(msg.symbol));
    EXPECT_EQ(std::strlen(msg.symbol), 8);
    
    // Test shorter symbol with padding
    std::memset(msg.symbol, 0, sizeof(msg.symbol));
    std::strncpy(msg.symbol, "TSLA", sizeof(msg.symbol));
    EXPECT_EQ(std::strlen(msg.symbol), 4);
}

// Test price precision (prices in 1/100000 dollars)
TEST(PillarMessagesTest, PricePrecision) {
    NyseQuoteMessage msg{};
    
    // Test precise price: $150.25500
    msg.bidPrice = 15025500;
    double bidPriceDollars = msg.bidPrice / 100000.0;
    EXPECT_NEAR(bidPriceDollars, 150.255, 0.00001);
    
    // Convert back
    uint32_t reconstructedPrice = static_cast<uint32_t>(bidPriceDollars * 100000);
    EXPECT_EQ(reconstructedPrice, 15025500);
}
