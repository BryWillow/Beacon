/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Main entry point for the exchange market data generator.
 *               Parses command-line arguments, loads configuration, and
 *               orchestrates message generation with proper error handling.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include "config_provider.h"
#include "message_generator.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>

namespace beaconconfig = beacon::market_data_generator::config;

const std::string JSON_EXTENSION = ".json"; // Changed from constexpr std::string_view to std::string
const std::string ILLEGAL_CHARS = "<>:\"|?*"; // Changed from constexpr std::string_view to std::string

std::string toLower(const std::string& inString) {
    std::string lowerString = inString;
    std::transform(lowerString.begin(), lowerString.end(), lowerString.begin(), ::tolower);
    return lowerString;
}

bool isPathWellFormed(const std::string& path) {
    try {
        // Check if the path can be constructed as a std::filesystem::path
        std::filesystem::path fsPath(path);
        if (fsPath.empty()) {
            return false;
        }

        // Check for illegal characters in the path
        for (char ch : ILLEGAL_CHARS) {
            if (path.find(ch) != std::string::npos) {
                std::cerr << "Path contains invalid character: " << ch << "\n";
                return false;
            }
        }

        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "The specified path is invalid: " << e.what() << "\n";
        return false;
    }
}

// Prints usage information for the program
void printUsage() {
    std::cout << "Usage: ./exchange_market_data_generator <config_file> <output_file>\n"
              << "  <config_file>: Path to the .json configuration file.\n"
              << "  <output_file>: Path to the output file for serialized data.\n\n"
              << "Examples:\n"
              << "  ./exchange_market_data_generator config.json output.itch\n"
              << "  ./exchange_market_data_generator config.json output.cme\n";
}

int main(int argc, char* argv[]) {
    try {
        // Validate the number of arguments
        if (argc != 3) {
            std::cerr << "[exchange_market_data_generator] Error: Invalid number of arguments.\n";
            printUsage();
            return 1;
        }

        // Parse command-line arguments
        std::string configFilePath = argv[1];
        std::string outputFilePath = argv[2];

        // Create the ConfigProvider and load the configuration
        beacon::market_data_generator::config::ConfigProvider configProvider("", outputFilePath);
        if (!configProvider.loadConfig(configFilePath)) {
            std::cerr << "[exchange_market_data_generator] Error: Failed to load configuration from file: " << configFilePath << "\n";
            return 1;
        }

        // Get the serializer and proceed with execution
        auto serializer = configProvider.getSerializer();

        // Pass the serializer to the message generator
        beacon::market_data_generator::MessageGenerator generator(configProvider);
        size_t numMessages = generator.getMessageCount();
        generator.generateMessages(outputFilePath, numMessages, configFilePath);
    }
    catch (const std::exception& e) {
        std::cerr << "[exchange_market_data_generator] Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}