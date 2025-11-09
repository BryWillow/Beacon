/*
 * =============================================================================
 * Project:      Beacon
 * Application:  md_generator
 * Purpose:      Defines an interface for serializing market data messages
 *               into exchange-specific formats.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "message.h"
#include <string>

namespace beacon::market_data_generator::serializers {

/// @brief Interface for serializing market data messages.
/// @details This interface defines a method for serializing messages into
/// exchange-specific formats.
class ISerializeMarketDataMessages {
  public:
    /// @brief Virtual destructor.
    virtual ~ISerializeMarketDataMessages() = default;

    /// @brief Serialize a market data message.
    /// @param message The message to serialize.
    virtual void serializeMessage(const Message& message) const = 0;
};

} // namespace beacon::market_data_generator::serializers