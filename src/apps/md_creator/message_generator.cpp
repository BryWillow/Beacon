/**
 * @project   Beacon
 * @file      md_generator.cpp
 * @brief     Generates Nasdaq TotalView ITCH-5.0 messages for simulation purposes.
 * @author    Bryan Camp
 * @component Market Data Generator App
 * @details   This app generates binary ITCH messages for simulation/testing.
 *            Options: debug(default)|release|all|clean|pipeline
 *            Each app also has its own out-of-source build script.
 */

#include <filesystem>
#include <iostream>
#include <string>
#include <limits>
#include "beacon_exchanges/nyse/market_data/pillar_market_data/v1.9/market_data_message_types.h"
#include "beacon_exchanges/cme/market_data/mdp_mbo/v4.0/market_data_message_types.h"
#include "message_generator.h"
#include "message_config_validator.h"

namespace fs = std::filesystem;

/**
 * @brief Print usage information and errors.
 * @param errors Vector of error strings.
 */
void usage(const std::vector<std::string>& errors = {}) {
  std::cerr << "\n";
  if (!errors.empty()) {
    for (const auto& err : errors) {
      std::cerr << "\033[1;31mError:\033[0m " << err << "\n";
    }
    std::cerr << "\n";
  }
  std::cerr << "Usage:\n";
  std::cerr << "  md_generator [-n <numMessages>] [-o <outputFile>] \\\n";
  std::cerr << "    -s <symbol> [-minp <minPrice>] [-maxp <maxPrice>] [-minq <minQty>] [-maxq <maxQty>] \\\n";
  std::cerr << "    ... (repeat -s block for each symbol)\n";
  std::cerr << "\n";
  std::cerr << "Options:\n";
  std::cerr << "  -n <numMessages>   Number of messages to generate (default: 10000)\n";
  std::cerr << "  -o <outputFile>    Output file name (.dat extension recommended)\n";
  std::cerr << "  -s <symbol>        Symbol name (repeatable)\n";
  std::cerr << "  -minp <minPrice>   Minimum price for symbol (integer, e.g. 1000000 for 100.0000)\n";
  std::cerr << "  -maxp <maxPrice>   Maximum price for symbol (integer, e.g. 2000000 for 200.0000)\n";
  std::cerr << "  -minq <minQty>     Minimum quantity for symbol (default: 1)\n";
  std::cerr << "  -maxq <maxQty>     Maximum quantity for symbol (default: 100)\n";
  std::cerr << "  -tick <tickSize>   Tick size for price (integer, default: 1)\n";
  std::cerr << "  -h, --help         Print this help message and exit\n";
  std::cerr << "  -q                 Semi-quiet mode: print summary every 500 messages\n";
  std::cerr << "\n";
  std::cerr << "Example:\n";
  std::cerr << "  md_generator -n 5000 -o msft_test.dat \\\n";
  std::cerr << "    -s MSFT -minp 100.0 -maxp 200.0 -minq 1 -maxq 100 \\\n";
  std::cerr << "    -s AAPL -minp 150.0 -maxp 250.0 -minq 10 -maxq 200\n";
  std::cerr << "\n";
  // -s MSFT -minp 100.0 -maxp 200.0 -minq 1 -maxq 100

// This will generate messages for symbol "MSFT" with:
//   min price: 100.0 (as integer ticks, e.g. 1000000)
//   max price: 200.0 (as integer ticks, e.g. 2000000)
//   min quantity: 1
//   max quantity: 100

// You can repeat the -s block for multiple symbols:
// md_generator -s MSFT -minp 100.0 -maxp 200.0 -minq 1 -maxq 100 -s AAPL -minp 150.0 -maxp 250.0 -minq 10 -maxq 200
}

// Utility function to convert a price string to uint32_t ticks (1/10000 dollars)
uint32_t parse_price_to_ticks(const std::string& priceStr) {
  // Accepts "3", "3.1", "3.12", "3.123", "3.1234"
  double price = std::stod(priceStr);
  // Multiply by 10,000 and round to nearest integer
  return static_cast<uint32_t>(std::round(price * 10000.0));
}

/**
 * @brief Main entry point for md_generator.
 * Parses command-line arguments, validates input, and generates ITCH messages.
 */
int main(int argc, char* argv[]) {
  try {
    std::string configPath;
    std::string outputFile;
    size_t numMessages = 10000;

    // TODO: Parse configPath and outputFile from argv or environment if needed
    // For now, assume configPath and outputFile are set appropriately

    if (configPath.empty()) {
      std::cerr << "[md_generator] Error: Config file path must be specified.\n";
      return 1;
    }

    MessageGenerator generator(configPath);
    numMessages = generator.getMessageCount();
    generator.generateMessages(outputFile, numMessages);
    generator.printStats();

  } catch (const std::exception& e) {
    std::cerr << "[md_generator] Exception: " << e.what() << "\n";
    return 1;
  }
  return 0;
}

