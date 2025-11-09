/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Tracks and reports statistics for generated market data
 *               including order counts, volumes, average prices, and spreads
 *               per symbol.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <string>
#include <unordered_map>

struct SymbolStats {
    size_t orders = 0;
    size_t trades = 0;
    size_t bidQty = 0;      // Quantity on bid side (buy orders)
    size_t askQty = 0;      // Quantity on ask side (sell orders)
    double totalBidPrc = 0.0;  // Weighted sum of bid prices
    double totalAskPrc = 0.0;  // Weighted sum of ask prices
    double minPrice = 0.0;
    double maxPrice = 0.0;
};

class StatsManager {
public:
    void updateStats(const std::string& symbol, bool isBid, size_t quantity, double price);
    void updateTradeStats(const std::string& symbol, size_t quantity, double price);
    void printStats(size_t totalMessages, double elapsedSeconds) const;
private:
    std::unordered_map<std::string, SymbolStats> _symbolStats;
};
