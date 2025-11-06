// =============================================================================
// Project:      Beacon Tests
// File:         test_itch_serialization.cpp
// Purpose:      GoogleTest suite for ITCH serialization/deserialization
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "src/libs/exchange_protocols/nsdq/market_data/itch/v5/itch_message_types.h"
#include <cstring>
#include <vector>

using namespace beacon::nsdq::market_data::itch;

TEST(ITCHSerializationTest, SerializeAddOrder) {
    AddOrderMessage msg{};
    msg.sequenceNumber = 100;
    msg.orderRefNum = 200;
    std::strncpy(msg.stock, "MSFT", sizeof(msg.stock));
    msg.shares = 500;
    msg.price = 2755000;  // $275.50
    msg.side = 'S';
    
    // Serialize to bytes
    std::vector<uint8_t> buffer(sizeof(AddOrderMessage));
    std::memcpy(buffer.data(), &msg, sizeof(msg));
    
    EXPECT_EQ(buffer.size(), 33);
}

TEST(ITCHSerializationTest, DeserializeAddOrder) {
    // Create serialized data
    AddOrderMessage original{};
    original.sequenceNumber = 12345;
    original.orderRefNum = 67890;
    std::strncpy(original.stock, "TSLA", sizeof(original.stock));
    original.shares = 75;
    original.price = 2100000;  // $210.00
    original.side = 'B';
    
    std::vector<uint8_t> buffer(sizeof(AddOrderMessage));
    std::memcpy(buffer.data(), &original, sizeof(original));
    
    // Deserialize
    AddOrderMessage deserialized{};
    std::memcpy(&deserialized, buffer.data(), sizeof(deserialized));
    
    EXPECT_EQ(deserialized.sequenceNumber, original.sequenceNumber);
    EXPECT_EQ(deserialized.orderRefNum, original.orderRefNum);
    EXPECT_STREQ(deserialized.stock, original.stock);
    EXPECT_EQ(deserialized.shares, original.shares);
    EXPECT_EQ(deserialized.price, original.price);
    EXPECT_EQ(deserialized.side, original.side);
}

TEST(ITCHSerializationTest, RoundTripMultipleMessages) {
    std::vector<AddOrderMessage> messages;
    
    for (int i = 0; i < 10; ++i) {
        AddOrderMessage msg{};
        msg.sequenceNumber = i * 100;
        msg.orderRefNum = i * 1000;
        std::strncpy(msg.stock, "AAPL", sizeof(msg.stock));
        msg.shares = 100 + i * 10;
        msg.price = 1500000 + i * 1000;
        msg.side = (i % 2 == 0) ? 'B' : 'S';
        messages.push_back(msg);
    }
    
    // Serialize all
    std::vector<uint8_t> buffer(messages.size() * sizeof(AddOrderMessage));
    for (size_t i = 0; i < messages.size(); ++i) {
        std::memcpy(buffer.data() + i * sizeof(AddOrderMessage),
                   &messages[i], sizeof(AddOrderMessage));
    }
    
    // Deserialize all
    std::vector<AddOrderMessage> deserialized(messages.size());
    for (size_t i = 0; i < messages.size(); ++i) {
        std::memcpy(&deserialized[i],
                   buffer.data() + i * sizeof(AddOrderMessage),
                   sizeof(AddOrderMessage));
    }
    
    // Verify all match
    for (size_t i = 0; i < messages.size(); ++i) {
        EXPECT_EQ(deserialized[i].sequenceNumber, messages[i].sequenceNumber);
        EXPECT_EQ(deserialized[i].shares, messages[i].shares);
        EXPECT_EQ(deserialized[i].price, messages[i].price);
        EXPECT_EQ(deserialized[i].side, messages[i].side);
    }
}
