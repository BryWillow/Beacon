/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Interface for market data serializers. This is used by the
 *               ConfigProvider to create exchange-specific serializers.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <string>
#include <memory>
#include "config_provider.h"

namespace beacon::market_data_generator::serializers {

class IMarketDataSerializer {
public:
    virtual ~IMarketDataSerializer() = default;

    // Serialize a batch of market data messages
    virtual void serializeBatch(
        const std::vector<beacon::market_data_generator::config::SymbolData>& symbols,
        size_t messageCount,
        double tradeProbability
    ) = 0;
};

// Factory function to create the appropriate serializer instance
std::unique_ptr<IMarketDataSerializer> createSerializer(
    const std::string& exchangeType,
    const std::string& outputFilePath,
    size_t flushInterval = 1000
);

} // namespace beacon::market_data_generator::serializers