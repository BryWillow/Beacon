/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Implements NYSE Pillar v1.9 binary serialization with proper
 *               message type handling, fixed-point price encoding, and side
 *               indicators for order book construction.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <cstring>
#include <fstream>
#include <stdexcept>

#include <nyse/market_data/pillar_market_data/current/messages_active.h>

#include "nyse_serializer.h"

namespace beacon::market_data_generator::serializers {

NyseMarketDataSerializer::NyseMarketDataSerializer(const std::string& outputFilePath)
    : _outputFilePath(outputFilePath), _outputFile(outputFilePath, std::ios::binary) {
    if (!_outputFile) {
        throw std::runtime_error("Failed to open output file: " + _outputFilePath);
    }
}

NyseMarketDataSerializer::~NyseMarketDataSerializer() {
    if (_outputFile.is_open()) {
        _outputFile.close();
    }
}

void NyseMarketDataSerializer::serializeMessage(const Message& message) const {
    using namespace beacon::nyse::market_data;

    // Determine message type based on the message properties
    if (message.messageType == MessageType::Last) {
        // Serialize as Trade message
        NyseTradeMessage tradeMsg{};
        tradeMsg.sequenceNumber = message.sequenceNumber;
        
        // Copy symbol (pad with spaces if needed)
        std::memset(tradeMsg.symbol, ' ', sizeof(tradeMsg.symbol));
        std::memcpy(tradeMsg.symbol, message.symbol.c_str(), 
                    std::min(message.symbol.size(), sizeof(tradeMsg.symbol)));
        
        // NYSE price is in fixed-point (assuming 1/10000 for consistency)
        tradeMsg.price = static_cast<uint32_t>(message.price * 10000.0);
        tradeMsg.shares = static_cast<uint32_t>(message.quantity);
        tradeMsg.tradeId = message.sequenceNumber; // Using sequence as trade ID
        tradeMsg.side = (message.type == "Buy") ? 'B' : 'S';
        
        std::memset(tradeMsg.reserved, 0, sizeof(tradeMsg.reserved));

        _outputFile.write(reinterpret_cast<const char*>(&tradeMsg), sizeof(tradeMsg));
    }
    else {
        // Serialize as Quote message (for Bid/Ask)
        NyseQuoteMessage quoteMsg{};
        quoteMsg.sequenceNumber = message.sequenceNumber;
        
        // Copy symbol (pad with spaces if needed)
        std::memset(quoteMsg.symbol, ' ', sizeof(quoteMsg.symbol));
        std::memcpy(quoteMsg.symbol, message.symbol.c_str(), 
                    std::min(message.symbol.size(), sizeof(quoteMsg.symbol)));
        
        // NYSE price is in fixed-point (assuming 1/10000 for consistency)
        uint32_t priceFixed = static_cast<uint32_t>(message.price * 10000.0);
        uint32_t size = static_cast<uint32_t>(message.quantity);
        
        if (message.type == "Buy") {
            quoteMsg.bidPrice = priceFixed;
            quoteMsg.bidSize = size;
            quoteMsg.askPrice = priceFixed + 10; // Spread of 0.001
            quoteMsg.askSize = size;
        }
        else {
            quoteMsg.bidPrice = priceFixed - 10; // Spread of 0.001
            quoteMsg.bidSize = size;
            quoteMsg.askPrice = priceFixed;
            quoteMsg.askSize = size;
        }
        
        std::memset(quoteMsg.reserved, 0, sizeof(quoteMsg.reserved));

        _outputFile.write(reinterpret_cast<const char*>(&quoteMsg), sizeof(quoteMsg));
    }

    // Flush periodically for reliability
    if (message.sequenceNumber % 1000 == 0) {
        _outputFile.flush();
    }
}

} // namespace beacon::market_data_generator::serializers