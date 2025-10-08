// ---------------------------------------------------------------------------
// @file        itch_parser.h
// Project     : Beacon
// Component   : NSDQ ITCH 5.0
// Description : Parses raw ITCH packets into typed messages and pushes them to a ring rbItchMessages
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "itch_messages.h"
#include "spsc_ringbuffer.h"

namespace beaconrb = beacon::hft::ringbuffer;

namespace beacon::nsdq::market_data::itch {

  /**
   * @class ItchFeedHandler
   * @brief Converts raw ITCH packets into message variants and pushes them to a ring rbItchMessages.
   * @note  ItchMessgaeVariant contains the types of messages supported by this RingBuffer.
   *
   * Usage:
   *   SpScRingBuffer<ItchMessageVariant> buffer;
   *  ItchFeedHandler parser(buffer);
   *. parser.parsePacket(rawItchBytes, length);
  */
  class ItchFeedHandler {

    public:
      explicit ItchFeedHandler(beaconrb::SpScRingBuffer<ItchMessageVariant>& rbItchMessages)
        : _rbItchMessages(rbItchMessages) {}

      // Non-copyable
      ItchFeedHandler(const ItchFeedHandler&) = delete;
      ItchFeedHandler &operator=(const ItchFeedHandler&) = delete;

      // Non-movable
      ItchFeedHandler(ItchFeedHandler &&) = delete;
      ItchFeedHandler& operator=(ItchFeedHandler &&) = delete;

      /**
        * @brief Decodes a raw, binary ITCH message into one of many possible message types.
        *  Inserts the parsed message onto the single-producer single-consumer queue
        *  provided in the ctor. This is the "consumer" for market data.
        *
        *  @param rawItchBytes  An ITCH message in binary format.
        *. @param numberOfBytes The number of bytess in the binary ITCH message.
        *
        *. @note  populates the provided ring buffer (ctor) as new packets arrive
        *.
        *  @throws std::runtime_error if packet is malformed or an unknown type.
        */
      void parsePacket(const char* rawItchBytes, size_t numberOfBytes) {

          // There is generally one message in a set of bytes,somes some messages
          // can contain multiple messages. Set the offset to the end of the first message.
        size_t offset = 0;
        auto ensureBytes = [&](size_t required) {
          if (offset + required > numberOfBytes) {
            throw std::runtime_error("Packet truncated");
          }
        };

        // Per the ITCH spec, a UDP packet may contain multiple ITCH messages.
        // The messages are packed into a byte array, with no delimeter.
        // So, essentially what we're doing here is:
        // a. Reading the first character to determine the message type
        // b. Based on (a) reading and converting those bytes into the appropriate Beacon struct
        // c. Advancing the offset to read the next message in the packet until we've reached the end
        // d. Enqueueing each message onto the lock-free ring buffer for down-stream processing
        // e. Note that we're currently dropping packets if the ring buffer is full (tryPush returns false) -- need to alert/monitor
        // f. protecting ourselves from malformed packets via the ensureBytes lambda above
        while (offset < numberOfBytes) {
          MessageType type = static_cast<MessageType>(rawItchBytes[offset]);
          switch (type) {
            case MessageType::AddOrder: {
              ensureBytes(sizeof(AddOrderMessage));
              AddOrderMessage msg;
              std::memcpy(&msg, rawItchBytes + offset, sizeof(msg));
              _rbItchMessages.tryPush(msg);
              offset += sizeof(msg);
              break;
            }
            case MessageType::Trade: {
              ensureBytes(sizeof(TradeMessage));
              TradeMessage msg;
              std::memcpy(&msg, rawItchBytes + offset, sizeof(msg));
              _rbItchMessages.tryPush(msg);
              offset += sizeof(msg);
              break;
            }
            case MessageType::OrderExecuted: {
              ensureBytes(sizeof(OrderExecutedMessage));
              OrderExecutedMessage msg;
              std::memcpy(&msg, rawItchBytes + offset, sizeof(msg));
              _rbItchMessages.tryPush(msg);
              offset += sizeof(msg);
              break;
            }
            case MessageType::OrderCancel: {
              ensureBytes(sizeof(OrderCancelMessage));
              OrderCancelMessage msg;
              std::memcpy(&msg, rawItchBytes + offset, sizeof(msg));
              _rbItchMessages.tryPush(msg);
              offset += sizeof(msg);
              break;
            }
            case MessageType::OrderDelete: {
              ensureBytes(sizeof(OrderDeleteMessage));
              OrderDeleteMessage msg;
              std::memcpy(&msg, rawItchBytes + offset, sizeof(msg));
              _rbItchMessages.tryPush(msg);
              offset += sizeof(msg);
              break;
            }
            case MessageType::ReplaceOrder: {
              ensureBytes(sizeof(ReplaceOrderMessage));
              ReplaceOrderMessage msg;
              std::memcpy(&msg, rawItchBytes + offset, sizeof(msg));
              _rbItchMessages.tryPush(msg);
              offset += sizeof(msg);
              break;
            }
            case MessageType::MarketDepth: {
              ensureBytes(sizeof(MarketDepthMessage));
                MarketDepthMessage msg;
                std::memcpy(&msg, rawItchBytes + offset, sizeof(msg));
                _rbItchMessages.tryPush(msg);
                offset += sizeof(msg);
                break;
              }
              default:
              throw std::runtime_error("Unknown ITCH message type");
            } // end switch
        } // end while
      }

      private:
        beaconrb::SpScRingBuffer<ItchMessageVariant>& _rbItchMessages;
    };
  }