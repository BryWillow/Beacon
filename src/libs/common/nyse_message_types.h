#pragma once

#include <cstdint>

// NYSE Order Entry Message
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

// NYSE Cancel Order Message
struct alignas(64) NyseCancelOrderMessage {
    uint64_t clientOrderId;
    uint32_t canceledShares;
    uint16_t reserved;
    char _padding[50];

    static_assert(sizeof(NyseCancelOrderMessage) == 64, "Struct must be 64 bytes");
};

// NYSE Replace Order Message
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
