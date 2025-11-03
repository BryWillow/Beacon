#pragma once

#include <cstdint>
#include <cstring>
#include <string>

enum class BeaconErrorCode : int {
  Success = 0,
  InvalidArgument,
  FileNotFound,
  // ...other codes...
};

struct SymbolParameters {
  std::string symbol;
  // ...other fields...
};

enum class MarketDataSource : uint8_t {
    ITCH,
    CME,
    NYSE,
    UNKNOWN
};

struct alignas(64) MarketDataTick {
    uint64_t sequenceNumber;      // 8 bytes: ITCH global sequence number
    uint64_t orderRefNum;         // 8 bytes: user-assigned reference number
    char symbol[8];               // 8 bytes: padded stock symbol
    uint32_t shares;              // 4 bytes: number of shares
    uint32_t price;               // 4 bytes: price in 1/10000 dollars
    char side;                    // 1 byte: 'B'=buy, 'S'=sell
    char type;                    // 1 byte: e.g., 'A'=Add, 'X'=Cancel, etc.
    uint16_t reserved;            // 2 bytes: padding/future use

    char _padding[28];            // 64 - (8+8+8+4+4+1+1+2) = 28 bytes

    static_assert(sizeof(MarketDataTick) == 64, "MarketDataTick must be 64 bytes for cache alignment");
};

// ...other shared types...
