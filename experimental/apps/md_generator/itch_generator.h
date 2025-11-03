#pragma once

#include "protocol_generator.h"

class ITCHGenerator : public ProtocolGenerator {
public:
    std::string generateMessage(const std::string& symbol, bool isBuy, size_t quantity, double price) const override {
        return "Symbol: " + symbol + ", Type: " + (isBuy ? "Buy" : "Sell") +
               ", Quantity: " + std::to_string(quantity) +
               ", Price: " + std::to_string(price);
    }
};
