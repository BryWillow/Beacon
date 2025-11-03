// ---------------------------------------------------------------------------
// @file        itch_message_types.h
// Project     : Beacon
// Component   : NSDQ ITCH 5.0
// Description : ITCH 5.0 message definitions and variant wrapper
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <variant>

namespace beacon::nsdq::market_data::itch {

/**
 * @enum MessageType
 * @brief ITCH 5.0 message type codes
 */
enum class MessageType : uint8_t {
  AddOrder =      'A',  ///< Add order message
  Trade =         'P',  ///< Trade message
  OrderExecuted = 'E',  ///< Order executed
  OrderCancel =   'X',  ///< Cancel an order
  OrderDelete =   'D', ///< Delete an order
  ReplaceOrder =  'U', ///< Replace an order
  MarketDepth =   'R'  ///< Market depth update
};

/**
 * @struct AddOrderMessage
 * @brief ITCH add order message
 */
struct AddOrderMessage {
  uint64_t sequenceNumber;   ///< ITCH global sequence number
  uint64_t orderRefNum;      ///< user-assigned reference number
  char stock[8];             ///< padded stock symbol
  uint32_t shares;           ///< number of shares
  uint32_t price;            ///< price in 1/10000 dollars
  char side;                 ///< 'B'=buy, 'S'=sell
};

/**
 * @struct TradeMessage
 * @brief ITCH trade message
 */
struct TradeMessage {
  uint64_t sequenceNumber;   ///< ITCH global sequence number
  uint64_t orderRefNum;      ///< user-assigned reference number
  char side;                 ///< 'B'=buy, 'S'=sell
  uint32_t shares;           ///< number of shares
  char stock[8];             ///< padded stock symbol
  uint32_t price;            ///< price in 1/10000 dollars
};

/**
 * @struct OrderExecutedMessage
 * @brief ITCH order executed message
 */
struct OrderExecutedMessage {
  uint64_t sequenceNumber;   ///< ITCH global sequence number
  uint32_t orderRefNum;      ///< user-assigned reference number
  uint32_t executedShares;   ///< shares executed
  uint64_t matchNumber;      ///< exchange match number
};

/**
 * @struct OrderCancelMessage
 * @brief ITCH order cancel message
 */
struct OrderCancelMessage {
  uint64_t sequenceNumber;   ///< ITCH global sequence number
  uint32_t orderRefNum;      ///< user-assigned reference number
  uint32_t canceledShares;   ///< shares canceled
};

/**
 * @struct OrderDeleteMessage
 * @brief ITCH order delete message
 */
struct OrderDeleteMessage {
  uint64_t sequenceNumber;   ///< ITCH global sequence number
  uint32_t orderRefNum;      ///< user-assigned reference number
};

/**
 * @struct ReplaceOrderMessage
 * @brief ITCH replace order message
 */
struct ReplaceOrderMessage {
  uint64_t sequenceNumber;   ///< ITCH global sequence number
  uint64_t sourceSequenceNumber; ///< Reference to original AddOrder sequence number
  uint32_t originalOrderRefNum;  ///< original order reference
  uint32_t newOrderRefNum;       ///< new order reference
  uint32_t shares;               ///< new share count
  uint32_t price;                ///< new price
};

/**
 * @struct MarketDepthMessage
 * @brief ITCH market depth update
 */
struct MarketDepthMessage {
  uint64_t sequenceNumber;   ///< ITCH global sequence number
  char stock[8];      ///< padded stock symbol
  char updateAction;  ///< 'A'=add, 'D'=delete, 'M'=modify
  char side;          ///< 'B'=bid, 'S'=ask
  uint32_t price;     ///< price level
  uint32_t shares;    ///< shares at this level
  uint32_t position;  ///< position in book
};

/**
 * @brief Variant type representing any ITCH message
 */
using ItchMessageVariant =
  std::variant<
    AddOrderMessage,
    TradeMessage,
    OrderExecutedMessage,
    OrderCancelMessage,
    OrderDeleteMessage,
    ReplaceOrderMessage,
    MarketDepthMessage>;

  enum class ItchMessageKind : uint8_t {
    AddOrder,
    Trade,
    Cancel,
    Delete,
    Replace,
    MarketDepth
  };
}  // namespace beacon::nsdq::market_data::itch