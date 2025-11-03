#include "message_config_validator.h"

bool ConfigValidator::validate(const nlohmann::json& j, std::vector<std::string>& errors) {
    // (1) Must contain num_messages
    if (!j.contains("num_messages") || !j["num_messages"].is_number_unsigned()) {
        errors.push_back("Config must contain a 'num_messages' field (unsigned integer).");
    }

    // (2) Must contain symbols array
    if (!j.contains("symbols") || !j["symbols"].is_array()) {
        errors.push_back("Config must contain a 'symbols' array.");
        return false;
    }

    bool anyPercent = false, allPercent = true;
    double percent_sum = 0.0;
    size_t symbolCount = j["symbols"].size();

    for (const auto& symbol : j["symbols"]) {
        std::string err;
        if (!validateSymbol(symbol, err)) {
            errors.push_back(err);
        }
        bool hasPercent = symbol.contains("percent");
        anyPercent = anyPercent || hasPercent;
        allPercent = allPercent && hasPercent;
        if (hasPercent) {
            if (!symbol["percent"].is_number()) {
                errors.push_back("Symbol '" + symbol["symbol"].get<std::string>() + "' has non-numeric 'percent' field (no $, ^, etc).");
            }
            // Extra check: ensure percent is not a string
            if (symbol["percent"].is_string()) {
                errors.push_back("Symbol '" + symbol["symbol"].get<std::string>() + "' 'percent' must not be a string (no $, ^, etc).");
            }
        }
    }

    // Enforce all-or-nothing for percent
    if (anyPercent && !allPercent) {
        errors.push_back(
            "Config error: If any symbol specifies 'percent', all symbols must specify 'percent'.\n"
            "Hint: Add a 'percent' field to every symbol, or remove all 'percent' fields."
        );
    }
    if (allPercent && std::abs(percent_sum - 100.0) > 1e-6) {
        errors.push_back(
            "Config error: The sum of all symbol 'percent' fields must be exactly 100.\n"
            "Current sum: " + std::to_string(percent_sum) +
            ". Hint: Adjust your percentages so they add up to 100."
        );
    }

    return errors.empty();
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
        error = "Price range 'min_price' and 'max_price' must be numeric values (no $, ^, etc).";
        return false;
    }
    // Extra check: ensure value is not a string containing weird characters
    if (pr["min_price"].is_string() || pr["max_price"].is_string()) {
        error = "Price range values must not be strings (no $, ^, etc).";
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
        error = "Quantity range 'min_quantity' and 'max_quantity' must be integer values (no $, ^, etc).";
        return false;
    }
    // Extra check: ensure value is not a string containing weird characters
    if (qr["min_quantity"].is_string() || qr["max_quantity"].is_string()) {
        error = "Quantity range values must not be strings (no $, ^, etc).";
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

// Optionally, implement parseSymbols() to construct SymbolParameters from valid config
