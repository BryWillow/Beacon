// ---------------------------------------------------------------------------
// File        : itch_message.h
// Project     : Beacon
// Component   : Common
// Description : TotalView ITCH message structure with serialization
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <cstring>      // strnlen
#include <fstream>      // for ifstream/ofstream
#include <arpa/inet.h>  // ntohl, htonl

#if defined(__x86_64__)
#include <immintrin.h>  // SSE/AVX intrinsics if needed
#endif

#include "hton_utils.h" // htonll / ntohll helpers

/**
 * @brief Buy/Sell side of an order
 */
enum class Side : uint8_t {
    Buy     = 0,
    Sell    = 1,
    Unknown = 255 ///< distinct invalid value for testing
};

/**
 * @brief ITCH message type codes
 */
enum class ItchMsgType : uint8_t {
    AddOrder      = 'A',
    AddOrderMP    = 'F',
    OrderExecuted = 'E',
    OrderCancel   = 'X',
    Trade         = 'P',
    Unknown       = 0
};

/**
 * @brief Single ITCH message
 *
 * Notes:
 * - Fixed-size struct for cache-line friendliness (64 bytes)
 * - No dynamic memory allocation
 * - Aligned to avoid false sharing between threads
 * - Supports network-to-host and host-to-network conversions
 * - Can serialize/deserialize to/from binary streams
 */
struct alignas(64) ItchMessage {
    ItchMsgType type           { ItchMsgType::Unknown }; ///< Order message type
    uint32_t    orderId        { 0 };                    ///< Order ID
    char        symbol[8]      { 0 };                    ///< Symbol (padded)
    uint32_t    size           { 0 };                    ///< Quantity
    double      price          { 0.0 };                  ///< Price
    Side        side           { Side::Unknown };        ///< Buy/Sell
    uint64_t    tsNanoSeconds  { 0 };                    ///< Timestamp in nanoseconds
    uint64_t    sequenceNumber { 0 };                    ///< Order sequence number

    // -----------------------------------------------------------------------
    // Helper functions
    // -----------------------------------------------------------------------

    /// @brief Returns symbol as std::string
    std::string symbolStr() const { return std::string(symbol, strnlen(symbol, 8)); }

    /// @brief Convenience checkers
    bool isAddOrder()  const { return type == ItchMsgType::AddOrder || type == ItchMsgType::AddOrderMP; }
    bool isExecuted()  const { return type == ItchMsgType::OrderExecuted; }
    bool isCanceled()  const { return type == ItchMsgType::OrderCancel; }
    bool isTrade()     const { return type == ItchMsgType::Trade; }

    // -----------------------------------------------------------------------
    // Byte order conversion
    // -----------------------------------------------------------------------

    /// @brief Converts network byte order (big-endian) to host byte order (little-endian)
    void toHostOrder() {
        orderId        = ntohl(orderId);
        size           = ntohl(size);
        sequenceNumber = ntohll(sequenceNumber);
        tsNanoSeconds  = ntohll(tsNanoSeconds);
        // price and char arrays do not require conversion
    }

    /// @brief Converts host byte order (little-endian) to network byte order (big-endian)
    void toNetworkOrder() {
        orderId        = htonl(orderId);
        size           = htonl(size);
        sequenceNumber = htonll(sequenceNumber);
        tsNanoSeconds  = htonll(tsNanoSeconds);
        // price and char arrays do not require conversion
    }

    // -----------------------------------------------------------------------
    // Serialization / Deserialization
    // -----------------------------------------------------------------------

    /**
     * @brief Serialize message to binary output stream
     * @param ofs Output file stream
     */
    void serialize(std::ofstream& ofs) const {
        ofs.write(reinterpret_cast<const char*>(this), sizeof(ItchMessage));
    }

    /**
     * @brief Deserialize message from binary input stream
     * @param ifs Input file stream
     * @return true if read succeeded, false if EOF or partial read
     */
    bool deserialize(std::ifstream& ifs) {
        if (!ifs.read(reinterpret_cast<char*>(this), sizeof(ItchMessage))) {
            return false; // failed to read full message
        }
        return true;
    }
};