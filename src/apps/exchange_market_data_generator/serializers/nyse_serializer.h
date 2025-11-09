/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      NYSE Pillar v1.9 binary serializer - converts market data
 *               messages into NYSE Pillar format with proper message types
 *               and fixed-point price encoding.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "market_data_serializer.h"

#include <string>
#include <fstream>

namespace beacon::market_data_generator::serializers {

class NyseMarketDataSerializer : public IMarketDataSerializer {
  public:
    /// @brief Constructor.
    /// @param outputFilePath The file path to write serialized messages.
    explicit NyseMarketDataSerializer(const std::string& outputFilePath);

    /// @brief Destructor - ensures file is properly closed.
    ~NyseMarketDataSerializer();

    /// @brief Serialize a market data message.
    /// @param message The message to serialize.
    void serializeMessage(const Message& message) const override;

  private:
    std::string _outputFilePath;
    mutable std::ofstream _outputFile;
};

} // namespace beacon::market_data_generator::serializers