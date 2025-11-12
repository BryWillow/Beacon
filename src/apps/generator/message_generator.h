/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Orchestrates market data message generation with realistic
 *               market microstructure (bid-ask spreads, trades, order flow)
 *               and statistical tracking.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "stats_manager.h"
#include "symbol_parameters.h"
#include "serializers/market_data_serializer.h"
#include "config_provider.h"

#include <string>
#include <vector>
#include <memory>

namespace beaconserialize = beacon::market_data_generator::serializers;
namespace beaconconfig = beacon::market_data_generator::config;

namespace beacon::market_data_generator {

class MessageGenerator {
public:
    explicit MessageGenerator(const beaconconfig::ConfigProvider& configProvider);

    // Delete unwanted constructors
    MessageGenerator() = delete;
    MessageGenerator(const MessageGenerator&) = delete;
    MessageGenerator& operator=(const MessageGenerator&) = delete;
    MessageGenerator(MessageGenerator&&) = delete;
    MessageGenerator& operator=(MessageGenerator&&) = delete;

    void generateMessages(const std::string& outputPath, size_t numMessages, const std::string& configPath = "");
    size_t getMessageCount() const;

private:
    std::vector<SymbolParameters> _symbols;
    StatsManager _statsManager;
    size_t _messageCount = 10000;
    double _tradeProbability = 0.1;
    size_t _flushInterval = 1000;
    std::unique_ptr<beaconserialize::IMarketDataSerializer> _serializer;
};

} // namespace beacon::market_data_generator