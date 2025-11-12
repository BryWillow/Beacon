/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Implements configuration loading from JSON files and factory
 *               methods for creating exchange-specific serializers. Validates
 *               all configuration parameters and provides defaults.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <iostream>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "config_provider.h"
#include "serializers/nsdq_serializer.h"
#include "serializers/cme_serializer.h"
#include "serializers/nyse_serializer.h"

using json = nlohmann::json;

namespace beacon::market_data_generator::config {

ConfigProvider::ConfigProvider(const std::string& exchangeType, const std::string& outputFilePath)
    : _exchangeType(exchangeType), _outputFilePath(outputFilePath), _exchange(exchangeType) {
    // Ensure the exchange is converted to lowercase for consistency
    std::transform(_exchange.begin(), _exchange.end(), _exchange.begin(), ::tolower);
}

bool ConfigProvider::loadConfig(const std::string& configName) {
    // Use an environment variable or fallback to relative path for config directory
    std::string configRoot;
    const char* envConfigRoot = std::getenv("BEACON_CONFIG_ROOT");
    if (envConfigRoot) {
        configRoot = envConfigRoot;
    } else {
        configRoot = "config/playback/";
    }
    std::string configPath = configRoot + configName;
    std::cout << "[ConfigProvider] Attempting to load config file: " << configPath << std::endl;
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        throw std::runtime_error("Failed to open config file: " + configPath);
    }

    json configJson;
    configFile >> configJson;

    // Parse num_messages
    if (configJson.contains("num_messages")) {
        _messageCount = configJson["num_messages"].get<size_t>();
        if (_messageCount == 0) {
            throw std::runtime_error("'num_messages' must be greater than 0.");
        }
    } else {
        throw std::runtime_error("Missing 'num_messages' field in config file.");
    }

    // Parse trade_probability (optional, default 0.1)
    if (configJson.contains("trade_probability")) {
        _tradeProbability = configJson["trade_probability"].get<double>();
        if (_tradeProbability < 0.0 || _tradeProbability > 1.0) {
            throw std::runtime_error("'trade_probability' must be between 0.0 and 1.0.");
        }
    }

    // Parse flush_interval (optional, default 1000)
    if (configJson.contains("flush_interval")) {
        _flushInterval = configJson["flush_interval"].get<size_t>();
        if (_flushInterval == 0) {
            throw std::runtime_error("'flush_interval' must be greater than 0.");
        }
    }

    // Parse default_spread_percent (optional, default 0.5)
    if (configJson.contains("spread_percentage")) {
        _defaultSpreadPercent = configJson["spread_percentage"].get<double>();
        if (_defaultSpreadPercent < 0.0) {
            throw std::runtime_error("'spread_percentage' must be non-negative.");
        }
    }

    // Parse exchange
    if (configJson.contains("exchange")) {
        _exchange = configJson["exchange"].get<std::string>();
        std::transform(_exchange.begin(), _exchange.end(), _exchange.begin(), ::tolower);

        if (_exchange != "nsdq" && _exchange != "cme" && _exchange != "nyse") {
            throw std::runtime_error(
                "Unsupported exchange: '" + _exchange +
                "'. Valid exchanges are: 'nsdq', 'cme', 'nyse'."
            );
        }
    } else {
        throw std::runtime_error("Missing 'exchange' field in config file.");
    }

    // Parse symbols
    if (configJson.contains("symbols")) {
        for (const auto& symbol : configJson["symbols"]) {
            SymbolData symbolData;
            symbolData.symbolName = symbol["symbol"].get<std::string>();
            symbolData.weight = symbol["percent"].get<double>(); // Read percent directly (1-100 range)
            symbolData.minPrice = symbol["price_range"]["min_price"].get<double>();
            symbolData.maxPrice = symbol["price_range"]["max_price"].get<double>();
            
            // Parse per-symbol spread_percentage (optional, falls back to global default)
            if (symbol.contains("spread_percentage")) {
                symbolData.spreadPercent = symbol["spread_percentage"].get<double>();
            } else {
                symbolData.spreadPercent = _defaultSpreadPercent;
            }
            
            _symbols.push_back(symbolData);
        }
    } else {
        throw std::runtime_error("Missing 'symbols' field in config file.");
    }

    return true;
}

std::unique_ptr<serializers::IMarketDataSerializer> ConfigProvider::createSerializer() const {
    if (_exchange == "nsdq") {
        // Create serializer for NASDAQ with flush interval
        return std::make_unique<serializers::NsdqMarketDataSerializer>(_outputFilePath, _flushInterval);
    } else if (_exchange == "cme") {
        // Create serializer for CME
        return std::make_unique<serializers::CmeMarketDataSerializer>(_outputFilePath);
    } else if (_exchange == "nyse") {
        // Create serializer for NYSE
        return std::make_unique<serializers::NyseMarketDataSerializer>(_outputFilePath);
    } else {
        throw std::runtime_error(
            "Unsupported exchange: '" + _exchange +
            "'. Valid exchanges are: 'nsdq', 'cme', 'nyse'. Ensure the exchange is correctly specified in the configuration file."
        );
    }
}

std::unique_ptr<serializers::IMarketDataSerializer> ConfigProvider::getSerializer() const {
    return createSerializer();
}

std::vector<SymbolData> ConfigProvider::getSymbolsForGeneration() const {
    return _symbols;
}

size_t ConfigProvider::getMessageCount() const {
    return _messageCount;
}

double ConfigProvider::getTradeProbability() const {
    return _tradeProbability;
}

size_t ConfigProvider::getFlushInterval() const {
    return _flushInterval;
}

} // namespace beacon::market_data_generator::config