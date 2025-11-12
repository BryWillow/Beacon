/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_generator
 * Purpose:      Defines the in-memory message structure used for passing market
 *               data between the generator and exchange-specific serializers.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <cstdint>
#include <string>

namespace beacon::market_data_generator {

  enum class MessageType {
      Bid,
      Ask,
      Last
  };

  class Message {
  public:
      std::string symbol;
      MessageType messageType; // Bid, Ask, or Last
      std::string type;        // "Buy" or "Sell"
      size_t quantity;
      double price;
      uint64_t sequenceNumber; // Unique sequence number for the message
      uint64_t timestamp;      // Timestamp in epoch milliseconds

      Message(const std::string& symbol, MessageType messageType, const std::string& type, size_t quantity, double price, uint64_t sequenceNumber, uint64_t timestamp)
          : symbol(symbol), messageType(messageType), type(type), quantity(quantity), price(price), sequenceNumber(sequenceNumber), timestamp(timestamp) {}
  };
} // namespace beacon::market_data_generator