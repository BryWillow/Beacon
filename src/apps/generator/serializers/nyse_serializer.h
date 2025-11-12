/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      NYSE-specific serializer for market data generation. Inherits
 *               from the base IMarketDataSerializer interface. Implements the
 *               serialize method to output market data in the format required
 *               by the NYSE.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "serializers/serializer_base.h"

namespace beacon::market_data_generator::serializers {

class NyseMarketDataSerializer : public IMarketDataSerializer {
public:
    NyseMarketDataSerializer(const std::string& outputFilePath);

    void serialize(const MarketDataMessage& message) override;

private:
    std::ofstream _outputFile;
};

} // namespace beacon::market_data_generator::serializers