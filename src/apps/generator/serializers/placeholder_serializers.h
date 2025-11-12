#pragma once

#include "iserialize_market_data_messages.h"

namespace beacon::market_data_generator::serializers {

class NsdqSerializer : public ISerializeMarketDataMessages {
  public:
    void serialize() override {
        // Placeholder implementation
    }
};

class CmeSerializer : public ISerializeMarketDataMessages {
  public:
    void serialize() override {
        // Placeholder implementation
    }
};

class NyseSerializer : public ISerializeMarketDataMessages {
  public:
    void serialize() override {
        // Placeholder implementation
    }
};

} // namespace beacon::market_data_generator::serializers
