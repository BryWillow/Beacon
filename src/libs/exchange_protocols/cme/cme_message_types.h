#pragma once

#include <cstdint>

// CME Order Entry Message
struct alignas(64) CmeOrderEntryMessage {
    uint64_t clientOrderId;      // 8 bytes: unique client order ID
    char symbol[8];              // 8 bytes: padded instrument symbol
    uint32_t quantity;           // 4 bytes: order quantity
    uint32_t price;              // 4 bytes: price in 1/10000 dollars
    char side;                   // 1 byte: 'B'=buy, 'S'=sell
    char orderType;              // 1 byte: e.g., 'L'=limit, 'M'=market
    char tif;                    // 1 byte: time in force
    char reserved1;              // 1 byte: reserved/future use
    uint16_t reserved2;          // 2 bytes: padding/future use
    char _padding[34];           // 64 - (8+8+4+4+1+1+1+1+2) = 34 bytes

    //static_assert(sizeof(CmeOrderEntryMessage) == 64, "CmeOrderEntryMessage must be 64 bytes");
};

// CME Cancel Order Message
struct alignas(64) CmeCancelOrderMessage {
    uint64_t clientOrderId;      // 8 bytes: unique client order ID
    uint32_t canceledQuantity;   // 4 bytes: quantity that was canceled
    uint16_t reserved;           // 2 bytes: reserved/future use
    char _padding[50];           // 64 - (8+4+2) = 50 bytes

    //static_assert(sizeof(CmeCancelOrderMessage) == 64, "CmeCancelOrderMessage must be 64 bytes");
};

// CME Replace Order Message
struct alignas(64) CmeReplaceOrderMessage {
    uint64_t originalOrderId;    // 8 bytes: order ID of the original order
    uint64_t newOrderId;         // 8 bytes: order ID of the new order
    uint32_t quantity;           // 4 bytes: order quantity
    uint32_t price;              // 4 bytes: price in 1/10000 dollars
    char side;                   // 1 byte: 'B'=buy, 'S'=sell
    char orderType;              // 1 byte: e.g., 'L'=limit, 'M'=market
    char tif;                    // 1 byte: time in force
    char reserved1;              // 1 byte: reserved/future use
    uint16_t reserved2;          // 2 bytes: padding/future use
    char _padding[34];           // 64 - (8+8+4+4+1+1+1+1+2) = 34 bytes

    //static_assert(sizeof(CmeReplaceOrderMessage) == 64, "CmeReplaceOrderMessage must be 64 bytes");
};
