#pragma once

#include <string>

class ProtocolGenerator {
public:
    virtual ~ProtocolGenerator() = default;

    // Generate a single message for the protocol
    virtual std::string generateMessage(const std::string& symbol, bool isBuy, size_t quantity, double price) const = 0;
};
