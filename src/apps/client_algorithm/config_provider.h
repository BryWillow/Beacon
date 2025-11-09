/*
 * =============================================================================
 * Project:      Beacon
 * Application:  client_algorithm
 * File:         config_provider.h
 * Purpose:      Configuration management for the trading algorithm client.
 *               Parses JSON configuration files and provides access to
 *               per-product settings including position limits and trading
 *               parameters.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace beacon::client_algorithm {

/**
 * @struct ProductConfig
 * @brief Configuration for a single tradeable product/symbol
 * 
 * Contains position and risk management parameters for each symbol
 * that the algorithm trades.
 */
struct ProductConfig {
    std::string symbol;          ///< Symbol name (e.g., "AAPL", "MSFT")
    int32_t starting_position;   ///< Initial position (positive=long, negative=short, 0=flat)
    uint32_t algo_position_limit;    ///< Algorithm's position limit (must be <= firm_position_limit)
    uint32_t firm_position_limit;    ///< Firm-wide position limit (hard regulatory/risk limit)
    bool enabled;                ///< Whether trading is enabled for this symbol
    
    /**
     * @brief Current position tracker (starts at starting_position)
     */
    mutable int32_t current_position;
    
    /**
     * @brief Constructor
     */
    ProductConfig() 
        : starting_position(0)
        , algo_position_limit(0)
        , firm_position_limit(0)
        , enabled(true)
        , current_position(0) {}
    
    /**
     * @brief Check if we can buy more of this product
     * @param quantity Number of shares to buy
     * @return true if buying this quantity won't exceed position limits
     */
    bool canBuy(uint32_t quantity) const {
        if (!enabled) return false;
        
        int32_t new_position = current_position + static_cast<int32_t>(quantity);
        
        // Check both limits - algo limit must always be <= firm limit
        uint32_t effective_limit = std::min(algo_position_limit, firm_position_limit);
        return new_position <= static_cast<int32_t>(effective_limit);
    }
    
    /**
     * @brief Check if we can sell (short) this product
     * @param quantity Number of shares to sell
     * @return true if selling this quantity won't exceed position limits
     */
    bool canSell(uint32_t quantity) const {
        if (!enabled) return false;
        
        int32_t new_position = current_position - static_cast<int32_t>(quantity);
        
        // Check both limits - algo limit must always be <= firm limit
        uint32_t effective_limit = std::min(algo_position_limit, firm_position_limit);
        return std::abs(new_position) <= static_cast<int32_t>(effective_limit);
    }
    
    /**
     * @brief Update position after a fill
     * @param quantity Quantity filled
     * @param isBuy true if buy, false if sell
     */
    void updatePosition(uint32_t quantity, bool isBuy) const {
        if (isBuy) {
            current_position += static_cast<int32_t>(quantity);
        } else {
            current_position -= static_cast<int32_t>(quantity);
        }
    }
    
    /**
     * @brief Get remaining capacity for buys
     * @return Number of shares that can still be bought
     */
    int32_t getRemainingBuyCapacity() const {
        uint32_t effective_limit = std::min(algo_position_limit, firm_position_limit);
        return static_cast<int32_t>(effective_limit) - current_position;
    }
    
    /**
     * @brief Get remaining capacity for sells
     * @return Number of shares that can still be sold
     */
    int32_t getRemainingSellCapacity() const {
        uint32_t effective_limit = std::min(algo_position_limit, firm_position_limit);
        return static_cast<int32_t>(effective_limit) + current_position;
    }
};

/**
 * @struct TradingConfig
 * @brief General trading parameters
 */
struct TradingConfig {
    uint32_t order_frequency;        ///< Send order every N market data messages
    char default_time_in_force;      ///< Default TIF: '0'=Day, '3'=IOC, '4'=FOK
    char default_capacity;           ///< Default capacity: 'A'=Agency, 'P'=Principal
    
    TradingConfig()
        : order_frequency(1000)
        , default_time_in_force('0')
        , default_capacity('A') {}
};

/**
 * @struct RiskManagementConfig
 * @brief Algorithm-level risk management parameters
 * 
 * PnL drawdown thresholds for algorithm-wide risk control.
 * All values are negative numbers representing losses.
 */
