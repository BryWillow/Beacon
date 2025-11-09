/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Defines parameter structures for symbol-specific message
 *               generation including price ranges, quantity ranges, spread
 *               percentages, and distribution weights.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <string>

// Each symbol must generate messages within this min/max quantity.
struct QuantityRange {
    uint32_t min_quantity;
    uint32_t max_quantity;
    double weight;
};

// Each symbol must generate messages within this min/max price.
struct PriceRange {
    double min_price;
    double max_price;
    double weight;
};

// All of the parameters needed to generate messages for a given symbol.
// Percent indicates the relative number of messages created for this symbol,
// as compared to other symbols included in this file.
struct SymbolParameters {
    std::string symbol;
    double percent;
    double spread_percent;  // Bid-ask spread as percentage (e.g., 0.5 for 0.5%)
    PriceRange price_range; // Singular price range
    QuantityRange quantity_range; // Singular quantity range
};