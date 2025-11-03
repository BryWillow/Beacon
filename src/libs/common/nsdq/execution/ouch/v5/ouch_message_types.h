#pragma once

#include <cstdint>

// OUCH 5.0 Order Entry Message
struct alignas(64) OuchEnterOrderMessage {
    uint64_t clientOrderId;      // 8 bytes: unique client order ID
    char symbol[8];              // 8 bytes: padded stock symbol
    uint32_t shares;             // 4 bytes: number of shares
    uint32_t price;              // 4 bytes: price in 1/10000 dollars
    char side;                   // 1 byte: 'B'=buy, 'S'=sell
    char timeInForce;            // 1 byte: time in force
    char orderType;              // 1 byte: e.g., 'O'=order, 'C'=cancel, etc.
    char capacity;               // 1 byte: e.g., 'A'=agency, 'P'=principal
    uint16_t reserved;           // 2 bytes: padding/future use
    char _padding[34];           // 64 - (8+8+4+4+1+1+1+1+2) = 34 bytes

    static_assert(sizeof(OuchEnterOrderMessage) == 64, "OuchEnterOrderMessage must be 64 bytes");
};

// OUCH 5.0 Cancel Order Message
struct alignas(64) OuchCancelOrderMessage {
    uint64_t clientOrderId;      // 8 bytes: unique client order ID
    uint32_t canceledShares;     // 4 bytes: shares to cancel
    uint16_t reserved;           // 2 bytes: padding/future use
    char _padding[50];           // 64 - (8+4+2) = 50 bytes

    static_assert(sizeof(OuchCancelOrderMessage) == 64, "OuchCancelOrderMessage must be 64 bytes");
};

// OUCH 5.0 Replace Order Message
struct alignas(64) OuchReplaceOrderMessage {
    uint64_t originalOrderId;    // 8 bytes: original client order ID
    uint64_t newOrderId;         // 8 bytes: new client order ID
    uint32_t shares;             // 4 bytes: new share count
    uint32_t price;              // 4 bytes: new price
    char side;                   // 1 byte: 'B'=buy, 'S'=sell
    char timeInForce;            // 1 byte
    char orderType;              // 1 byte
    char capacity;               // 1 byte
    uint16_t reserved;           // 2 bytes: padding/future use
    char _padding[34];           // 64 - (8+8+4+4+1+1+1+1+2) = 34 bytes

    static_assert(sizeof(OuchReplaceOrderMessage) == 64, "OuchReplaceOrderMessage must be 64 bytes");
};
