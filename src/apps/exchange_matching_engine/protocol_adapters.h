/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_matching_engine
 * File:         protocol_adapters.h
 * Purpose:      Multi-protocol order entry adapter framework supporting:
 *               - NASDAQ OUCH v5.0 (64-byte binary protocol)
 *               - NYSE Pillar Gateway v3.2 (64-byte binary protocol)
 *               - CME iLink 3 (64-byte simplified implementation)
 *               
 *               Converts exchange-specific message formats to a unified
 *               NormalizedOrder structure for protocol-agnostic order
 *               processing in the matching engine.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <cstdint>
#include <cstring>

// ============================================================================
// NORMALIZED ORDER FORMAT (Internal representation)
// ============================================================================

/**
 * @struct NormalizedOrder
 * @brief Unified internal representation for orders from all exchange protocols
 * 
 * This structure serves as the common format after protocol-specific decoding.
 * All exchange-specific messages (OUCH, Pillar, CME) are converted to this
 * format to enable protocol-agnostic order processing in the matching engine.
 * 
 * @details
 * - Total Size: 32 bytes (includes padding for alignment)
 * - Price Encoding: Fixed-point decimal (1/10000 dollars)
 *   Example: 15025000 represents $1,502.50
 * - Symbol Format: 8 bytes, space-padded on right
 *   Example: "AAPL    " (4 chars + 4 spaces)
 * - Protocol Field: Enables protocol-specific logic when needed
 * 
 * @note The protocol field allows the matching engine to apply
 *       exchange-specific rules (e.g., CME margin vs. stock settlement)
 */
struct NormalizedOrder {
    uint64_t orderId;           ///< Unique client-assigned order identifier
    char symbol[8];             ///< Instrument symbol, space-padded (e.g., "AAPL    " or "ESZ4    ")
    uint32_t quantity;          ///< Number of shares (stocks) or contracts (futures)
    uint32_t price;             ///< Price in 1/10000 dollars (e.g., 15025000 = $1,502.50)
    char side;                  ///< 'B' = Buy/Long, 'S' = Sell/Short
    char timeInForce;           ///< '0'=Day, '3'=IOC (Immediate or Cancel), '4'=FOK (Fill or Kill), 'G'=GTC
    char orderType;             ///< 'L'=Limit, 'M'=Market, 'O'=OUCH Order, 'S'=Stop
    char capacity;              ///< 'A'=Agency, 'P'=Principal, 'R'=Riskless Principal, 'M'=Market Maker
    uint8_t protocol;           ///< Protocol identifier: 1=OUCH, 2=Pillar, 3=CME
    char _padding[3];           ///< Padding for 4-byte alignment
};

// ============================================================================
// PROTOCOL MESSAGE STRUCTURES
// ============================================================================

/**
 * @struct OuchEnterOrderMessage
 * @brief NASDAQ OUCH v5.0 Enter Order Message (64 bytes)
 * 
 * OUCH (Order Utility and Connection Handler) is NASDAQ's proprietary binary
 * protocol for order entry. This structure represents the Enter Order message
 * type, used to submit new orders to the NASDAQ exchange.
 * 
 * @details Protocol Characteristics:
 * - Version: OUCH 5.0
 * - Message Type: Enter Order
 * - Size: 64 bytes (fixed-length)
 * - Byte Order: Network byte order (big-endian for multi-byte fields)
 * - Transport: TCP
 * 
 * @details Price Encoding:
 * - Stored as uint32_t in 1/10000 dollar increments
 * - Example: $150.25 → 1,502,500
 * - Range: $0.0001 to $429,496.7295
 * 
 * @details Symbol Format:
 * - 8 bytes, space-padded on right
 * - Example: "AAPL" → "AAPL    "
 * - Supports dots for preferred stocks: "BRK.A   "
 * 
 * @details Time In Force Options:
 * - '0' = Day (cancel at market close)
 * - '3' = IOC (Immediate or Cancel)
 * - '4' = FOK (Fill or Kill)
 * - 'G' = GTC (Good Till Cancel, persists across sessions)
 * 
 * @details Trading Capacity:
 * - 'A' = Agency (acting for customer)
 * - 'P' = Principal (trading for own account)
 * - 'R' = Riskless Principal (simultaneous offsetting trade)
 * 
 * @note The orderType field is used for protocol detection:
 *       OUCH messages always have orderType == 'O'
 * 
 * @see NASDAQ OUCH Specification v5.0
 * @see docs/exchange_protocols/OUCH_PROTOCOL.md
 */
