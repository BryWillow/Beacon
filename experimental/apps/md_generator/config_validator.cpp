#include "config_validator.h"
#include <fstream>

bool ConfigValidator::validateConfigFile(const std::string& configPath) {
    _errors.clear();

    std::ifstream configFile(configPath);
    if (!configFile) {
        _errors.push_back("Failed to open config file: " + configPath);
        return false;
    }

    nlohmann::json configJson;
    try {
        configFile >> configJson;
    } catch (const std::exception& e) {
        _errors.push_back("Invalid JSON format: " + std::string(e.what()));
        return false;
    }

    return validateJson(configJson);
}

const std::vector<std::string>& ConfigValidator::getErrors() const {
    return _errors;
}

bool ConfigValidator::validateJson(const nlohmann::json& j) {
    if (!j.contains("num_messages") || !j["num_messages"].is_number_unsigned()) {
        _errors.push_back("Config must contain a 'num_messages' field (unsigned integer).");
    }
    if (!j.contains("symbols") || !j["symbols"].is_array()) {
        _errors.push_back("Config must contain a 'symbols' array.");
        return false;
    }

    bool anyPercent = false, allPercent = true;
    double percentSum = 0.0;
    for (const auto& symbol : j["symbols"]) {
        std::string err;
        if (!validateSymbol(symbol, err)) {
            _errors.push_back(err);
        }
        bool hasPercent = symbol.contains("percent");
        anyPercent = anyPercent || hasPercent;
        allPercent = allPercent && hasPercent;
        if (hasPercent) {
            if (!symbol["percent"].is_number()) {
                _errors.push_back("Symbol '" + symbol["symbol"].get<std::string>() + "' has non-numeric 'percent' field.");
            } else {
                percentSum += symbol["percent"].get<double>();
            }
        }
    }
    if (anyPercent && !allPercent) {
        _errors.push_back("If any symbol specifies 'percent', all symbols must specify 'percent'.");
    }
    if (allPercent && std::abs(percentSum - 100.0) > 1e-6) {
        _errors.push_back("Sum of all symbol 'percent' fields must be exactly 100. Current sum: " + std::to_string(percentSum));
    }

    percentSum = 0.0; // Reset for new calculation
    for (const auto& symbol : j["symbols"]) {
        if (symbol.contains("percent_messages_to_create")) {
            if (!symbol["percent_messages_to_create"].is_number()) {
                _errors.push_back("Symbol '" + symbol["symbol"].get<std::string>() + "' has non-numeric 'percent_messages_to_create' field.");
            } else {
                percentSum += symbol["percent_messages_to_create"].get<double>();
            }
        }
    }

    if (std::abs(percentSum - 100.0) > 1e-6) {
        _errors.push_back("Sum of all symbol 'percent_messages_to_create' fields must be exactly 100. Current sum: " + std::to_string(percentSum));
    }

    return _errors.empty();
}

bool ConfigValidator::validateSymbol(const nlohmann::json& symbol, std::string& error) {
    if (!symbol.contains("symbol") || !symbol["symbol"].is_string()) {
        error = "Each symbol must have a 'symbol' string field.";
        return false;
    }
    if (!symbol.contains("price_ranges") || !symbol["price_ranges"].is_array()) {
        error = "Symbol " + symbol["symbol"].get<std::string>() + " missing 'price_ranges' array.";
        return false;
    }
    if (!symbol.contains("quantity_ranges") || !symbol["quantity_ranges"].is_array()) {
        error = "Symbol " + symbol["symbol"].get<std::string>() + " missing 'quantity_ranges' array.";
        return false;
    }
    for (const auto& pr : symbol["price_ranges"]) {
        std::string pr_err;
        if (!validatePriceRange(pr, pr_err)) {
            error = "Symbol " + symbol["symbol"].get<std::string>() + ": " + pr_err;
            return false;
        }
    }
    for (const auto& qr : symbol["quantity_ranges"]) {
        std::string qr_err;
        if (!validateQuantityRange(qr, qr_err)) {
            error = "Symbol " + symbol["symbol"].get<std::string>() + ": " + qr_err;
            return false;
        }
    }
    return true;
}

bool ConfigValidator::validatePriceRange(const nlohmann::json& pr, std::string& error) {
    if (!pr.contains("min_price") || !pr.contains("max_price")) {
        error = "Price range missing 'min_price' or 'max_price'.";
        return false;
    }
    if (!pr["min_price"].is_number() || !pr["max_price"].is_number()) {
        error = "Price range 'min_price' and 'max_price' must be numeric values.";
        return false;
    }
    double minp = pr["min_price"].get<double>();
    double maxp = pr["max_price"].get<double>();
    if (minp < 0 || maxp < minp) {
        error = "Invalid price range: min_price must be >= 0 and max_price >= min_price.";
        return false;
    }
    return true;
}

bool ConfigValidator::validateQuantityRange(const nlohmann::json& qr, std::string& error) {
    if (!qr.contains("min_quantity") || !qr.contains("max_quantity")) {
        error = "Quantity range missing 'min_quantity' or 'max_quantity'.";
        return false;
    }
    if (!qr["min_quantity"].is_number_integer() || !qr["max_quantity"].is_number_integer()) {
        error = "Quantity range 'min_quantity' and 'max_quantity' must be integer values.";
        return false;
    }
    int minq = qr["min_quantity"].get<int>();
    int maxq = qr["max_quantity"].get<int>();
    if (minq < 1 || maxq < minq) {
        error = "Invalid quantity range: min_quantity must be >= 1 and max_quantity >= min_quantity.";
        return false;
    }
    return true;
}
