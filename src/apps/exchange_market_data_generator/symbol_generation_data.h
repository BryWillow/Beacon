#pragma once

#include <string>
#include "symbol_parameters.h" // For PriceRange and QuantityRange

struct SymbolGenerationData {
    std::string symbol;
    double percent; // Field to store the percentage
    PriceRange price_range; // Single price range
    QuantityRange quantity_range; // Single quantity range
};
