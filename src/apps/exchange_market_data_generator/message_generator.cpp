/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Implements market data generation with configurable symbol
 *               distribution, realistic bid-ask spread maintenance, and
 *               proper market microstructure simulation.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include "message_generator.h"
#include "config_provider.h"
#include "serializers/market_data_serializer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>

namespace beaconserialize = beacon::market_data_generator::serializers;
namespace beaconconfig = beacon::market_data_generator::config;

namespace beacon::market_data_generator {

  MessageGenerator::MessageGenerator(const beaconconfig::ConfigProvider& configProvider)
  {
    // Get the serializer from the config provider
    _serializer = configProvider.getSerializer();
    
    // Use the new methods from ConfigProvider
    auto symbolsForGeneration = configProvider.getSymbolsForGeneration();
    _messageCount = configProvider.getMessageCount();
    _tradeProbability = configProvider.getTradeProbability();
    _flushInterval = configProvider.getFlushInterval();

    // Convert SymbolData to SymbolParameters
    for (const auto& symbolData : symbolsForGeneration) {
        SymbolParameters symbolParams;
        symbolParams.symbol = symbolData.symbolName;
        symbolParams.percent = symbolData.weight; // weight field contains the percent (1-100)
        symbolParams.spread_percent = symbolData.spreadPercent;
        symbolParams.price_range.min_price = symbolData.minPrice;
        symbolParams.price_range.max_price = symbolData.maxPrice;
        symbolParams.price_range.weight = 1.0; // Default weight for price generation

        // Similarly, handle quantity_range
        symbolParams.quantity_range.min_quantity = 1;
        symbolParams.quantity_range.max_quantity = 100;
        symbolParams.quantity_range.weight = 1.0; // Default weight

        _symbols.push_back(symbolParams);
    }
  }