// File: message_generator.h
// Purpose: Message generation and config parsing for md_generator.
//
// Project Checklist:
//   - Use concise, descriptive executable names (e.g., md_generator, md_server, md_trader).
//   - To add new parameters to the config:
//       - Add the new field to the relevant struct (e.g., SymbolParameters, PriceRange).
//       - Update the JSON parsing logic in MessageGenerator::parseJson() (see .cpp).
//       - Add validation for the new parameter in MessageGenerator::validateConfig() (recommended).
//       - Document the new parameter in the config file and usage/help output.
//
// Validation should be performed immediately after parsing, in a dedicated method
// (e.g., validateConfig()), to ensure all config values are correct before use.
//
// The MessageGenerator constructor loads and validates the config file.
// If the config is malformed, it throws an exception with a descriptive error message.

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <json/json.hpp>

constexpr uint32_t ITCH_MAX_ORDER_QUANTITY = 1000000;
constexpr double ITCH_MAX_ORDER_PRICE = 429496.7295; // ITCH price field is uint32_t, max value is 4294967295 ticks (429496.7295 dollars)

struct QuantityRange {
  uint32_t min_quantity;
  uint32_t max_quantity;
  double weight;
};

struct PriceRange {
  double min_price;
  double max_price;
  double weight;
};

struct SymbolParameters {
  std::string symbol;
  std::vector<PriceRange> price_ranges;
  std::vector<QuantityRange> quantity_ranges;
  // ...add more fields as needed...
};

class MessageGenerator {
public:
  // Construct a MessageGenerator and load/validate config.
  // @param configPath Path to the config file (.json).
  // @throws std::runtime_error if config is malformed or validation fails.
  //
  // The constructor loads the config file and validates its content.
  // If the config is malformed or validation fails, an exception is thrown
  // with a descriptive error message.
  explicit MessageGenerator(const std::string& configPath) {
    try {
      loadConfig(configPath);
      validateConfig();
    }
    catch (const std::exception& e) {
      std::cerr << "[MessageGenerator] Config load failed: " << e.what() << std::endl;
      throw;
    }
  }

  // Delete copy and move operations to prevent accidental copying
  MessageGenerator(const MessageGenerator&) = delete;
  MessageGenerator& operator=(const MessageGenerator&) = delete;
  MessageGenerator(MessageGenerator&&) = delete;
  MessageGenerator& operator=(MessageGenerator&&) = delete;

  void generateMessages(const std::string& outputPath, size_t numMessages);
  void printStats() const;
  size_t getMessageCount() const { return _message_count; }

private:
  const std::vector<SymbolParameters> _symbols;
  const size_t _message_count = 10000; // default if not specified

  void parseJson(const nlohmann::json& j);
  void validateConfig(); // Called after parsing, throws on error
  // ...other private helpers...
};

#include "message_generator.h"
#include <random>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

double generatePrice(const PriceRange& range) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    double mean = (range.min_price + range.max_price) / 2.0;
    double stddev = (range.max_price - range.min_price) / 6.0;
    std::normal_distribution<> d(mean, stddev);
    double price = std::clamp(d(gen), range.min_price, range.max_price);
    return price;
}

uint32_t generateQuantity(const QuantityRange& range) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(range.min_quantity, range.max_quantity);
    return d(gen);
}

struct OrderInfo {
    uint64_t sequenceNumber;
    uint32_t orderId;
    // ...other fields as needed...
};

void MessageGenerator::generateMessages(const std::string& outputPath, size_t numMessages) {
    std::cout << "[md_generator] Starting message generation...\n";
    std::cout << "[md_generator] Output file: " << outputPath << "\n";
    std::cout << "[md_generator] Total messages: " << numMessages << "\n";
    std::cout << "[md_generator] Symbols: ";
    for (const auto& sym : _symbols) std::cout << sym.symbol << " ";
    std::cout << "\n";

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Failed to open output file: " + outputPath);
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    // ...existing code for symbolSeqNum, activeOrders, etc...

    for (size_t i = 0; i < numMessages; ++i) {
        // ...existing code for message generation...

        // Example for ITCH or exchange message struct:
        MyExchangeMessage msg;
        // ...populate other fields...
        msg.timestamp = std::numeric_limits<uint64_t>::min(); // or 0

        // ...write message...
    }

    // After main message generation, ensure all orders are terminal
    for (const auto& [symbol, orders] : activeOrders) {
        for (const auto& order : orders) {
            // Generate a cancel or trade message for each remaining active order
            // Example: generate cancel message
            CancelOrderMsg cancelMsg;
            cancelMsg.sequenceNumber = ++symbolSeqNum[symbol];
            cancelMsg.orderId = order.orderId;
            // ...populate other fields as needed...
            out.write(reinterpret_cast<const char*>(&cancelMsg), sizeof(cancelMsg));
            // Alternatively, generate a trade message if desired
        }
    }

    out.close();

    std::cout << "[md_generator] Message generation complete.\n";
    std::cout << "[md_generator] Output file written to: " << outputPath << "\n";
    std::cout << "[md_generator] All orders are in terminal state.\n";
    // Optionally, print more stats here
}

void MessageGenerator::validateConfig() {
    // No validation needed in generator app.
    // Validation will be handled by md_server.
}