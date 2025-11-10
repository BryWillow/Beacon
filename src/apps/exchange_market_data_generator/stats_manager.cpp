/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Implements real-time statistics collection and formatted
 *               reporting of market data generation metrics including bid/ask
 *               prices, spreads, and volume distribution.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include "stats_manager.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

void StatsManager::updateStats(const std::string& symbol, bool isBid, size_t quantity, double price) {
    auto& stats = _symbolStats[symbol];
    stats.orders++;
    
    // Update min/max price
    if (stats.orders == 1) {
        stats.minPrice = price;
        stats.maxPrice = price;
    } else {
        stats.minPrice = std::min(stats.minPrice, price);
        stats.maxPrice = std::max(stats.maxPrice, price);
    }
    
    // Track bid vs ask prices separately for accurate spread calculation
    if (isBid) {
        stats.bidQty += quantity;
        stats.totalBidPrc += price * quantity;
    } else {
        stats.askQty += quantity;
        stats.totalAskPrc += price * quantity;
    }
}

void StatsManager::updateTradeStats(const std::string& symbol, size_t quantity [[maybe_unused]], double price) {
    auto& stats = _symbolStats[symbol];
    stats.trades++;
    
    // Update min/max price for trades as well
    if (stats.orders == 0 && stats.trades == 1) {
        stats.minPrice = price;
        stats.maxPrice = price;
    } else {
        stats.minPrice = std::min(stats.minPrice, price);
        stats.maxPrice = std::max(stats.maxPrice, price);
    }
}

void StatsManager::printStats(size_t totalMessages [[maybe_unused]], double elapsedSeconds [[maybe_unused]]) const {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════════════════════════════\n";
    std::cout << "                              MARKET DATA GENERATION SUMMARY                                           \n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════════════════════════════\n\n";
    
    // Calculate totals first
    size_t totalOrders = 0;
    size_t totalTrades = 0;
    size_t totalVolume = 0;
    
    for (const auto& [symbol, stats] : _symbolStats) {
        totalOrders += stats.orders;
        totalTrades += stats.trades;
        totalVolume += stats.bidQty + stats.askQty;
    }
    
    // Header with column labels
    std::cout << std::left;
    std::cout << "  " << std::setw(8) << "Symbol"
              << std::setw(10) << "Orders"
              << std::setw(10) << "Trades"
              << std::setw(12) << "TotalVol"
              << std::setw(12) << "AvgBid"
              << std::setw(12) << "AvgAsk"
              << std::setw(12) << "Spread"
              << std::setw(12) << "Spread%"
              << std::setw(12) << "Min-Max"
              << "\n";
    
    std::cout << "  " << std::string(8, '-')
              << std::string(10, '-')
              << std::string(10, '-')
              << std::string(12, '-')
              << std::string(12, '-')
              << std::string(12, '-')
              << std::string(12, '-')
              << std::string(12, '-')
              << std::string(12, '-')
              << "\n";
    
    // Symbol rows
    for (const auto& [symbol, stats] : _symbolStats) {
        // Calculate volume-weighted average prices for bid and ask sides
        double avgBidPrc = stats.bidQty > 0 ? stats.totalBidPrc / stats.bidQty : 0.0;
        double avgAskPrc = stats.askQty > 0 ? stats.totalAskPrc / stats.askQty : 0.0;
        
        // Spread is the difference between average ask and average bid
        // This should always be positive in a normal market (ask > bid)
        double spread = avgAskPrc - avgBidPrc;
        double spreadPercent = avgAskPrc > 0 ? (spread / avgAskPrc) * 100.0 : 0.0;
        double priceRange = stats.maxPrice - stats.minPrice;
        size_t totalVol = stats.bidQty + stats.askQty;
        
        std::cout << "  " << std::setw(8) << symbol
                  << std::setw(10) << stats.orders
                  << std::setw(10) << stats.trades
                  << std::setw(12) << totalVol
                  << std::fixed << std::setprecision(2)
                  << std::setw(12) << avgBidPrc
                  << std::setw(12) << avgAskPrc
                  << std::setw(12) << spread
                  << std::setw(12) << spreadPercent
                  << std::setw(12) << priceRange
                  << "\n";
    }
    
    std::cout << "  " << std::string(8, '-')
              << std::string(10, '-')
              << std::string(10, '-')
              << std::string(12, '-')
              << std::string(80, ' ')
              << "\n";
    
    // Totals row
    std::cout << "  " << std::setw(8) << "TOTAL"
              << std::setw(10) << totalOrders
              << std::setw(10) << totalTrades
              << std::setw(12) << totalVolume
              << "\n\n";
    
    std::cout << "═══════════════════════════════════════════════════════════════════════════════════════════════════════\n";
}
