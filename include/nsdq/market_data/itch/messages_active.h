#ifndef BEACON_NSDQ_MARKET_DATA_ITCH_MESSAGES_ACTIVE_H
#define BEACON_NSDQ_MARKET_DATA_ITCH_MESSAGES_ACTIVE_H

#pragma once

// Redirect header: include the real ITCH messages_active.h from one of the known locations.
// Adjust or extend the candidate paths if your environment uses a different layout.

#if defined(__has_include)
  // Prefer a vendored copy inside the repo (common patterns)
  #if __has_include("vendor/nsdq/market_data/itch/messages_active.h")
    #include "vendor/nsdq/market_data/itch/messages_active.h"
  #elif __has_include(<nsdq/market_data/itch/messages_active.h>)
    #include <nsdq/market_data/itch/messages_active.h>
  #else
    #error "messages_active.h not found. Place vendor copy or install system header."
  #endif
#else
  #error "__has_include not supported; provide messages_active.h"
#endif

#endif // BEACON_NSDQ_MARKET_DATA_ITCH_MESSAGES_ACTIVE_H
