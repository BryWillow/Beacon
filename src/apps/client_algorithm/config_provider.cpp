/*
 * =============================================================================
 * Project:      Beacon
 * Application:  client_algorithm
 * File:         config_provider.cpp
 * Purpose:      Implementation of configuration management for trading algorithm.
 *               Handles JSON parsing and validation of per-product settings.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "config_provider.h"

namespace beacon::client_algorithm {

bool ConfigProvider::loadFromFile(const std::string& filepath) {
    try {
        // Read JSON file
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "ERROR: Could not open config file: " << filepath << "\n";
            return false;
        }
        
        nlohmann::json j;
        file >> j;
        
        // Parse each section
        if (!parseMarketDataConfig(j)) {
            std::cerr << "ERROR: Failed to parse market_data config section\n";
            return false;
        }
        
        if (!parseExchangeConfig(j)) {
            std::cerr << "ERROR: Failed to parse exchange config section\n";
            return false;
        }
        
        if (!parseTradingConfig(j)) {
            std::cerr << "ERROR: Failed to parse trading config section\n";
            return false;
        }
        
        if (!parseRiskManagementConfig(j)) {
            std::cerr << "ERROR: Failed to parse risk_management config section\n";
            return false;
        }
        
        if (!parseProducts(j)) {
            std::cerr << "ERROR: Failed to parse products array\n";
            return false;
        }
        
        // Build index for fast lookups
        buildSymbolIndex();
        
        std::cout << "Configuration loaded successfully:\n";
        std::cout << "  - Market Data: " << market_data_config_.host << ":" 
                  << market_data_config_.port << "\n";
        std::cout << "  - Exchange: " << exchange_config_.host << ":" 
                  << exchange_config_.port << "\n";
    std::cout << "  Risk (PnL): warning=" << risk_config_.pnl_drawdown_warning 
              << ", alert=" << risk_config_.pnl_drawdown_alert 
              << ", hard_stop=" << risk_config_.pnl_drawdown_hard_stop << std::endl;
    std::cout << "  Risk (Rejects): warning=" << risk_config_.order_reject_warning 
              << ", alert=" << risk_config_.order_reject_alert 
              << ", hard_stop=" << risk_config_.order_reject_hard_stop << std::endl;
    std::cout << "  Risk (Burst): warning=" << risk_config_.order_messaging_burst_warning 
              << ", alert=" << risk_config_.order_messaging_burst_alert 
              << ", hard_stop=" << risk_config_.order_messaging_burst_hard_stop << " orders/sec" << std::endl;
        std::cout << "  - Products: " << products_.size() 
                  << " (" << getEnabledProductCount() << " enabled)\n";
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Exception while loading config: " << e.what() << "\n";
        return false;
    }
}

bool ConfigProvider::parseMarketDataConfig(const nlohmann::json& j) {
    if (!j.contains("market_data")) {
        std::cerr << "ERROR: Missing 'market_data' section\n";
        return false;
    }
    
    const auto& md = j["market_data"];
    
    if (!md.contains("host") || !md.contains("port")) {
        std::cerr << "ERROR: market_data section missing 'host' or 'port'\n";
        return false;
    }
    
    market_data_config_.host = md["host"].get<std::string>();
    market_data_config_.port = md["port"].get<uint16_t>();
    
    return true;
}

bool ConfigProvider::parseExchangeConfig(const nlohmann::json& j) {
    if (!j.contains("exchange")) {
        std::cerr << "ERROR: Missing 'exchange' section\n";
        return false;
    }
    
    const auto& ex = j["exchange"];
    
    if (!ex.contains("host") || !ex.contains("port")) {
        std::cerr << "ERROR: exchange section missing 'host' or 'port'\n";
        return false;
    }
    
    exchange_config_.host = ex["host"].get<std::string>();
    exchange_config_.port = ex["port"].get<uint16_t>();
    
    return true;
}

bool ConfigProvider::parseTradingConfig(const nlohmann::json& j) {
    if (!j.contains("trading")) {
        std::cerr << "WARNING: Missing 'trading' section, using defaults\n";
        return true;  // Optional section
    }
    
    const auto& trading = j["trading"];
    
    if (trading.contains("order_frequency")) {
        trading_config_.order_frequency = trading["order_frequency"].get<uint32_t>();
    }
    
    if (trading.contains("default_time_in_force")) {
        std::string tif = trading["default_time_in_force"].get<std::string>();
        if (!tif.empty()) {
            trading_config_.default_time_in_force = tif[0];
        }
    }
    
    if (trading.contains("default_capacity")) {
        std::string capacity = trading["default_capacity"].get<std::string>();
        if (!capacity.empty()) {
            trading_config_.default_capacity = capacity[0];
        }
    }
    
    return true;
}

bool ConfigProvider::parseRiskManagementConfig(const nlohmann::json& j) {
    if (!j.contains("risk_management")) {
        std::cerr << "WARNING: Missing 'risk_management' section, using defaults\n";
        return true;  // Optional section, use defaults
    }
    
    const auto& risk = j["risk_management"];
    
    if (risk.contains("pnl_drawdown_warning")) {
        risk_config_.pnl_drawdown_warning = risk["pnl_drawdown_warning"].get<double>();
    }
    
    if (risk.contains("pnl_drawdown_alert")) {
        risk_config_.pnl_drawdown_alert = risk["pnl_drawdown_alert"].get<double>();
    }
    
    if (risk.contains("pnl_drawdown_hard_stop")) {
        risk_config_.pnl_drawdown_hard_stop = risk["pnl_drawdown_hard_stop"].get<double>();
    }
    
    if (risk.contains("order_reject_warning")) {
        risk_config_.order_reject_warning = risk["order_reject_warning"].get<uint32_t>();
    }
    
    if (risk.contains("order_reject_alert")) {
        risk_config_.order_reject_alert = risk["order_reject_alert"].get<uint32_t>();
    }
    
    if (risk.contains("order_reject_hard_stop")) {
        risk_config_.order_reject_hard_stop = risk["order_reject_hard_stop"].get<uint32_t>();
    }
    
    // Validate PnL thresholds are in correct order
    if (!risk_config_.validatePnL()) {
        std::cerr << "WARNING: PnL drawdown thresholds not in correct order!\n";
        std::cerr << "  Expected: warning >= alert >= hard_stop (all negative)\n";
        std::cerr << "  Got: warning=" << risk_config_.pnl_drawdown_warning
                  << " alert=" << risk_config_.pnl_drawdown_alert
                  << " hard_stop=" << risk_config_.pnl_drawdown_hard_stop << "\n";
        std::cerr << "  Continuing with these values, but check your configuration!\n";
    }
    
    // Validate reject thresholds are in correct order
    if (!risk_config_.validateRejects()) {
        std::cerr << "WARNING: Order reject thresholds not in correct order!\n";
        std::cerr << "  Expected: warning <= alert <= hard_stop (all positive)\n";
        std::cerr << "  Got: warning=" << risk_config_.order_reject_warning
                  << " alert=" << risk_config_.order_reject_alert
                  << " hard_stop=" << risk_config_.order_reject_hard_stop << "\n";
        std::cerr << "  Continuing with these values, but check your configuration!\n";
    }
    
    return true;
}

bool ConfigProvider::parseProducts(const nlohmann::json& j) {
    if (!j.contains("products")) {
        std::cerr << "ERROR: Missing 'products' array\n";
        return false;
    }
    
    const auto& products = j["products"];
    
    if (!products.is_array()) {
        std::cerr << "ERROR: 'products' must be an array\n";
        return false;
    }
    
    if (products.empty()) {
        std::cerr << "WARNING: 'products' array is empty\n";
        return true;
    }
    
    for (const auto& product : products) {
        ProductConfig config;
        
        // Required fields
        if (!product.contains("symbol")) {
            std::cerr << "ERROR: Product missing 'symbol' field\n";
            return false;
        }
        config.symbol = product["symbol"].get<std::string>();
        
        if (!product.contains("starting_position")) {
            std::cerr << "ERROR: Product '" << config.symbol 
                      << "' missing 'starting_position' field\n";
            return false;
        }
        config.starting_position = product["starting_position"].get<int32_t>();
        
        if (!product.contains("algo_position_limit")) {
            std::cerr << "ERROR: Product '" << config.symbol 
                      << "' missing 'algo_position_limit' field\n";
            return false;
        }
        config.algo_position_limit = product["algo_position_limit"].get<uint32_t>();
        
        // firm_position_limit is optional (defaults to algo_position_limit if not specified)
        if (product.contains("firm_position_limit")) {
            config.firm_position_limit = product["firm_position_limit"].get<uint32_t>();
        } else {
            config.firm_position_limit = config.algo_position_limit;  // Default to algo limit
        }
        
        // Optional field
        if (product.contains("enabled")) {
            config.enabled = product["enabled"].get<bool>();
        } else {
            config.enabled = true;  // Default to enabled
        }
        
        // Initialize current position to starting position
        config.current_position = config.starting_position;
        
        // Validate
        if (config.algo_position_limit == 0) {
            std::cerr << "WARNING: Product '" << config.symbol 
                      << "' has algo_position_limit=0, will not be tradeable\n";
        }
        
        if (config.firm_position_limit == 0) {
            std::cerr << "WARNING: Product '" << config.symbol 
                      << "' has firm_position_limit=0, will not be tradeable\n";
        }
        
        // CRITICAL: algo_position_limit must be <= firm_position_limit
        if (config.algo_position_limit > config.firm_position_limit) {
            std::cerr << "ERROR: Product '" << config.symbol 
                      << "' algo_position_limit (" << config.algo_position_limit
                      << ") exceeds firm_position_limit (" << config.firm_position_limit << ")\n";
            std::cerr << "  algo_position_limit must be <= firm_position_limit\n";
            return false;
        }
        
        if (std::abs(config.starting_position) > static_cast<int32_t>(config.algo_position_limit)) {
            std::cerr << "WARNING: Product '" << config.symbol 
                      << "' starting_position (" << config.starting_position
                      << ") exceeds algo_position_limit (" << config.algo_position_limit << ")\n";
        }
        
        if (std::abs(config.starting_position) > static_cast<int32_t>(config.firm_position_limit)) {
            std::cerr << "WARNING: Product '" << config.symbol 
                      << "' starting_position (" << config.starting_position
                      << ") exceeds firm_position_limit (" << config.firm_position_limit << ")\n";
        }
        
        products_.push_back(config);
        
        std::cout << "  Product: " << config.symbol 
                  << " | Start Pos: " << config.starting_position
                  << " | Algo Limit: " << config.algo_position_limit
                  << " | Firm Limit: " << config.firm_position_limit
                  << " | Enabled: " << (config.enabled ? "Yes" : "No") << "\n";
    }
    
    return true;
}

void ConfigProvider::buildSymbolIndex() {
    symbol_index_map_.clear();
    for (size_t i = 0; i < products_.size(); ++i) {
        symbol_index_map_[products_[i].symbol] = i;
    }
}

const ProductConfig* ConfigProvider::getProductConfig(const std::string& symbol) const {
    auto it = symbol_index_map_.find(symbol);
    if (it != symbol_index_map_.end()) {
        return &products_[it->second];
    }
    return nullptr;
}

std::vector<ProductConfig> ConfigProvider::getEnabledProducts() const {
    std::vector<ProductConfig> enabled;
    for (const auto& product : products_) {
        if (product.enabled) {
            enabled.push_back(product);
        }
    }
    return enabled;
}

bool ConfigProvider::isSymbolEnabled(const std::string& symbol) const {
    const ProductConfig* config = getProductConfig(symbol);
    return config != nullptr && config->enabled;
}

size_t ConfigProvider::getEnabledProductCount() const {
    size_t count = 0;
    for (const auto& product : products_) {
        if (product.enabled) {
            ++count;
        }
    }
    return count;
}

} // namespace beacon::client_algorithm