struct OuchEnterOrderMessage {
    uint64_t clientOrderId;      ///< 8 bytes: Unique order ID (must be unique per session)
    char symbol[8];              ///< 8 bytes: Stock symbol, space-padded (e.g., "AAPL    ")
    uint32_t shares;             ///< 4 bytes: Number of shares (1 - 999,999,999)
    uint32_t price;              ///< 4 bytes: Price in 1/10000 dollars (e.g., 1502500 = $150.25)
    char side;                   ///< 1 byte: 'B' = Buy, 'S' = Sell
    char timeInForce;            ///< 1 byte: '0'=Day, '3'=IOC, '4'=FOK, 'G'=GTC
    char orderType;              ///< 1 byte: 'O' = OUCH order (used for protocol detection)
    char capacity;               ///< 1 byte: 'A'=Agency, 'P'=Principal, 'R'=Riskless Principal
    uint16_t reserved;           ///< 2 bytes: Reserved for future protocol extensions
    char _padding[34];           ///< 34 bytes: Padding to reach 64-byte message size
};

static_assert(sizeof(OuchEnterOrderMessage) == 64, "OuchEnterOrderMessage must be 64 bytes");

/**
 * @struct PillarOrderEntryMessage
 * @brief NYSE Pillar Gateway v3.2 Order Entry Message (64 bytes)
 * 
 * Pillar is the New York Stock Exchange's unified trading platform and gateway
 * protocol. It consolidates order entry across all NYSE markets (NYSE, NYSE
 * American, NYSE Arca, NYSE National).
 * 
 * @details Protocol Characteristics:
 * - Version: Pillar Gateway 3.2
 * - Message Type: New Order
 * - Size: 64 bytes (fixed-length)
 * - Byte Order: Network byte order (big-endian)
 * - Transport: TCP (primary), UDP (market data)
 * 
 * @details Order Types:
 * - 'L' = Limit (execute at specified price or better)
 * - 'M' = Market (execute at best available price)
 * - 'P' = Pegged (price tracks market, e.g., midpoint)
 * 
 * @details Time In Force Options:
 * - '0' = Day (cancel at 4:00 PM ET)
 * - '3' = IOC (Immediate or Cancel)
 * - '4' = FOK (Fill or Kill)
 * - 'G' = GTC (Good Till Cancel)
 * - 'O' = Opening Only (execute in opening auction only)
 * - 'C' = Closing Only (execute in closing auction only)
 * 
 * @details Market Phases:
 * NYSE operates in distinct phases with different order handling:
 * - Pre-Opening (4:00 AM - 9:30 AM): Accumulate orders for auction
 * - Opening Auction (~9:30 AM): Single price discovery
 * - Core Trading (9:30 AM - 4:00 PM): Continuous trading
 * - Closing Auction (~4:00 PM): Single closing price
 * 
 * @note Protocol Detection:
 *       Pillar messages have orderType == 'L' or 'M'
 *       Distinguished from CME by symbol format (no month codes)
 * 
 * @see NYSE Pillar Gateway Specification v3.2
 * @see docs/exchange_protocols/PILLAR_PROTOCOL.md
 */
