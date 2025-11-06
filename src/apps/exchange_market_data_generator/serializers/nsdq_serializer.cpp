/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Implements NASDAQ ITCH v5.0 binary serialization with proper
 *               message type handling, fixed-point price encoding, and
 *               periodic buffer flushing for performance.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <cstring>
#include <fstream>
#include <stdexcept>

#include <nsdq/market_data/itch/current/messages_active.h>

#include "nsdq_serializer.h"

namespace beacon::market_data_generator::serializers {

NsdqMarketDataSerializer::NsdqMarketDataSerializer(const std::string& outputPath, size_t flushInterval)
    : _outputPath(outputPath), _flushInterval(flushInterval), _outputFile(outputPath, std::ios::binary) {
    if (!_outputFile) {
        throw std::runtime_error("Failed to open output file: " + _outputPath);
    }
}

NsdqMarketDataSerializer::~NsdqMarketDataSerializer() {
    if (_outputFile.is_open()) {
        _outputFile.close();
    }
}

void NsdqMarketDataSerializer::serializeMessage(const Message& message) const {
    using namespace beacon::nsdq::market_data::itch;

    // Determine message type based on the message properties
    if (message.messageType == MessageType::Last) {
        // Serialize as Trade message
        TradeMessage tradeMsg{};
        tradeMsg.sequenceNumber = message.sequenceNumber;
        tradeMsg.orderRefNum = message.sequenceNumber; // Using sequence as ref
        tradeMsg.side = (message.type == "Buy") ? 'B' : 'S';
        tradeMsg.shares = static_cast<uint32_t>(message.quantity);
        
        // Copy symbol (pad with spaces if needed)
        std::memset(tradeMsg.stock, ' ', sizeof(tradeMsg.stock));
        std::memcpy(tradeMsg.stock, message.symbol.c_str(), 
                    std::min(message.symbol.size(), sizeof(tradeMsg.stock)));
        
        // ITCH price is in 1/10000 dollars
        tradeMsg.price = static_cast<uint32_t>(message.price * 10000.0);

        _outputFile.write(reinterpret_cast<const char*>(&tradeMsg), sizeof(tradeMsg));
    }
    else {
        // Serialize as AddOrder message (for Bid/Ask)
        AddOrderMessage addMsg{};
        addMsg.sequenceNumber = message.sequenceNumber;
        addMsg.orderRefNum = message.sequenceNumber;
        
        // Copy symbol (pad with spaces if needed)
        std::memset(addMsg.stock, ' ', sizeof(addMsg.stock));
        std::memcpy(addMsg.stock, message.symbol.c_str(), 
                    std::min(message.symbol.size(), sizeof(addMsg.stock)));
        
        addMsg.shares = static_cast<uint32_t>(message.quantity);
        
        // ITCH price is in 1/10000 dollars
        addMsg.price = static_cast<uint32_t>(message.price * 10000.0);
        
        addMsg.side = (message.type == "Buy") ? 'B' : 'S';

        _outputFile.write(reinterpret_cast<const char*>(&addMsg), sizeof(addMsg));
    }

    // Flush periodically based on configured interval
    if (message.sequenceNumber % _flushInterval == 0) {
        _outputFile.flush();
    }
}

} // namespace beacon::market_data_generator::serializers