#pragma once

#include <string>
#include <unordered_map>

struct SymbolStats {
    size_t orders = 0;
    size_t buyQty = 0;
    size_t sellQty = 0;
    double totalBuyPrc = 0.0;
    double totalSellPrc = 0.0;
};

class StatsManager {
public:
    void updateStats(const std::string& symbol, bool isBuy, size_t quantity, double price);
    void printStats() const; // Ensure this matches the implementation
private:
    std::unordered_map<std::string, SymbolStats> _symbolStats;
};
