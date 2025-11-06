/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      NASDAQ ITCH v5.0 binary serializer - converts market data
 *               messages into NASDAQ ITCH format with proper byte ordering
 *               and fixed-point price encoding (1/10000 dollar increments).
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "market_data_serializer.h"

#include <string>
#include <fstream>

namespace beacon::market_data_generator::serializers {

class NsdqMarketDataSerializer : public IMarketDataSerializer {
  public:
    /// @brief Constructor.
    /// @param outputPath The file path to write serialized messages.
    /// @param flushInterval Number of messages between buffer flushes (default: 1000).
    explicit NsdqMarketDataSerializer(const std::string& outputPath, size_t flushInterval = 1000);

    /// @brief Destructor - ensures file is properly closed.
    ~NsdqMarketDataSerializer();

    /// @brief Serialize a market data message.
    /// @param message The message to serialize.
    void serializeMessage(const Message& message) const override;

  private:
    std::string _outputPath;
    size_t _flushInterval;
    mutable std::ofstream _outputFile;
};

} // namespace beacon::market_data_generator::serializers
