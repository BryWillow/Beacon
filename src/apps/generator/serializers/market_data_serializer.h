/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Defines the interface for serializing market data messages into
 *               exchange-specific binary formats (ITCH, Pillar, MDP, etc.)
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "message.h"

namespace beacon::market_data_generator::serializers {

/// @brief Interface for serializing market data messages to exchange-specific formats.
/// @details Implementations handle binary serialization for different exchange protocols
///          (NASDAQ ITCH, NYSE Pillar, CME MDP MBO) with proper byte ordering and
///          format-specific field encodings.
class IMarketDataSerializer {
  public:
    virtual ~IMarketDataSerializer() = default;

    /// @brief Serialize a market data message to the exchange-specific binary format.
    /// @param message The message to serialize (contains type, symbol, price, quantity, etc.)
    /// @note This method writes directly to the output file configured in the constructor
    virtual void serializeMessage(const Message& message) const = 0;
};

} // namespace beacon::market_data_generator::serializers
