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
#include <regex>
#include "beacon_exchanges/nyse/market_data/pillar_market_data/v1.9/market_data_message_types.h"
#include "beacon_exchanges/cme/market_data/mdp_mbo/v4.0/market_data_message_types.h"
#include "message_generator.h"

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
  std::cerr << "  md_generator --config <config.json> [-n <numMessages>] [-o <outputFile>]\n";
  std::cerr << "\n";
  std::cerr << "Config file requirements:\n";
  std::cerr << "  - Must contain 'num_messages' (unsigned integer).\n";
  std::cerr << "  - Must contain 'symbols' array.\n";
  std::cerr << "  - Each symbol must specify price and quantity ranges.\n";
  std::cerr << "  - Optionally, each symbol can specify a 'percent' field.\n";
  std::cerr << "    If any symbol specifies 'percent', all must, and the sum must be exactly 100.\n";
  std::cerr << "    Percentages are specified as numbers between 0 and 100 (e.g., 75 for 75%).\n";
  std::cerr << "\n";
  std::cerr << "Options:\n";
  std::cerr << "  --config <file>    Path to config file (.json)\n";
  std::cerr << "  -n <numMessages>   Number of messages to generate (overrides config)\n";
  std::cerr << "  -o <outputFile>    Output file name (.dat extension recommended)\n";
  std::cerr << "  -h, --help         Print this help message and exit\n";
  std::cerr << "  -q                 Semi-quiet mode: print summary every 500 messages\n";
  std::cerr << "\n";
  std::cerr << "All symbol parameters (price/quantity ranges, percent, etc.) are specified in the config file only.\n";
  std::cerr << "\n";
  std::cerr << "Example config snippet:\n";
  std::cerr << R"({
  "num_messages": 10000,
  "symbols": [
    { "symbol": "MSFT", "percent": 75, "price_ranges": [{ "min_price": 100.0, "max_price": 200.0 }], "quantity_ranges": [{ "min_quantity": 1, "max_quantity": 100 }] },
    { "symbol": "AAPL", "percent": 25, "price_ranges": [{ "min_price": 150.0, "max_price": 250.0 }], "quantity_ranges": [{ "min_quantity": 10, "max_quantity": 200 }] }
  ]
})" << "\n";
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

    // Add config base name to output file if not already present
    std::string configBase;
    {
      // Extract base filename without extension
      std::filesystem::path cfgPath(configPath);
      configBase = cfgPath.stem().string();
      // If outputFile is empty, use default
      if (outputFile.empty()) {
        outputFile = configBase + ".dat";
      } else {
        // Insert configBase before extension if not already present
        std::filesystem::path outPath(outputFile);
        std::string outStem = outPath.stem().string();
        std::string outExt = outPath.extension().string();
        // Remove ".json" from configBase if present (shouldn't be, but just in case)
        configBase = std::regex_replace(configBase, std::regex("\\.json$"), "");
        // If configBase not in outStem, prepend it
        if (outStem.find(configBase) == std::string::npos) {
          outputFile = outPath.parent_path().string() + "/" + configBase + "_" + outStem + outExt;
        }
      }
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