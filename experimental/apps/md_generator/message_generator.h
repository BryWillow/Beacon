#pragma once

#include <string>
#include <vector>
#include "stats_manager.h"
#include "symbol_parameters.h" // Include the new header for SymbolParameters

class MessageGenerator {
public:
    explicit MessageGenerator(const std::string& configPath);

    // Delete unwanted constructors
    MessageGenerator() = delete;
    MessageGenerator(const MessageGenerator&) = delete;
    MessageGenerator& operator=(const MessageGenerator&) = delete;
    MessageGenerator(MessageGenerator&&) = delete;
    MessageGenerator& operator=(MessageGenerator&&) = delete;

    void generateMessages(const std::string& outputPath, size_t numMessages);
    size_t getMessageCount() const;

private:
    std::vector<SymbolParameters> _symbols;
    StatsManager _statsManager;
    size_t _message_count = 10000;
};
