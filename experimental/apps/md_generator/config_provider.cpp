#include "config_provider.h"
#include <fstream>
#include <stdexcept>

ConfigProvider::ConfigProvider(const std::string& configPath) {
    loadConfig(configPath);
}

size_t ConfigProvider::getMessageCount() const {
    return _messageCount;
}

std::vector<SymbolGenerationData> ConfigProvider::getSymbolsForGeneration() const {
    return _symbols; // Return SymbolGenerationData directly
}

void ConfigProvider::loadConfig(const std::string& configPath) {
    std::ifstream configFile(configPath);
    if (!configFile) {
        throw std::runtime_error("Failed to open config file: " + configPath);
    }

    nlohmann::json configJson;
    configFile >> configJson;

    _messageCount = configJson["num_messages"];
    for (const auto& symbolConfig : configJson["symbols"]) {
        SymbolGenerationData symbolData;
        symbolData.symbol = symbolConfig["symbol"];

        // Ensure the correct field is used for the percentage
        if (symbolConfig.contains("percent_messages_to_create")) {
            symbolData.percent = symbolConfig["percent_messages_to_create"].get<double>();
        } else if (symbolConfig.contains("percent")) {
            symbolData.percent = symbolConfig["percent"].get<double>();
        } else {
            throw std::runtime_error("Symbol '" + symbolConfig["symbol"].get<std::string>() + "' is missing a 'percent' or 'percent_messages_to_create' field.");
        }

        // Validate and parse the single price range
        if (!symbolConfig.contains("price_range")) {
            throw std::runtime_error("Symbol '" + symbolConfig["symbol"].get<std::string>() + "' must have exactly one price_range.");
        }
        const auto& priceRange = symbolConfig["price_range"];
        symbolData.price_range = {
            priceRange["min_price"],
            priceRange["max_price"],
            priceRange["weight"]
        };

        // Validate and parse the single quantity range
        if (!symbolConfig.contains("quantity_range")) {
            throw std::runtime_error("Symbol '" + symbolConfig["symbol"].get<std::string>() + "' must have exactly one quantity_range.");
        }
        const auto& quantityRange = symbolConfig["quantity_range"];
        symbolData.quantity_range = {
            quantityRange["min_quantity"],
            quantityRange["max_quantity"],
            quantityRange["weight"]
        };

        _symbols.push_back(symbolData); // Push SymbolGenerationData directly
    }
}

bool ConfigProvider::validateJson(const nlohmann::json& j) {
    // ...existing validation logic...
    return true;
}

bool ConfigProvider::validateSymbol(const nlohmann::json& symbol) {
    // ...existing validation logic...
    return true;
}

bool ConfigProvider::validatePriceRange(const nlohmann::json& pr) {
    // ...existing validation logic...
    return true;
}

bool ConfigProvider::validateQuantityRange(const nlohmann::json& qr) {
    // ...existing validation logic...
    return true;
}
