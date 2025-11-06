#pragma once

#include "iserialize_exchange_messages.h"
#include <sstream>
#include <iomanip>

namespace beacon::market_data_generator::serializers {

  class SerializerBase : public ISerializeExchangeMessages {
  protected:
      // Utility method to format a message's common fields
      std::string formatCommonFields(const Message& message) const {
          std::ostringstream oss;
          oss << "Symbol:" << message.symbol
              << "|Type:" << message.type
              << "|Quantity:" << message.quantity
              << "|Price:" << std::fixed << std::setprecision(2) << message.price
              << "|Seq:" << message.sequenceNumber
              << "|Timestamp:" << message.timestamp;
          return oss.str();
      }
  };
} // namespace beacon::market_data_generator::serializers