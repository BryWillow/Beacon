#pragma once

#include <string>
#include <vector>
#include <json/json.hpp>
#include "message_generator.h"

class ConfigValidator {
public:
    ConfigValidator() = default;
    bool validate(const nlohmann::json& j, std::vector<std::string>& errors);

    // Optionally, parse and return SymbolParameters if valid
    std::vector<SymbolParameters> parseSymbols(const nlohmann::json& j, std::vector<std::string>& errors);

private:
    bool validateSymbol(const nlohmann::json& symbol, std::string& error);
    bool validatePriceRange(const nlohmann::json& pr, std::string& error);
    bool validateQuantityRange(const nlohmann::json& qr, std::string& error);
    bool validateNumMessages(const nlohmann::json& j, std::string& error);
    bool validateSymbolPercentages(const nlohmann::json& j, std::string& error);
};
