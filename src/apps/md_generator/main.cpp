// Project:   Beacon
// File:      main.cpp
// Purpose:   Generates Nasdaq TotalView ITCH-5.0 messages for simulation purposes.
// Author:    Bryan Camp
// Component: Market Data Generator App
// Details:   This app generates binary ITCH messages for simulation/testing.
//            Options: debug(default)|release|all|clean|pipeline
//            Each app also has its own out-of-source build script.

#include <filesystem>
#include <iostream>
#include <string>
//#include "nyse/market_data/pillar_market_data/current/messages_active.h"
//#include "cme/market_data/mdp_mbo/current/messages_active.h"
#include "message_generator.h"

namespace fs = std::filesystem;

// Print usage information and errors.
// @param errors Vector of error strings.
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

// Main entry point for md_generator.
// Parses command-line arguments, validates input, and generates ITCH messages.
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