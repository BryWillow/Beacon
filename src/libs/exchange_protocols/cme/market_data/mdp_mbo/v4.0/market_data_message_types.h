/*
 * Project: Beacon
 * File: market_data_message_types.h
 * CME MDP 4.0 Market By Order (MBO) message structs for direct binary parsing.
 *
 * These structs represent the raw binary message formats for CME MDP 4.0 MBO.
 * Field order, types, and alignment must match the official CME specification.
 * Designed for direct memcpy from raw binary messages.
 *
 * Do not change field order, types, or alignment unless updating for a new protocol version.
 * For details, see CME MDP 4.0 Market Data Binary Specification.
 */

#pragma once

#include <cstdint>

namespace beacon::cme::market_data {

struct alignas(32) CmeMboAddOrder {
    uint64_t sequenceNumber;
    uint32_t orderId;
    char symbol[8];
    uint32_t price;
    uint32_t quantity;
    char side;
    char orderType;
    char reserved[3];
};
//static_assert(sizeof(CmeMboAddOrder) == 32, "Struct must be 32 bytes");

struct alignas(32) CmeMboModifyOrder {
    uint64_t sequenceNumber;
    uint32_t orderId;
    char symbol[8];
    uint32_t price;
    uint32_t quantity;
    char side;
    char orderType;
    char reserved[3];
};
//static_assert(sizeof(CmeMboModifyOrder) == 32, "Struct must be 32 bytes");

struct alignas(32) CmeMboDeleteOrder {
    uint64_t sequenceNumber;
    uint32_t orderId;
    char symbol[8];
    char side;
    char reserved[11];
};
//static_assert(sizeof(CmeMboDeleteOrder) == 32, "Struct must be 32 bytes");

struct alignas(32) CmeMboTrade {
    uint64_t sequenceNumber;
    uint32_t orderId;
    char symbol[8];
    uint32_t price;
    uint32_t quantity;
    uint64_t tradeId;
    char side;
    char reserved[3];
};
//static_assert(sizeof(CmeMboTrade) == 32, "Struct must be 32 bytes");

struct alignas(32) CmeMboBookClear {
    uint64_t sequenceNumber;
    char symbol[8];
    char reserved[24];
};
//static_assert(sizeof(CmeMboBookClear) == 32, "Struct must be 32 bytes");

} // namespace beacon::cme::market_data