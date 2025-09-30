// ---------------------------------------------------------------------------
// File        : matching_engine_enums.h
// Project     : Beacon
// Component   : Matching Engine
// Description : Enums for order side, order status, and client API result codes
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

/**
 * @defgroup MatchingEngineEnums Enums for Matching Engine
 * @{
 */

/** @brief Side of an order */
enum class _OrderSide : int {
    Invalid,   ///< Invalid / uninitialized
    Buy,       ///< Buy order
    Sell       ///< Sell order
};

/** @brief Status of an order */
enum class _OrderStatus : int {
    Invalid,         ///< Invalid / uninitialized
    PendingSubmit,   ///< Order submitted, waiting for ack
    Open,            ///< Order accepted, partially or fully live
    PartiallyFilled, ///< Order partially executed
    Filled,          ///< Order fully executed
    PendingCancel,   ///< Cancel requested, waiting for ack
    Cancelled,       ///< Order cancelled
    PendingUpdate    ///< Update requested, waiting for ack
};

/** @brief Result codes returned by the client API */
enum class _ClientResult : int {
    Invalid,               ///< Invalid / uninitialized
    Success,               ///< Operation succeeded
    InvalidRequest,        ///< Request was malformed or illegal
    ConnectionLost,        ///< Connection to matching engine lost
    CircuitBreakerActive   ///< Circuit breaker halted trading
};

/** @} */ // end of MatchingEngineEnums