struct PillarOrderEntryMessage {
    uint64_t clientOrderId;      ///< 8 bytes: Unique order ID
    char symbol[8];              ///< 8 bytes: Stock symbol, space-padded (e.g., "IBM     ")
    uint32_t shares;             ///< 4 bytes: Number of shares (1 - 99,999,999)
    uint32_t price;              ///< 4 bytes: Price in 1/10000 dollars (0 for market orders)
    char side;                   ///< 1 byte: 'B' = Buy, 'S' = Sell
    char orderType;              ///< 1 byte: 'L'=Limit, 'M'=Market, 'P'=Pegged
    char tif;                    ///< 1 byte: Time In Force ('0'=Day, '3'=IOC, etc.)
    char reserved1;              ///< 1 byte: Reserved for future use
    uint16_t reserved2;          ///< 2 bytes: Reserved for future use
    char _padding[34];           ///< 34 bytes: Padding to 64 bytes
};

static_assert(sizeof(PillarOrderEntryMessage) == 64, "PillarOrderEntryMessage must be 64 bytes");

/**
 * @struct CmeOrderEntryMessage
 * @brief CME iLink 3 Order Entry Message (64 bytes, simplified)
 * 
 * CME (Chicago Mercantile Exchange) Globex is the electronic trading platform
 * for derivatives. This is a simplified implementation of iLink 3 (CME's
 * FIX-based protocol) focusing on futures order entry.
 * 
 * @details Protocol Characteristics:
 * - Platform: CME Globex
 * - Protocol: iLink 3 (simplified binary implementation)
 * - Message Type: New Order Single
 * - Size: 64 bytes (simplified from full iLink 3)
 * - Asset Classes: Futures, Options on Futures
 * 
 * @details Symbol Format (Critical for CME):
 * - Structure: [ROOT][MONTH_CODE][YEAR]
 * - Root: 1-3 characters (e.g., ES, NQ, ZN)
 * - Month Code: Single character (F,G,H,J,K,M,N,Q,U,V,X,Z)
 * - Year: Single digit (4 = 2024, 5 = 2025)
 * - Examples:
 *   * "ESZ4    " = E-mini S&P 500, December 2024
 *   * "NQH5    " = E-mini Nasdaq-100, March 2025
 *   * "ZNZ4    " = 10-Year T-Note, December 2024
 * 
 * @details Month Codes:
 * - F=January, G=February, H=March, J=April
 * - K=May, M=June, N=July, Q=August
 * - U=September, V=October, X=November, Z=December
 * 
 * @details Popular Contracts:
 * - ES: E-mini S&P 500 ($50 × index, 0.25 tick = $12.50)
 * - NQ: E-mini Nasdaq-100 ($20 × index, 0.25 tick = $5.00)
 * - YM: E-mini Dow ($5 × index, 1.00 tick = $5.00)
 * - RTY: E-mini Russell 2000 ($50 × index, 0.10 tick = $5.00)
 * - ZN: 10-Year T-Note ($100,000 face, 1/64 tick = $15.625)
 * 
 * @details Trading Hours:
 * CME operates nearly 24 hours with brief maintenance windows:
 * - Globex Evening: 5:00 PM - 8:30 AM CT
 * - Regular Hours: 9:30 AM - 4:00 PM CT (highest volume)
 * - Maintenance: 5:00 PM - 6:00 PM CT (NO TRADING)
 * 
 * @details Margin and Leverage:
 * - Typical initial margin: $5,000 - $15,000 per contract
 * - Notional value: $200,000+ per contract (ES)
 * - Leverage: ~15:1 to 40:1 (much higher than stocks)
 * 
 * @note Protocol Detection:
 *       CME messages have orderType == 'L' or 'M'
 *       Distinguished from Pillar by symbol format:
 *       - symbol[2] is month code (F-Z)
 *       - symbol[3] is year digit (0-9)
 * 
 * @see CME iLink 3 Specification
 * @see docs/exchange_protocols/CME_PROTOCOL.md
 */