struct RiskManagementConfig {
    double pnl_drawdown_warning;     ///< Warning threshold (e.g., -5000.0 = warn at $5K loss)
    double pnl_drawdown_alert;       ///< Alert threshold (e.g., -10000.0 = alert at $10K loss)
    double pnl_drawdown_hard_stop;   ///< Hard stop threshold (e.g., -15000.0 = stop at $15K loss)
    
    uint32_t order_reject_warning;   ///< Warning threshold for order rejects (e.g., 10 rejects)
    uint32_t order_reject_alert;     ///< Alert threshold for order rejects (e.g., 25 rejects)
    uint32_t order_reject_hard_stop; ///< Hard stop threshold for order rejects (e.g., 50 rejects)
    
    uint32_t order_messaging_burst_warning;    ///< Warning threshold for order burst rate (orders/sec)
    uint32_t order_messaging_burst_alert;      ///< Alert threshold for order burst rate (orders/sec)
    uint32_t order_messaging_burst_hard_stop;  ///< Hard stop threshold for order burst rate (orders/sec)
    
    RiskManagementConfig()
        : pnl_drawdown_warning(-5000.0)
        , pnl_drawdown_alert(-10000.0)
        , pnl_drawdown_hard_stop(-15000.0)
        , order_reject_warning(10)
        , order_reject_alert(25)
        , order_reject_hard_stop(50)
        , order_messaging_burst_warning(100)
        , order_messaging_burst_alert(250)
        , order_messaging_burst_hard_stop(500) {}
    
    /**
     * @brief Check if current PnL triggers warning level
     * @param current_pnl Current PnL (negative = loss)
     * @return true if PnL is at or below warning threshold
     */
    bool isWarningLevel(double current_pnl) const {
        return current_pnl <= pnl_drawdown_warning;
    }
    
    /**
     * @brief Check if current PnL triggers alert level
     * @param current_pnl Current PnL (negative = loss)
     * @return true if PnL is at or below alert threshold
     */
    bool isAlertLevel(double current_pnl) const {
        return current_pnl <= pnl_drawdown_alert;
    }
    
    /**
     * @brief Check if current PnL triggers hard stop
     * @param current_pnl Current PnL (negative = loss)
     * @return true if PnL is at or below hard stop threshold
     */
    bool isHardStop(double current_pnl) const {
        return current_pnl <= pnl_drawdown_hard_stop;
    }
    
    /**
     * @brief Check if order reject count triggers warning level
     * @param reject_count Current number of order rejects
     * @return true if reject count is at or above warning threshold
     */
    bool isRejectWarningLevel(uint32_t reject_count) const {
        return reject_count >= order_reject_warning;
    }
    
    /**
     * @brief Check if order reject count triggers alert level
     * @param reject_count Current number of order rejects
     * @return true if reject count is at or above alert threshold
     */
    bool isRejectAlertLevel(uint32_t reject_count) const {
        return reject_count >= order_reject_alert;
    }
    
    /**
     * @brief Check if order reject count triggers hard stop
     * @param reject_count Current number of order rejects
     * @return true if reject count is at or above hard stop threshold
     */
    bool isRejectHardStop(uint32_t reject_count) const {
        return reject_count >= order_reject_hard_stop;
    }
    
    /**
     * @brief Check if order burst rate triggers warning level
     * @param orders_per_second Current order submission rate
     * @return true if rate is at or above warning threshold
     */
    bool isBurstWarningLevel(uint32_t orders_per_second) const {
        return orders_per_second >= order_messaging_burst_warning;
    }
    
    /**
     * @brief Check if order burst rate triggers alert level
     * @param orders_per_second Current order submission rate
     * @return true if rate is at or above alert threshold
     */
    bool isBurstAlertLevel(uint32_t orders_per_second) const {
        return orders_per_second >= order_messaging_burst_alert;
    }
    
    /**
     * @brief Check if order burst rate triggers hard stop
     * @param orders_per_second Current order submission rate
     * @return true if rate is at or above hard stop threshold
     */
    bool isBurstHardStop(uint32_t orders_per_second) const {
        return orders_per_second >= order_messaging_burst_hard_stop;
    }
    
