/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Implements CME MDP MBO v4.0 binary serialization with proper
 *               message type mapping, order ID generation, and fixed-point
 *               price encoding for Market by Order data.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <cme/market_data/mdp_mbo/current/messages_active.h>

#include "cme_serializer.h"

namespace beacon::market_data_generator::serializers {

CmeMarketDataSerializer::CmeMarketDataSerializer(const std::string& outputFilePath)
    : _outputFilePath(outputFilePath), 
      _outputFile(outputFilePath, std::ios::binary),
      _nextOrderId(1) {
    if (!_outputFile) {
        throw std::runtime_error("Failed to open output file: " + _outputFilePath);
    }
}

CmeMarketDataSerializer::~CmeMarketDataSerializer() {
    if (_outputFile.is_open()) {
        _outputFile.close();
    }
}

void CmeMarketDataSerializer::serializeMessage(const Message& message) const {
    using namespace beacon::cme::market_data;

    // Determine message type based on the message properties
    if (message.messageType == MessageType::Last) {
        // Serialize as Trade message
        CmeMboTrade tradeMsg{};
        tradeMsg.sequenceNumber = message.sequenceNumber;
        tradeMsg.orderId = _nextOrderId++;
        
        // Copy symbol (pad with spaces if needed)
        std::memset(tradeMsg.symbol, ' ', sizeof(tradeMsg.symbol));
        std::memcpy(tradeMsg.symbol, message.symbol.c_str(), 
                    std::min(message.symbol.size(), sizeof(tradeMsg.symbol)));
        
        // CME price is in fixed-point (assuming 1/10000 for consistency)
        tradeMsg.price = static_cast<uint32_t>(message.price * 10000.0);
        tradeMsg.quantity = static_cast<uint32_t>(message.quantity);
        tradeMsg.tradeId = message.sequenceNumber;
        tradeMsg.side = (message.type == "Buy") ? 'B' : 'S';
        
        std::memset(tradeMsg.reserved, 0, sizeof(tradeMsg.reserved));

        _outputFile.write(reinterpret_cast<const char*>(&tradeMsg), sizeof(tradeMsg));
    }
    else {
        // Serialize as AddOrder message (for Bid/Ask)
        CmeMboAddOrder addMsg{};
        addMsg.sequenceNumber = message.sequenceNumber;
        addMsg.orderId = _nextOrderId++;
        
        // Copy symbol (pad with spaces if needed)
        std::memset(addMsg.symbol, ' ', sizeof(addMsg.symbol));
        std::memcpy(addMsg.symbol, message.symbol.c_str(), 
                    std::min(message.symbol.size(), sizeof(addMsg.symbol)));
        
        // CME price is in fixed-point (assuming 1/10000 for consistency)
        addMsg.price = static_cast<uint32_t>(message.price * 10000.0);
        addMsg.quantity = static_cast<uint32_t>(message.quantity);
        addMsg.side = (message.type == "Buy") ? 'B' : 'S';
        addMsg.orderType = 'L'; // Limit order
        
        std::memset(addMsg.reserved, 0, sizeof(addMsg.reserved));

        _outputFile.write(reinterpret_cast<const char*>(&addMsg), sizeof(addMsg));
    }

    // Flush periodically for reliability
    if (message.sequenceNumber % 1000 == 0) {
        _outputFile.flush();
    }
}

} // namespace beacon::market_data_generator::serializers