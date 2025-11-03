#pragma once

#include "protocol_generator.h"

// This class is a placeholder for generating FIX messages.
// Currently, the application only supports NSDQ ITCH.
// Future development may include support for FIX.
class FIXGenerator : public ProtocolGenerator {
public:
    std::string generateMessage(const std::string& symbol, bool isBuy, size_t quantity, double price) const override {
        return "8=FIX.4.4|35=D|55=" + symbol + "|54=" + (isBuy ? "1" : "2") +
               "|38=" + std::to_string(quantity) +
               "|44=" + std::to_string(price) + "|";
    }
};
