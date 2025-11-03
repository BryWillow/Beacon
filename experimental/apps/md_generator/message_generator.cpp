#include "message_generator.h"
#include "config_provider.h" // Ensure ConfigProvider is used
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <iomanip> // For std::setw
#include <cmath> // For std::pow

MessageGenerator::MessageGenerator(const std::string& configPath) {
    ConfigProvider configProvider(configPath);

    _message_count = configProvider.getMessageCount();
    auto symbolsForGeneration = configProvider.getSymbolsForGeneration();

    // Convert SymbolGenerationData to SymbolParameters
    for (const auto& symbolData : symbolsForGeneration) {
        SymbolParameters symbolParams;
        symbolParams.symbol = symbolData.symbol;
        symbolParams.percent = symbolData.percent;
        symbolParams.price_range = symbolData.price_range; // Fixed: Use singular price_range
        symbolParams.quantity_range = symbolData.quantity_range; // Fixed: Use singular quantity_range
        _symbols.push_back(symbolParams);
    }
}

void MessageGenerator::generateMessages(const std::string& outputPath, size_t numMessages) {
    std::ofstream outFile(outputPath);
    if (!outFile) {
        throw std::runtime_error("Failed to open output file: " + outputPath);
    }

    // Calculate the number of messages for each symbol based on percentages
    std::vector<size_t> messagesPerSymbol;
    size_t totalMessagesAssigned = 0;
    for (const auto& symbol : _symbols) {
        size_t messagesForSymbol = static_cast<size_t>(std::round(numMessages * (symbol.percent / 100.0)));
        messagesPerSymbol.push_back(messagesForSymbol);
        totalMessagesAssigned += messagesForSymbol;
    }

    // Adjust for rounding errors to ensure the total number of messages matches numMessages
    if (totalMessagesAssigned != numMessages) {
        size_t adjustment = numMessages - totalMessagesAssigned;
        messagesPerSymbol[0] += adjustment; // Adjust the first symbol to balance the total
    }

    // Print the first line of the output
    std::cout << "\n"; // Add a newline before the message
    std::cout << "Generating " << numMessages << " ITCH messages...\n";

    std::random_device rd;
    std::mt19937 gen(rd());

    // Generate messages for each symbol
    for (size_t i = 0; i < _symbols.size(); ++i) {
        const SymbolParameters& symbolParams = _symbols[i];
        size_t messagesForSymbol = messagesPerSymbol[i];

        const auto& priceRange = symbolParams.price_range;
        const auto& quantityRange = symbolParams.quantity_range;

        for (size_t j = 0; j < messagesForSymbol; ++j) {
            bool isBuy = (j % 2 == 0);

            // Generate a biased price using the weight
            double priceBias = std::pow(static_cast<double>(j) / messagesForSymbol, priceRange.weight);
            double price = priceRange.min_price + priceBias * (priceRange.max_price - priceRange.min_price);

            // Generate a biased quantity using the weight
            double quantityBias = std::pow(static_cast<double>(j) / messagesForSymbol, quantityRange.weight);
            size_t quantity = static_cast<size_t>(quantityRange.min_quantity + quantityBias * (quantityRange.max_quantity - quantityRange.min_quantity));

            outFile << "Symbol: " << symbolParams.symbol << ", "
                    << "Type: " << (isBuy ? "Buy" : "Sell") << ", "
                    << "Quantity: " << quantity << ", "
                    << "Price: " << price << "\n";

            _statsManager.updateStats(symbolParams.symbol, isBuy, quantity, price); // Use StatsManager
        }
    }

    // Ensure all orders are in a terminal state
    for (const auto& symbolParams : _symbols) {
        outFile << "Symbol: " << symbolParams.symbol << ", "
                << "Type: Cancel, "
                << "Quantity: 0, "
                << "Price: 0.0\n";
    }

    std::cout << "Generation Complete. See results below:\n\n";
    _statsManager.printStats(); // Use StatsManager
}

size_t MessageGenerator::getMessageCount() const {
    return _message_count;
}
