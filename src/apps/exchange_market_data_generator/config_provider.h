/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Provides configuration management for the market data generator,
 *               including parsing JSON config files and instantiating the
 *               appropriate exchange-specific serializer.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "serializers/market_data_serializer.h"
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace beacon::market_data_generator::config {

/// @brief Represents data for a symbol, including its distribution weight and price range.
struct SymbolData {
    std::string symbolName; // The name of the symbol (e.g., "AAPL").
    double weight;          // The distribution percentage (1-100, e.g., 60 for 60%).
    double minPrice;        // The minimum price for the symbol.
    double maxPrice;        // The maximum price for the symbol.
    double spreadPercent;   // The bid-ask spread as percentage (e.g., 0.5 for 0.5%).
};

class ConfigProvider {
  public:
    /// @brief Constructor.
    /// @param exchangeType The type of exchange to use (e.g., "CME").
    /// @param outputFilePath The file path for serialized output.
    ConfigProvider(const std::string& exchangeType, const std::string& outputFilePath);

    /// @brief Get the serializer based on the type.
    std::unique_ptr<serializers::IMarketDataSerializer> getSerializer() const;

    /// @brief Get the symbols for generation.
    /// @return A vector of SymbolData objects.
    std::vector<SymbolData> getSymbolsForGeneration() const;

    /// @brief Get the total message count.
    /// @return The number of messages to generate.
    size_t getMessageCount() const;

    /// @brief Get the trade probability.
    /// @return The probability (0.0-1.0) that a message is a trade.
    double getTradeProbability() const;

    /// @brief Get the flush interval.
    /// @return The number of messages between buffer flushes.
    size_t getFlushInterval() const;

    /// @brief Load symbols from a configuration file.
    /// @param configPath The path to the configuration file.
    void loadSymbolsFromConfig(const std::string& configPath);

    /// @brief Load configuration from a file.
    /// @param configPath The path to the configuration file.
    bool loadConfig(const std::string& configPath);

    /// @brief Create a serializer based on the exchange type.
    std::unique_ptr<serializers::IMarketDataSerializer> createSerializer() const;

  private:
    std::string _exchangeType; // The type of exchange (e.g., "CME").
    std::string _outputFilePath; // The file path for serialized output.

    std::string _exchange; // The exchange name (e.g., "nsdq").
    std::vector<SymbolData> _symbols; // Symbols for generation.
    size_t _messageCount = 10000; // Default message count.
    double _tradeProbability = 0.1; // Default 10% trades.
    size_t _flushInterval = 1000; // Default flush every 1000 messages.
    double _defaultSpreadPercent = 0.5; // Default 0.5% spread.
};

} // namespace beacon::market_data_generator::config