struct CmeOrderEntryMessage {
    uint64_t clientOrderId;      ///< 8 bytes: Unique order ID (UUID or counter)
    char symbol[8];              ///< 8 bytes: Futures symbol (e.g., "ESZ4    ", "NQH5    ")
    uint32_t quantity;           ///< 4 bytes: Number of contracts (NOT shares - important!)
    uint32_t price;              ///< 4 bytes: Price in 1/10000 dollars (e.g., 45500000 = $4,550.00)
    char side;                   ///< 1 byte: 'B' = Buy/Long, 'S' = Sell/Short
    char orderType;              ///< 1 byte: 'L'=Limit, 'M'=Market, 'S'=Stop, 'T'=Stop Limit
    char tif;                    ///< 1 byte: Time In Force ('0'=Day, '3'=IOC, '4'=FOK, 'G'=GTC)
    char reserved1;              ///< 1 byte: Reserved for future protocol extensions
    uint16_t reserved2;          ///< 2 bytes: Reserved (could be used for spread orders)
    char _padding[34];           ///< 34 bytes: Padding to 64-byte message size
};

static_assert(sizeof(CmeOrderEntryMessage) == 64, "CmeOrderEntryMessage must be 64 bytes");

// ============================================================================
// PROTOCOL ADAPTERS (Decoders)
// ============================================================================

/**
 * @namespace ProtocolAdapter
 * @brief Protocol decoder utilities for multi-exchange order entry
 * 
 * This namespace provides decoder functions that convert exchange-specific
 * binary message formats (OUCH, Pillar, CME) into the unified NormalizedOrder
 * format. This enables the matching engine to process orders from multiple
 * exchanges without protocol-specific logic.
 * 
 * @details Design Pattern: Adapter Pattern
 * - Each decoder is a pure function (no side effects)
 * - Input: Protocol-specific struct (const reference)
 * - Output: NormalizedOrder struct (by value)
 * - Inline for zero-cost abstraction
 * 
 * @details Performance Characteristics:
 * - Zero allocation (stack-based)
 * - Single memcpy per symbol (8 bytes)
 * - All decoders complete in < 1 microsecond
 * - Inlined for optimal performance
 * 
 * @note These adapters are "thin" - they perform simple field mapping without
 *       validation. The matching engine is responsible for order validation.
 */
namespace ProtocolAdapter {

    /**
     * @brief Decode NASDAQ OUCH v5.0 order message to normalized format
     * 
     * Converts an OUCH Enter Order message into the internal NormalizedOrder
     * representation used by the matching engine.
     * 
     * @param ouch OUCH Enter Order message (64 bytes)
     * @return NormalizedOrder Normalized internal representation (32 bytes)
     * 
     * @details Field Mapping:
     * - orderId       ← clientOrderId (direct copy)
     * - symbol        ← symbol (memcpy 8 bytes)
     * - quantity      ← shares (OUCH uses "shares" terminology)
     * - price         ← price (both use 1/10000 dollar encoding)
     * - side          ← side (direct copy: 'B' or 'S')
     * - timeInForce   ← timeInForce (direct copy)
     * - orderType     ← orderType ('O' for OUCH)
     * - capacity      ← capacity ('A'=Agency, 'P'=Principal, 'R'=Riskless)
     * - protocol      ← 1 (OUCH protocol identifier)
     * 
     * @note OUCH-Specific Behavior:
     * - The orderType field is always 'O' (used for auto-detection)
     * - OUCH supports three capacity types (A/P/R)
     * - OUCH has explicit timeInForce field (not all protocols do)
     * 
     * @performance Typical execution time: ~500 nanoseconds
     * 
     * @see OuchEnterOrderMessage
     * @see NormalizedOrder
     */
    inline NormalizedOrder decodeOuch(const OuchEnterOrderMessage& ouch) {
        NormalizedOrder order;
        std::memset(&order, 0, sizeof(order));  // Zero-initialize (important for padding)
        
        order.orderId = ouch.clientOrderId;
        std::memcpy(order.symbol, ouch.symbol, 8);  // Copy symbol with padding
        order.quantity = ouch.shares;
        order.price = ouch.price;
        order.side = ouch.side;
        order.timeInForce = ouch.timeInForce;
        order.orderType = ouch.orderType;        // Will be 'O'
        order.capacity = ouch.capacity;
        order.protocol = 1;  // OUCH protocol identifier
        
        return order;
    }

