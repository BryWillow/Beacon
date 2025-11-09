# NASDAQ OUCH v5.0 Protocol Documentation

## Overview

**OUCH (Order Utility and Connection Handler)** is NASDAQ's proprietary binary protocol for order entry and management. This document describes the implementation in the Beacon trading system.

## Protocol Characteristics

- **Type**: Binary, fixed-length messages
- **Transport**: TCP (reliable, ordered delivery)
- **Byte Order**: Network byte order (big-endian for multi-byte fields)
- **Message Size**: 64 bytes for inbound, 32 bytes for outbound
- **Version**: 5.0 (current implementation)

---

## Message Types

### Inbound Messages (Client → Exchange)

#### 1. Enter Order Message (64 bytes)

Submits a new order to the exchange.

**Structure:**
```cpp
struct OuchEnterOrderMessage {
    uint64_t clientOrderId;      // 8 bytes: unique client order ID
    char symbol[8];              // 8 bytes: padded stock symbol (space-filled)
    uint32_t shares;             // 4 bytes: number of shares
    uint32_t price;              // 4 bytes: price in 1/10000 dollars
    char side;                   // 1 byte: 'B'=buy, 'S'=sell
    char timeInForce;            // 1 byte: '0'=Day, '3'=IOC, '4'=FOK
    char orderType;              // 1 byte: 'O'=order
    char capacity;               // 1 byte: 'A'=agency, 'P'=principal
    uint16_t reserved;           // 2 bytes: reserved for future use
    char _padding[34];           // 34 bytes: padding to 64 bytes
};
```

**Field Details:**

| Field | Type | Description | Valid Values |
|-------|------|-------------|--------------|
| `clientOrderId` | uint64_t | Unique order identifier assigned by client | 1 - 2^64-1 |
| `symbol` | char[8] | Stock symbol, space-padded on right | "AAPL    ", "MSFT    " |
| `shares` | uint32_t | Order quantity | 1 - 999,999,999 |
| `price` | uint32_t | Price in 1/10000 dollars (fixed-point) | Example: 1,502,500 = $150.25 |
| `side` | char | Buy or sell indicator | 'B' = Buy, 'S' = Sell |
| `timeInForce` | char | Order duration | '0' = Day, '3' = IOC (Immediate or Cancel), '4' = FOK (Fill or Kill) |
| `orderType` | char | Type of order | 'O' = Standard order |
| `capacity` | char | Trading capacity | 'A' = Agency, 'P' = Principal, 'R' = Riskless Principal |
| `reserved` | uint16_t | Reserved for future use | Set to 0 |

**Example:**
```
Order: BUY 100 AAPL @ $150.25, Day Order, Agency
clientOrderId: 12345
symbol: "AAPL    " (space-padded)
shares: 100
price: 1502500 (represents $150.25)
side: 'B'
timeInForce: '0'
orderType: 'O'
capacity: 'A'
```

#### 2. Cancel Order Message (64 bytes)

Cancels an existing order.

**Structure:**
```cpp
struct OuchCancelOrderMessage {
    uint64_t clientOrderId;      // 8 bytes: order ID to cancel
    uint32_t canceledShares;     // 4 bytes: shares to cancel (0 = all)
    uint16_t reserved;           // 2 bytes: reserved
    char _padding[50];           // 50 bytes: padding to 64 bytes
};
```

#### 3. Replace Order Message (64 bytes)

Replaces an existing order with a new one.

**Structure:**
```cpp
struct OuchReplaceOrderMessage {
    uint64_t originalOrderId;    // 8 bytes: original order ID
    uint64_t newOrderId;         // 8 bytes: new order ID
    uint32_t shares;             // 4 bytes: new share count
    uint32_t price;              // 4 bytes: new price
    char side;                   // 1 byte: 'B' or 'S'
    char timeInForce;            // 1 byte: TIF
    char orderType;              // 1 byte: order type
    char capacity;               // 1 byte: capacity
    uint16_t reserved;           // 2 bytes: reserved
    char _padding[34];           // 34 bytes: padding
};
```

---

### Outbound Messages (Exchange → Client)

#### 1. Order Accepted Message

Confirms order has been accepted and entered into the order book.

