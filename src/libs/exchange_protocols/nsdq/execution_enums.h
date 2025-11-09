// ---------------------------------------------------------------------------
// @file        engine_enums.h
// Project     : Beacon
// Component   : Execution
// Description : Execution-related enums.
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

namespace beacon::exchanges::common {

  /**
   * @brief Side of an order
   */
  enum class OrderSide : int {
      Invalid, ///< Invalid / uninitialized
      Buy,     ///< Buy order
      Sell     ///< Sell order
  };

  /**
   * @brief Status of an order
   */
  enum class OrderStatus : int {
      Invalid,         ///< Invalid / uninitialized
      PendingSubmit,   ///< Order submitted, waiting for ack
      Open,            ///< Order accepted, partially or fully live
      PartiallyFilled, ///< Order partially executed
      Filled,          ///< Order fully executed
      PendingCancel,   ///< Cancel requested, waiting for ack
      Cancelled,       ///< Order cancelled
      PendingUpdate    ///< Update requested, waiting for ack
  };

  /**
   * @brief Result codes returned by the client API
   */
  enum class ClientResult : int
  {
      Invalid,             ///< Invalid / uninitialized
      Success,             ///< Operation succeeded
      InvalidRequest,      ///< Request was malformed or illegal
      ConnectionLost,      ///< Connection to matching engine lost
      CircuitBreakerActive ///< Circuit breaker halted trading
  };

} // namespace beacon::nsdq::common