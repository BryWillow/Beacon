/*
 * Project: Beacon
 * File   : execution_message_types.h
 * Purpose:  v3.2 Pillar Message Structs for direct binary parsing.
 * Field order, types, and alignment must match the official NYSE specification.
 * Designed for direct memcpy from raw binary messages.

 * Do not change field order, types, or alignment unless updating for a new protocol version.
 * For details, see NYSE Pillar Market Data Binary Specification v3.2.
 */

#pragma once

#include <cstdint>

namespace beacon::nyse::execution {

  struct alignas(64) NyseOrderEntryMessage {
      uint64_t clientOrderId;
      char symbol[8];
      uint32_t shares;
      uint32_t price;
      char side;
      char orderType;
      char tif;
      char reserved1;
      uint16_t reserved2;
      char _padding[34];
      static_assert(sizeof(NyseOrderEntryMessage) == 64, "Struct must be 64 bytes");
  };

  struct alignas(64) NyseCancelOrderMessage {
      uint64_t clientOrderId;
      uint32_t canceledShares;
      uint16_t reserved;
      char _padding[50];
      static_assert(sizeof(NyseCancelOrderMessage) == 64, "Struct must be 64 bytes");
  };

  struct alignas(64) NyseReplaceOrderMessage {
      uint64_t originalOrderId;
      uint64_t newOrderId;
      uint32_t shares;
      uint32_t price;
      char side;
      char orderType;
      char tif;
      char reserved1;
      uint16_t reserved2;
      char _padding[34];
      static_assert(sizeof(NyseReplaceOrderMessage) == 64, "Struct must be 64 bytes");
  };

  struct alignas(32) NyseOrderAcceptedMessage {
      uint64_t clientOrderId;
      char symbol[8];
      uint32_t shares;
      uint32_t price;
      char side;
      char orderType;
      char tif;
      char reserved[3];
      static_assert(sizeof(NyseOrderAcceptedMessage) == 32, "Struct must be 32 bytes");
  };

  struct alignas(32) NyseOrderRejectedMessage {
      uint64_t clientOrderId;
      char symbol[8];
      uint32_t rejectReason; // NYSE reason code
      char reserved[12];
      static_assert(sizeof(NyseOrderRejectedMessage) == 32, "Struct must be 32 bytes");
  };

  struct alignas(32) NyseOrderCancelledMessage {
      uint64_t clientOrderId;
      char symbol[8];
      uint32_t cancelledShares;
      char reserved[12];
      static_assert(sizeof(NyseOrderCancelledMessage) == 32, "Struct must be 32 bytes");
  };

  struct alignas(32) NyseOrderReplacedMessage {
      uint64_t originalOrderId;
      uint64_t newOrderId;
      char symbol[8];
      uint32_t shares;
      uint32_t price;
      char reserved[4];
      static_assert(sizeof(NyseOrderReplacedMessage) == 32, "Struct must be 32 bytes");
  };

  struct alignas(32) NyseTradeExecutionMessage {
      uint64_t clientOrderId;
      char symbol[8];
      uint32_t executedShares;
      uint32_t executionPrice;
      uint64_t tradeId;
      char reserved[4];
      static_assert(sizeof(NyseTradeExecutionMessage) == 32, "Struct must be 32 bytes");
  };
}  // namespace beacon::nyse::execution