**Fields:**
- Order ID
- Symbol
- Shares
- Price
- Side
- Timestamp
- Order reference number (exchange-assigned)

#### 2. Order Rejected Message

Indicates order was rejected.

**Rejection Reasons:**
- Invalid symbol
- Invalid price (outside price bands)
- Insufficient buying power
- Duplicate order ID
- Market closed
- Halted symbol

#### 3. Order Executed Message

Reports a full or partial fill.

**Fields:**
- Order ID
- Executed shares
- Execution price
- Match number (unique trade ID)
- Liquidity flag (added/removed liquidity)
- Timestamp

#### 4. Order Cancelled Message

Confirms order cancellation.

**Fields:**
- Order ID
- Cancelled shares
- Reason (user requested, IOC expired, etc.)
- Timestamp

---

## Price Encoding

OUCH uses **fixed-point decimal encoding** for prices:

```
Stored Value = Price × 10,000
```

**Examples:**
| Decimal Price | Stored Value | Calculation |
|---------------|--------------|-------------|
| $100.00 | 1,000,000 | 100.00 × 10,000 |
| $150.25 | 1,502,500 | 150.25 × 10,000 |
| $0.01 | 100 | 0.01 × 10,000 |
| $9,999.99 | 99,999,900 | 9,999.99 × 10,000 |

**Conversion Functions:**
```cpp
// Decimal to OUCH
uint32_t toOuchPrice(double price) {
    return static_cast<uint32_t>(price * 10000.0);
}

// OUCH to decimal
double fromOuchPrice(uint32_t ouchPrice) {
    return ouchPrice / 10000.0;
}
```

---

## Time In Force (TIF) Options

| Code | Name | Description |
|------|------|-------------|
| '0' | Day | Valid until market close |
| '3' | IOC | Immediate or Cancel - execute immediately, cancel unfilled portion |
| '4' | FOK | Fill or Kill - execute entire order immediately or cancel |
| 'G' | GTC | Good Till Cancel - persists across trading sessions |

---

## Trading Capacity

| Code | Name | Description |
|------|------|-------------|
| 'A' | Agency | Acting as agent for customer |
| 'P' | Principal | Trading for own account |
| 'R' | Riskless Principal | Simultaneous offsetting transaction |

---

## Symbol Formatting

Symbols are **8 bytes, space-padded on the right**.

**Examples:**
```
"AAPL    "  // Apple Inc.
"MSFT    "  // Microsoft
"GOOGL   "  // Alphabet (Class A)
"BRK.A   "  // Berkshire Hathaway (includes dot)
"T       "  // AT&T (1 character, 7 spaces)
```

**Code:**
```cpp
void formatSymbol(const char* input, char* output) {
    std::memset(output, ' ', 8);  // Fill with spaces
    std::memcpy(output, input, std::min(strlen(input), 8ul));
}
```

---

## Connection and Session Management

### Establishing Connection

1. **TCP Handshake**: Client initiates TCP connection to exchange
2. **Login Message**: Client sends login credentials (not implemented in simple version)
3. **Session Established**: Exchange confirms, ready to accept orders

### Heartbeats

- Exchange sends periodic heartbeat messages
- Client must respond to avoid disconnection
- Typical interval: 1-5 seconds

### Disconnection Handling

- All open orders are **automatically cancelled** on disconnect
- Client must reconnect and re-enter orders
- Order IDs must be unique across sessions

---

## Error Handling

### Client-Side Validation

Before sending order, validate:
- ✓ Order ID is unique
- ✓ Symbol is valid and properly formatted
- ✓ Price is within tick size rules
- ✓ Quantity is within exchange limits
- ✓ Account has sufficient buying power

### Exchange Rejections

Common rejection reasons:
```cpp
enum RejectReason {
    INVALID_SYMBOL = 1,
    INVALID_PRICE = 2,
    INVALID_QUANTITY = 3,
    DUPLICATE_ORDER_ID = 4,
    MARKET_CLOSED = 5,
    HALTED = 6,
    INSUFFICIENT_FUNDS = 7
};
```

---

## Performance Considerations

### Latency Optimization