  void MessageGenerator::generateMessages(const std::string& outputPath, size_t numMessages, const std::string& configPath) {
    // Start timing
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Calculate the number of messages for each symbol based on percentages
    std::vector<size_t> messagesPerSymbol;
    size_t totalMessagesAssigned = 0;
    
    for (const auto& symbol : _symbols) {
      size_t messagesForSymbol = static_cast<size_t>(std::round(numMessages * (symbol.percent / 100.0)));
      messagesPerSymbol.push_back(messagesForSymbol);
      totalMessagesAssigned += messagesForSymbol;
    }

    // Adjust for rounding errors to ensure the total number of messages matches numMessages
    // CRITICAL: Must check which direction to adjust to avoid unsigned integer underflow
    // Example: If rounding gives us 30,000 messages but we only want 10,000, we need to
    // subtract 20,000. With unsigned integers, 10,000 - 30,000 would wrap to ~18 quintillion!
    if (totalMessagesAssigned != numMessages) {
      if (totalMessagesAssigned < numMessages) {
        // Need to add messages
        size_t adjustment = numMessages - totalMessagesAssigned;
        messagesPerSymbol[0] += adjustment;
      } else {
        // Need to remove messages
        size_t adjustment = totalMessagesAssigned - numMessages;
        messagesPerSymbol[0] -= adjustment;
      }
    }

    // Print header and generation info
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════════════════════════════\n";
    std::cout << "                        EXCHANGE MARKET DATA GENERATOR - STARTING                                      \n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════════════════════════════\n\n";
    
    std::cout << "Configuration:\n";
    std::cout << "  Target Messages:    " << numMessages << "\n";
    std::cout << "  Symbols:            ";
    for (size_t i = 0; i < _symbols.size(); ++i) {
      std::cout << _symbols[i].symbol << " (" << _symbols[i].percent << "%)";
      if (i < _symbols.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "\n\n";
    
    if (!configPath.empty()) {
      // Extract just the filename from the path
      size_t lastSlash = configPath.find_last_of("/\\");
      std::string filename = (lastSlash != std::string::npos) ? configPath.substr(lastSlash + 1) : configPath;
      std::cout << "Generating messages (using " << filename << ")...\n";
    } else {
      std::cout << "Generating messages...\n";
    }

    // Use the Mersenne Twister algorithm with a random seed.
    std::random_device seedGenerator;
    auto randomSeed = seedGenerator();
    std::mt19937 gen(randomSeed);

    // Get current time once - don't make system calls for every message
    std::time_t currentTime = std::time(nullptr);    // Market state: track the most recent bid and ask prices and their sequence numbers for each symbol
    struct MarketState {
        double bidPrice = 0.0;
        double askPrice = 0.0;
        size_t bidSeqNum = 0;
        size_t askSeqNum = 0;
    };
    std::unordered_map<std::string, MarketState> marketState; // {symbol -> MarketState}
    size_t globalSequenceNumber = 1; // Start sequence numbers at 1
      
      // Generate messages for each symbol
      for (size_t i = 0; i < _symbols.size(); i++) {
          const SymbolParameters& symbolParams = _symbols[i];
          size_t messagesForSymbol = messagesPerSymbol[i];

          const auto& priceRange = symbolParams.price_range;
          const auto& quantityRange = symbolParams.quantity_range;

          // Define a uniform distribution for quantities
          std::uniform_int_distribution<> quantityDist(quantityRange.min_quantity, quantityRange.max_quantity);

          // Calculate spread based on symbol's configured spread percentage
          // Example: For a $100-$200 range with 0.5% spread, spread = $0.50
          // This creates realistic bid-ask spreads proportional to the price level
          double spread = (priceRange.max_price - priceRange.min_price) * (symbolParams.spread_percent / 100.0);

          for (size_t j = 0; j < messagesForSymbol; j++) {
              // Randomly decide if this is a trade using configured probability
              bool isTrade = (std::generate_canonical<double, 10>(gen) < _tradeProbability);
              
              double bidPrice, askPrice;
              size_t sourceSeqNum = 0;
              double tradePrice = 0.0;
              
              if (isTrade) {
                // For trades, use the current market state
                auto it = marketState.find(symbolParams.symbol);
                if (it != marketState.end()) {
                    const auto& state = it->second;
                    bidPrice = state.bidPrice;
                    askPrice = state.askPrice;
                    tradePrice = (bidPrice + askPrice) / 2.0; // Trade at midpoint
                    sourceSeqNum = (j % 2 == 0) ? state.bidSeqNum : state.askSeqNum;
                }
                else {
                    // No market state yet, skip this trade
                    continue;
                }
              } else {
                // For orders, update market prices periodically to create realistic price movement
                // This ensures bid and ask move together (not independently on each message)
                static constexpr size_t PRICE_UPDATE_INTERVAL = 10; // Update market every 10 messages
                bool shouldUpdatePrices = (j % PRICE_UPDATE_INTERVAL == 0) || (marketState.find(symbolParams.symbol) == marketState.end());
                
                if (shouldUpdatePrices) {
                    // Generate new prices with proper market microstructure
                    // Use weighted random to allow skewing price distribution within range
                    double randomFactor = std::generate_canonical<double, 10>(gen);
                    double weightedFactor = std::pow(randomFactor, priceRange.weight);
                    double basePrice = priceRange.min_price + weightedFactor * (priceRange.max_price - priceRange.min_price);

                    // Generate bid and ask with proper spread
                    // Bid is below base, ask is above base - maintaining proper market structure
                    bidPrice = std::clamp(basePrice - spread / 2.0, priceRange.min_price, priceRange.max_price);
                    askPrice = std::clamp(basePrice + spread / 2.0, priceRange.min_price, priceRange.max_price);
                    
                    // Safety check: ensure bid < ask after clamping
                    // If clamping causes inversion, push ask up by spread amount
                    if (bidPrice >= askPrice) {
                        askPrice = bidPrice + spread;
                    }

                    // Update the market state with BOTH bid and ask together
                    marketState[symbolParams.symbol].bidPrice = bidPrice;
                    marketState[symbolParams.symbol].askPrice = askPrice;
                    marketState[symbolParams.symbol].bidSeqNum = globalSequenceNumber;
                    marketState[symbolParams.symbol].askSeqNum = globalSequenceNumber;
                } else {
                    // Use existing market prices (bid and ask stay consistent)
                    auto& state = marketState[symbolParams.symbol];
                    bidPrice = state.bidPrice;
                    askPrice = state.askPrice;
                }
              }

              // Generate a quantity using the uniform distribution
              size_t quantity = quantityDist(gen);

              // Create the message
              Message message(
                  symbolParams.symbol,
                  isTrade ? MessageType::Last : (j % 2 == 0 ? MessageType::Bid : MessageType::Ask),
                  isTrade ? "Trade" : (j % 2 == 0 ? "Buy" : "Sell"),
                  quantity,
                  isTrade ? tradePrice : (j % 2 == 0 ? bidPrice : askPrice),
                  isTrade ? sourceSeqNum : globalSequenceNumber, // Use source sequence number for trades
                  currentTime // Use the cached timestamp
              );

              _serializer->serializeMessage(message);
              
              // Update statistics
              if (isTrade) {
                  _statsManager.updateTradeStats(symbolParams.symbol, quantity, tradePrice);
              } else {
                  bool isBuy = (j % 2 == 0);
                  double price = isBuy ? bidPrice : askPrice;
                  _statsManager.updateStats(symbolParams.symbol, isBuy, quantity, price);
              }

              // Increment the global sequence number
              globalSequenceNumber++;
          }
      }

      // End timing
      auto endTime = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed = endTime - startTime;

      std::cout << "\n>>> File generation complete: " << outputPath << " <<<\n\n";
      _statsManager.printStats(numMessages, elapsed.count());
  }

  size_t MessageGenerator::getMessageCount() const {
      return _messageCount; // Use the correct variable name
  }
} // namespace beacon::market_data_generator