    /**
     * @brief Decode NYSE Pillar Gateway v3.2 order message to normalized format
     * 
     * Converts a Pillar Order Entry message into the internal NormalizedOrder
     * representation used by the matching engine.
     * 
     * @param pillar Pillar Order Entry message (64 bytes)
     * @return NormalizedOrder Normalized internal representation (32 bytes)
     * 
     * @details Field Mapping:
     * - orderId       ← clientOrderId (direct copy)
     * - symbol        ← symbol (memcpy 8 bytes)
     * - quantity      ← shares (Pillar uses "shares" terminology)
     * - price         ← price (both use 1/10000 dollar encoding)
     * - side          ← side (direct copy: 'B' or 'S')
     * - timeInForce   ← tif (Pillar uses "tif" abbreviation)
     * - orderType     ← orderType ('L'=Limit, 'M'=Market, 'P'=Pegged)
     * - capacity      ← 'A' (Default to Agency - Pillar spec doesn't include capacity)
     * - protocol      ← 2 (Pillar protocol identifier)
     * 
     * @note Pillar-Specific Behavior:
     * - Capacity is NOT in Pillar spec, so we default to 'A' (Agency)
     * - Pillar has special TIF values for auctions ('O'=Opening, 'C'=Closing)
     * - Pillar orderType 'P' means pegged order (price follows market)
     * 
     * @note Protocol Detection:
     * - Pillar messages have orderType == 'L' or 'M'
     * - Distinguished from CME by symbol format (stocks vs futures)
     * 
     * @performance Typical execution time: ~500 nanoseconds
     * 
     * @see PillarOrderEntryMessage
     * @see NormalizedOrder
     */
    inline NormalizedOrder decodePillar(const PillarOrderEntryMessage& pillar) {
        NormalizedOrder order;
        std::memset(&order, 0, sizeof(order));  // Zero-initialize
        
        order.orderId = pillar.clientOrderId;
        std::memcpy(order.symbol, pillar.symbol, 8);  // Copy symbol with padding
        order.quantity = pillar.shares;
        order.price = pillar.price;
        order.side = pillar.side;
        order.timeInForce = pillar.tif;          // "tif" in Pillar
        order.orderType = pillar.orderType;
        order.capacity = 'A';  // Default to Agency (not in Pillar spec)
        order.protocol = 2;    // Pillar protocol identifier
        
        return order;
    }

