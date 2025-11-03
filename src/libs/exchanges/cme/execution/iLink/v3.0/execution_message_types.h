/*
 * Project: Beacon
 * File: execution_message_types.h
 * CME iLink 3.0 execution protocol message structs for direct binary parsing.
 *
 * These structs represent the raw binary message formats for CME iLink 3.0.
 * Field order, types, and alignment must match the official CME specification.
 * Designed for direct memcpy from raw binary messages.
 *
 * Do not change field order, types, or alignment unless updating for a new protocol version.
 * For details, see CME iLink 3.0 Binary Order Entry Specification.
 */

#pragma once

#include <cstdint>

namespace beacon::cme::execution {

  // iLink 3 New Order Single Message
  struct alignas(32) CmeIlink3NewOrderSingle {
      uint64_t clientOrderId;
      char symbol[8];
      uint32_t quantity;
      uint32_t price;
      char side;         // '1'=buy, '2'=sell
      char orderType;    // '2'=limit, '1'=market
      char tif;          // time in force
      char reserved[3];  // pad to 32 bytes
      static_assert(sizeof(CmeIlink3NewOrderSingle) == 32, "Struct must be 32 bytes");
  };

  // iLink 3 Order Cancel Request Message
  struct alignas(32) CmeIlink3OrderCancelRequest {
      uint64_t clientOrderId;
      char symbol[8];
      uint32_t cancelQuantity;
      char reserved[12]; // pad to 32 bytes
      static_assert(sizeof(CmeIlink3OrderCancelRequest) == 32, "Struct must be 32 bytes");
  };

  // iLink 3 Order Modify Request Message
  struct alignas(32) CmeIlink3OrderModifyRequest {
      uint64_t originalOrderId;
      uint64_t newOrderId;
      char symbol[8];
      uint32_t quantity;
      uint32_t price;
      char reserved[4]; // pad to 32 bytes
      static_assert(sizeof(CmeIlink3OrderModifyRequest) == 32, "Struct must be 32 bytes");
  };

  // iLink 3 Execution Report Message
  struct alignas(32) CmeIlink3ExecutionReport {
      uint64_t clientOrderId;
      char symbol[8];
      uint32_t executedQuantity;
      uint32_t executionPrice;
      uint64_t execId;
      char execType;     // e.g., '0'=new, '4'=cancelled, '8'=rejected, 'F'=trade
      char reserved[3];  // pad to 32 bytes
      static_assert(sizeof(CmeIlink3ExecutionReport) == 32, "Struct must be 32 bytes");
  };
} // namespace beacon::cme::execution