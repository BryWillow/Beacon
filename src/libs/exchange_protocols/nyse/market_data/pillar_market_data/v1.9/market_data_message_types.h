/*
 * Project: Beacon
 * File   : market_data_message_types.h
 * Purpose:  v1.9 Pillar Msesage Structs for direct binary parsing.
 * Field order, types, and alignment must match the official NYSE specification.
 * Designed for direct memcpy from raw binary messages.
 *
 * Do not change field order, types, or alignment unless updating for a new protocol version.
 * For details, see NYSE Pillar Market Data Binary Specification v1.9.
 */

#pragma once

#include <cstdint>

namespace beacon::nyse::market_data {

  struct alignas(32) NyseQuoteMessage {
      uint64_t sequenceNumber;
      char symbol[8];
      uint32_t bidPrice;
      uint32_t bidSize;
      uint32_t askPrice;
      uint32_t askSize;
      char reserved[4];
      //static_assert(sizeof(NyseQuoteMessage) == 32, "Struct must be 32 bytes");
  };

  struct alignas(32) NyseBookUpdateMessage {
      uint64_t sequenceNumber;
      char symbol[8];
      uint32_t price;
      uint32_t size;
      uint32_t level;
      char side;
      char updateType;
      char reserved[5];
      //static_assert(sizeof(NyseBookUpdateMessage) == 32, "Struct must be 32 bytes");
  };

  struct alignas(32) NyseTradeMessage {
      uint64_t sequenceNumber;
      char symbol[8];
      uint32_t price;
      uint32_t shares;
      uint64_t tradeId;
      char side;
      char reserved[3];
      //static_assert(sizeof(NyseTradeMessage) == 32, "Struct must be 32 bytes");
  };
} // namespace beacon::nyse::market_data