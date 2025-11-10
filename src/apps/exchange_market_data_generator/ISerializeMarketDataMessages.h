/*
 * Project: Beacon
 * Application: md_generator
 * Purpose: Interface for serializing market data messages.
 * Author: Bryan Camp
 */

#pragma once

#include <string>

class ISerializeMarketDataMessages {
public:
    virtual ~ISerializeMarketDataMessages() = default;

    // Serialize a market data message
    virtual void serializeMessage(const std::string& message) = 0;

    // Deserialize a market data message
    virtual std::string deserializeMessage(const std::string& data) = 0;
};
