// ---------------------------------------------------------------------------
// @file        itch_parser.h
// Project     : Beacon
// Component   : NSDQ ITCH 5.0
// Description : Parses raw ITCH packets into typed messages and pushes them to a ring buffer
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "itch_messages.h"
#include "spsc_ringbuffer.h"

namespace spsc = beacon::hft::ringbuffer;

namespace beacon::nsdq::market_data::itch {

/**
 * @class ItchParser
 * @brief Converts raw ITCH packets into message variants and pushes them to a ring buffer.
 *
 * Usage:
 *   SpScRingBuffer<ItchMessageVariant> buffer;
 *   ItchParser parser(buffer);
 *   parser.parsePacket(data, length);
 */
class ItchParser {
   public:
    explicit ItchParser(spsc::SpScRingBuffer<ItchMessageVariant>& buffer) : _ringBuffer(buffer) {}

    /**
     * @brief Parse a single packet of ITCH messages
     * @param data Raw packet bytes
     * @param len Packet length
     *
     * Throws std::runtime_error if packet is truncated or contains an unknown message type.
     */
    void parsePacket(const char* data, size_t len) {
        size_t offset = 0;

        auto ensureBytes = [&](size_t required) {
            if (offset + required > len) {
                throw std::runtime_error("Packet truncated");
            }
        };

        while (offset < len) {
            MessageType type = static_cast<MessageType>(data[offset]);

            switch (type) {
                case MessageType::AddOrder: {
                    ensureBytes(sizeof(AddOrderMessage));
                    AddOrderMessage msg;
                    std::memcpy(&msg, data + offset, sizeof(msg));
                    _ringBuffer.tryPush(msg);
                    offset += sizeof(msg);
                    break;
                }
                case MessageType::Trade: {
                    ensureBytes(sizeof(TradeMessage));
                    TradeMessage msg;
                    std::memcpy(&msg, data + offset, sizeof(msg));
                    _ringBuffer.tryPush(msg);
                    offset += sizeof(msg);
                    break;
                }
                case MessageType::OrderExecuted: {
                    ensureBytes(sizeof(OrderExecutedMessage));
                    OrderExecutedMessage msg;
                    std::memcpy(&msg, data + offset, sizeof(msg));
                    _ringBuffer.tryPush(msg);
                    offset += sizeof(msg);
                    break;
                }
                case MessageType::OrderCancel: {
                    ensureBytes(sizeof(OrderCancelMessage));
                    OrderCancelMessage msg;
                    std::memcpy(&msg, data + offset, sizeof(msg));
                    _ringBuffer.tryPush(msg);
                    offset += sizeof(msg);
                    break;
                }
                case MessageType::OrderDelete: {
                    ensureBytes(sizeof(OrderDeleteMessage));
                    OrderDeleteMessage msg;
                    std::memcpy(&msg, data + offset, sizeof(msg));
                    _ringBuffer.tryPush(msg);
                    offset += sizeof(msg);
                    break;
                }
                case MessageType::ReplaceOrder: {
                    ensureBytes(sizeof(ReplaceOrderMessage));
                    ReplaceOrderMessage msg;
                    std::memcpy(&msg, data + offset, sizeof(msg));
                    _ringBuffer.tryPush(msg);
                    offset += sizeof(msg);
                    break;
                }
                case MessageType::MarketDepth: {
                    ensureBytes(sizeof(MarketDepthMessage));
                    MarketDepthMessage msg;
                    std::memcpy(&msg, data + offset, sizeof(msg));
                    _ringBuffer.tryPush(msg);
                    offset += sizeof(msg);
                    break;
                }
                default:
                    throw std::runtime_error("Unknown ITCH message type");
            }
        }
    }

   private:
    spsc::SpScRingBuffer<ItchMessageVariant>& _ringBuffer;  ///< Ring buffer for parsed messages
};

}  // namespace beacon::nsdq::market_data::itch