#pragma once

#include <string>
#include <vector>
#include "symbol_parameters.h" // For PriceRange and QuantityRange

struct SymbolGenerationData {
    std::string symbol;
    double percent; // Field to store the percentage
    std::vector<PriceRange> priceRanges;
    std::vector<QuantityRange> quantityRanges;
};
