// ---------------------------------------------------------------------------
// File        : messages_active.h
// Project     : Beacon
// Component   : NSDQ Market Data
// Description : Alias for the currently active ITCH protocol version
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstdint>

constexpr uint8_t ITCH_MSG_ADD_ORDER = 'A';
constexpr uint8_t ITCH_MSG_CANCEL_ORDER = 'X';
constexpr uint8_t ITCH_MSG_TRADE = 'P';

struct ITCHAddOrder {
    uint8_t messageType;
    uint16_t stockLocate;
    uint16_t trackingNumber;
    uint64_t timestamp;
    uint64_t orderReferenceNumber;
    char buySellIndicator;
    uint32_t shares;
    char stock[8];
    uint32_t price;
    // ...other fields as needed...
};

struct ITCHCancelOrder {
    uint8_t messageType;
    uint64_t orderReferenceNumber;
    uint32_t cancelledShares;
    // ...other fields as needed...
};

struct ITCHTrade {
    uint8_t messageType;
    uint64_t orderReferenceNumber;
    uint32_t shares;
    char stock[8];
    uint32_t price;
    // ...other fields as needed...
};