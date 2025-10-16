// ---------------------------------------------------------------------------
// @file        nsdq_common_enums.h
// Project     : Beacon
// Component   : NSDQ Common
// Description : Shared enums and type definitions for ITCH & OUCH messages
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace beacon::nsdq::common
{
    /**
     * @enum Side
     * @brief Buy or Sell side indicator for orders
     */
    enum class Side : uint8_t {
      Buy =  'B', ///< Buy side
      Sell = 'S'  ///< Sell side
    };

    /**
     * @enum UpdateType
     * @brief Market depth update types
     */
    enum class UpdateType : uint8_t{
      Add =    'A', ///< Add a new order / level
      Delete = 'D', ///< Remove an order / level
      Modify = 'M'  ///< Modify an existing order / level
    };

    /**
     * @enum MessageType
     * @brief Generic message type codes for ITCH / OUCH
     */
    enum class MessageType : uint8_t {
      AddOrder =      'A', ///< Add Order
      Trade =         'P', ///< Trade (ITCH: P or Q)
      OrderExecuted = 'E', ///< Order Executed
      OrderCancel =   'X', ///< Cancel an order
      OrderDelete =   'D', ///< Delete an order
      ReplaceOrder =  'U', ///< Replace an order
      MarketDepth =   'R'  ///< Market depth update
    };

    /**
     * @brief Common type for order reference numbers
     * User-assigned or exchange-assigned identifier
     */
    using OrderRefNum = uint64_t;

    /**
     * @brief Common type for timestamps in nanoseconds
     */
    using TimestampNs = uint64_t;

} // namespace beacon::nsdq::common
