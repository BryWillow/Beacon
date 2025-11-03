#include "stats_manager.h"
#include <iostream>
#include <iomanip>

void StatsManager::updateStats(const std::string& symbol, bool isBuy, size_t quantity, double price) {
    auto& stats = _symbolStats[symbol]; // Automatically creates entry if symbol doesn't exist
    stats.orders++;
    if (isBuy) {
        stats.buyQty += quantity;
        stats.totalBuyPrc += price * quantity;
    } else {
        stats.sellQty += quantity;
        stats.totalSellPrc += price * quantity;
    }
}

void StatsManager::printStats() const {
    std::cout << std::left;
    std::cout << std::setw(9) << "Symbol"        // Shrunk by 3
              << " |  " << std::setw(10) << "OrderCnt"
              << " |  " << std::setw(10) << "BuyQty"       // Shrunk by 2
              << " |  " << std::setw(10) << "SellQty"      // Shrunk by 2
              << " |  " << std::setw(12) << "AvgBuyPrc"    // Shrunk by 2
              << " |  " << std::setw(12) << "AvgSellPrc"   // Shrunk by 2
              << "\n";

    // Print dashes under each column header with proper alignment
    std::cout << std::string(9, '-') << " |  "  // Adjusted for "Symbol"
              << std::string(10, '-') << " |  "
              << std::string(10, '-') << " |  " // Adjusted for "BuyQty"
              << std::string(10, '-') << " |  " // Adjusted for "SellQty"
              << std::string(12, '-') << " |  " // Adjusted for "AvgBuyPrc"
              << std::string(12, '-')           // Adjusted for "AvgSellPrc"
              << "\n";

    for (const auto& [symbol, stats] : _symbolStats) {
        double avgBuyPrc = stats.buyQty > 0 ? stats.totalBuyPrc / stats.buyQty : 0.0;
        double avgSellPrc = stats.sellQty > 0 ? stats.totalSellPrc / stats.sellQty : 0.0;

        std::cout << std::setw(9) << symbol
                  << " |  " << std::setw(10) << stats.orders
                  << " |  " << std::setw(10) << stats.buyQty
                  << " |  " << std::setw(10) << stats.sellQty
                  << " |  " << std::setw(12) << avgBuyPrc
                  << " |  " << std::setw(12) << avgSellPrc
                  << "\n";
    }
}
