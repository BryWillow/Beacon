/*
 * =============================================================================
 * Project:      Beacon
 * Application:  md_generator
 * Purpose:      Defines the ITCH serializer for market data messages.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "iserialize_market_data_messages.h"
#include <string>
#include <fstream>

namespace beacon::market_data_generator::serializers {

/// @brief ITCH serializer for market data messages.
class ItchMarketDataSerializer : public ISerializeMarketDataMessages {
  public:
    /// @brief Constructor.
    /// @param outputFilePath The file path to write serialized messages.
    explicit ItchMarketDataSerializer(const std::string& outputFilePath);

    /// @brief Destructor - ensures file is properly closed.
    ~ItchMarketDataSerializer();

    /// @brief Serialize a market data message.
    /// @param message The message to serialize.
    void serializeMessage(const Message& message) const override;

  private:
    std::string _outputFilePath;
    mutable std::ofstream _outputFile;
};

} // namespace beacon::market_data_generator::serializers