1. **TCP_NODELAY**: Disable Nagle's algorithm
   ```cpp
   int noDelay = 1;
   setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));
   ```

2. **Socket Buffers**: Increase buffer sizes
   ```cpp
   int bufSize = 2 * 1024 * 1024;  // 2 MB
   setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufSize, sizeof(bufSize));
   ```

3. **Pre-allocated Structures**: Reuse message structures
   ```cpp
   OuchEnterOrderMessage order;  // Allocate once
   // Reuse for multiple orders
   ```

### Throughput

- **Typical**: 10,000 - 50,000 orders/second per connection
- **Burst**: Up to 100,000 orders/second
- **Message Size**: 64 bytes = efficient network utilization

---

## Code Examples

### Sending an Order

```cpp
#include "protocol_adapters.h"

int main() {
    // Connect to exchange
    int sock = connectToExchange("127.0.0.1", 54321);
    
    // Create order
    OuchEnterOrderMessage order;
    std::memset(&order, 0, sizeof(order));
    
    order.clientOrderId = 12345;
    std::memcpy(order.symbol, "AAPL    ", 8);
    order.shares = 100;
    order.price = 1502500;  // $150.25
    order.side = 'B';
    order.timeInForce = '0';  // Day
    order.orderType = 'O';
    order.capacity = 'A';  // Agency
    
    // Send order
    send(sock, &order, sizeof(order), 0);
    
    // Receive acknowledgment
    ExecutionReport ack;
    recv(sock, &ack, sizeof(ack), 0);
    
    return 0;
}
```

### Receiving Execution Reports

```cpp
void listenForExecutions(int sock) {
    ExecutionReport exec;
    
    while (true) {
        ssize_t bytes = recv(sock, &exec, sizeof(exec), 0);
        
        if (bytes != sizeof(exec)) break;
        
        if (exec.msgType == 3) {  // Execution
            std::cout << "FILLED: " << exec.execQty 
                      << " @ $" << (exec.execPrice / 10000.0) << "\n";
        }
    }
}
```

---

## Testing

### Unit Tests

```cpp
TEST(OuchProtocol, PriceEncoding) {
    EXPECT_EQ(toOuchPrice(150.25), 1502500);
    EXPECT_DOUBLE_EQ(fromOuchPrice(1502500), 150.25);
}

TEST(OuchProtocol, SymbolFormatting) {
    char symbol[8];
    formatSymbol("AAPL", symbol);
    EXPECT_EQ(std::string(symbol, 8), "AAPL    ");
}

TEST(OuchProtocol, MessageSize) {
    EXPECT_EQ(sizeof(OuchEnterOrderMessage), 64);
    EXPECT_EQ(sizeof(ExecutionReport), 32);
}
```

### Integration Test

Run the test client:
```bash
./build/client_algo algo 127.0.0.1 12345 127.0.0.1 54321 100
```

---

## References

- NASDAQ OUCH Specification v5.0
- NASDAQ Market Data Standards
- FIX Protocol comparison: OUCH is binary, FIX is text-based
- Beacon Implementation: `src/apps/client_algorithm/main_algo.cpp`

---

## Beacon-Specific Implementation Notes

### Current Status

✅ **Implemented:**
- Enter Order Message (64 bytes)
- Execution Reports (32 bytes)
- Price encoding/decoding
- Symbol formatting
- Protocol adapter for normalization

⏳ **Not Yet Implemented:**
- Cancel Order
- Replace Order
- Order Accepted/Rejected messages
- Order book (orders immediately fill)
- Partial fills
- Login/session management

### File Locations

```
src/apps/exchange_matching_engine/
├── protocol_adapters.h          # OUCH decoder to normalized format
└── main.cpp                     # Matching engine with OUCH support

src/apps/client_algorithm/
├── main_algo.cpp                # Client sending OUCH orders
└── test_ouch.cpp                # OUCH test client (future)

src/libs/exchange_protocols/nsdq/execution/ouch/
├── v5.0/
│   └── execution_message_types.h    # Official OUCH structures
└── current/
    └── messages_active.cpp          # Active version alias
```

---

**Last Updated**: November 5, 2025  
**Author**: Bryan Camp  
**Version**: 1.0