    /**
     * @brief Validate that PnL thresholds are in correct order
     * @return true if warning >= alert >= hard_stop (all negative)
     */
    bool validatePnL() const {
        return (pnl_drawdown_warning >= pnl_drawdown_alert) &&
               (pnl_drawdown_alert >= pnl_drawdown_hard_stop);
    }
    
    /**
     * @brief Validate that reject thresholds are in correct order
     * @return true if warning <= alert <= hard_stop (all positive)
     */
    bool validateRejects() const {
        return (order_reject_warning <= order_reject_alert) &&
               (order_reject_alert <= order_reject_hard_stop);
    }
    
    /**
     * @brief Validate that burst thresholds are in correct order
     * @return true if warning <= alert <= hard_stop (all positive)
     */
    bool validateBurst() const {
        return (order_messaging_burst_warning <= order_messaging_burst_alert) &&
               (order_messaging_burst_alert <= order_messaging_burst_hard_stop);
    }
    
    /**
     * @brief Validate all thresholds
     * @return true if all thresholds are valid
     */
    bool validate() const {
        return validatePnL() && validateRejects() && validateBurst();
    }
};

/**
 * @struct ConnectionConfig
 * @brief Network connection configuration
 */
struct ConnectionConfig {
    std::string host;
    uint16_t port;
    
    ConnectionConfig() : port(0) {}
};

/**
 * @class ConfigProvider
 * @brief Loads and provides access to client algorithm configuration
 * 
 * Parses JSON configuration files containing:
 * - Market data connection settings
 * - Exchange connection settings
 * - Per-product position limits and settings
 * - General trading parameters
 */
class ConfigProvider {
public:
    /**
     * @brief Load configuration from JSON file
     * @param filepath Path to JSON configuration file
     * @return true if loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& filepath);
    
    /**
     * @brief Get market data connection configuration
     */
    const ConnectionConfig& getMarketDataConfig() const { return market_data_config_; }
    
    /**
     * @brief Get exchange connection configuration
     */
    const ConnectionConfig& getExchangeConfig() const { return exchange_config_; }
    
    /**
     * @brief Get trading parameters
     */
    const TradingConfig& getTradingConfig() const { return trading_config_; }
    
    /**
     * @brief Get risk management parameters
     */
    const RiskManagementConfig& getRiskManagementConfig() const { return risk_config_; }
    
    /**
     * @brief Get configuration for a specific product
     * @param symbol Symbol name (e.g., "AAPL")
     * @return Pointer to ProductConfig or nullptr if not found
     */
    const ProductConfig* getProductConfig(const std::string& symbol) const;
    
    /**
     * @brief Get all product configurations
     */
    const std::vector<ProductConfig>& getAllProducts() const { return products_; }
    
    /**
     * @brief Get only enabled products
     */
    std::vector<ProductConfig> getEnabledProducts() const;
    
    /**
     * @brief Check if a symbol is configured and enabled
     */
    bool isSymbolEnabled(const std::string& symbol) const;
    
    /**
     * @brief Get number of configured products
     */
    size_t getProductCount() const { return products_.size(); }
    
    /**
     * @brief Get number of enabled products
     */
    size_t getEnabledProductCount() const;
    
private:
    ConnectionConfig market_data_config_;
    ConnectionConfig exchange_config_;
    TradingConfig trading_config_;
    RiskManagementConfig risk_config_;
    std::vector<ProductConfig> products_;
    std::map<std::string, size_t> symbol_index_map_;  ///< Fast lookup: symbol -> index
    
    /**
     * @brief Parse market data config section
     */
    bool parseMarketDataConfig(const nlohmann::json& j);
    
    /**
     * @brief Parse exchange config section
     */
    bool parseExchangeConfig(const nlohmann::json& j);
    
    /**
     * @brief Parse trading config section
     */
    bool parseTradingConfig(const nlohmann::json& j);
    
    /**
     * @brief Parse risk management config section
     */
    bool parseRiskManagementConfig(const nlohmann::json& j);
    
    /**
     * @brief Parse products array
     */
    bool parseProducts(const nlohmann::json& j);
    
    /**
     * @brief Build symbol index map for fast lookups
     */
    void buildSymbolIndex();
};

} // namespace beacon::client_algorithm
