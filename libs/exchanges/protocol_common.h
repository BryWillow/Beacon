#pragma once

#include <cstdint>

// Safest representation for exchange prices
struct ExchangeOrder {
  int64_t price; // price in smallest unit (e.g., cents, ticks, basis points)
  int32_t quantity;
  // ...existing code...
};