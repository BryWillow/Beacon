#pragma once

#include <string>
#include <vector>
#include <json/json.hpp>
#include "symbol_parameters.h" // Include the consolidated header for SymbolParameters and SymbolGenerationData

class ConfigProvider {
public:
    explicit ConfigProvider(const std::string& configPath);

    // Delete unwanted constructors.
    ConfigProvider() = delete;
    ConfigProvider(const ConfigProvider&) = delete;
    ConfigProvider& operator=(const ConfigProvider&) = delete;
    ConfigProvider(ConfigProvider&&) = delete;
    ConfigProvider& operator=(ConfigProvider&&) = delete;

    size_t getMessageCount() const;
    std::vector<SymbolGenerationData> getSymbolsForGeneration() const;

private:
    size_t _messageCount;
    std::vector<SymbolGenerationData> _symbols;

    void loadConfig(const std::string& configPath);
    bool validateJson(const nlohmann::json& j);
    bool validateSymbol(const nlohmann::json& symbol);
    bool validatePriceRange(const nlohmann::json& pr);
    bool validateQuantityRange(const nlohmann::json& qr);
};