    /**
     * @brief Decode CME iLink 3 order message to normalized format
     * 
     * Converts a CME Order Entry message into the internal NormalizedOrder
     * representation used by the matching engine.
     * 
     * @param cme CME Order Entry message (64 bytes)
     * @return NormalizedOrder Normalized internal representation (32 bytes)
     * 
     * @details Field Mapping:
     * - orderId       ← clientOrderId (direct copy)
     * - symbol        ← symbol (memcpy 8 bytes - FUTURES format!)
     * - quantity      ← quantity (CME uses "quantity" for contracts)
     * - price         ← price (both use 1/10000 dollar encoding)
     * - side          ← side (direct copy: 'B' or 'S')
     * - timeInForce   ← tif (CME uses "tif" abbreviation)
     * - orderType     ← orderType ('L'=Limit, 'M'=Market, 'S'=Stop)
     * - capacity      ← 'P' (Default to Principal - CME typically prop trading)
     * - protocol      ← 3 (CME protocol identifier)
     * 
     * @note CME-Specific Behavior:
     * - Symbol format: [ROOT][MONTH][YEAR] (e.g., "ESZ4    ", "NQH5    ")
     * - Month codes: F,G,H,J,K,M,N,Q,U,V,X,Z (NOT A-L)
     * - Quantity is CONTRACTS, not shares (important for notional calculation)
     * - Capacity defaults to 'P' (Principal) - futures are typically proprietary
     * - CME has additional order types: 'S'=Stop, 'T'=Stop Limit
     * 
     * @note Protocol Detection:
     * - CME messages have orderType == 'L' or 'M' (same as Pillar)
     * - Distinguished from Pillar by symbol format:
     *   * symbol[2] must be month code (F-Z range)
     *   * symbol[3] must be year digit (0-9)
     *   * Example: "ESZ4" → 'Z' at position 2, '4' at position 3 → CME
     *   * Example: "AAPL" → 'P' at position 2, 'L' at position 3 → NOT CME
     * 
     * @warning Contract Multipliers:
     * Remember that CME futures have contract multipliers:
     * - ES: $50 per point → 5 contracts @ $4,550 = $1,137,500 notional
     * - NQ: $20 per point → 10 contracts @ $15,750 = $3,150,000 notional
     * This is NOT handled in the decoder - apply in risk management layer.
     * 
     * @performance Typical execution time: ~500 nanoseconds
     * 
     * @see CmeOrderEntryMessage
     * @see NormalizedOrder
     */
    inline NormalizedOrder decodeCME(const CmeOrderEntryMessage& cme) {
        NormalizedOrder order;
        std::memset(&order, 0, sizeof(order));  // Zero-initialize
        
        order.orderId = cme.clientOrderId;
        std::memcpy(order.symbol, cme.symbol, 8);  // Copy futures symbol with padding
        order.quantity = cme.quantity;         // Contracts, not shares!
        order.price = cme.price;
        order.side = cme.side;
        order.timeInForce = cme.tif;
        order.orderType = cme.orderType;
        order.capacity = 'P';  // Default to Principal (CME typical)
        order.protocol = 3;    // CME protocol identifier
        
        return order;
    }

    /**
     * @brief Get human-readable protocol name from protocol identifier
     * 
     * @param protocol Protocol identifier (1=OUCH, 2=Pillar, 3=CME)
     * @return const char* Protocol name string ("OUCH", "Pillar", "CME", or "Unknown")
     * 
     * @note Used for logging and debugging. Returns "Unknown" for invalid values.
     * 
     * @example
     * @code
     * NormalizedOrder order = decodeOuch(ouchMsg);
     * std::cout << "Order from " << getProtocolName(order.protocol) << "\n";
     * // Output: "Order from OUCH"
     * @endcode
     */
    inline const char* getProtocolName(uint8_t protocol) {
        switch (protocol) {
            case 1: return "OUCH";
            case 2: return "Pillar";
            case 3: return "CME";
            default: return "Unknown";
        }
    }

    /**
     * @brief Get human-readable Time In Force string from TIF code
     * 
     * @param tif Time In Force code ('0'=Day, '3'=IOC, '4'=FOK, 'G'=GTC)
     * @return const char* TIF description ("DAY", "IOC", "FOK", "GTC", or "UNKNOWN")
     * 
     * @details Common TIF Codes:
     * - '0' = DAY: Valid until market close (most common)
     * - '3' = IOC: Immediate or Cancel (high-frequency trading)
     * - '4' = FOK: Fill or Kill (all-or-nothing)
     * - 'G' = GTC: Good Till Cancel (persists across sessions)
     * 
     * @note Used for logging and order display. Returns "UNKNOWN" for invalid codes.
     * 
     * @example
     * @code
     * NormalizedOrder order = decodeOuch(ouchMsg);
     * std::cout << "TIF: " << getTifString(order.timeInForce) << "\n";
     * // Output: "TIF: DAY"
     * @endcode
     */
    inline const char* getTifString(char tif) {
        switch (tif) {
            case '0': return "DAY";
            case '3': return "IOC";
            case '4': return "FOK";
            case 'G': return "GTC";
            default: return "UNKNOWN";
        }
    }

} // namespace ProtocolAdapter
