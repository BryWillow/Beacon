# NYSE Pillar Gateway Protocol Documentation

## Overview

**Pillar** is the New York Stock Exchange's (NYSE) unified trading platform and gateway protocol. It consolidates order entry, market data, and trade reporting across all NYSE markets (NYSE, NYSE American, NYSE Arca, NYSE National).

## Protocol Characteristics

- **Type**: Binary, fixed-length messages
- **Transport**: TCP (primary), UDP (market data)
- **Byte Order**: Network byte order (big-endian)
- **Message Size**: 64 bytes for order entry
- **Version**: 3.2 (current implementation)
- **Markets Supported**: NYSE, NYSE American, NYSE Arca, NYSE National

---

## Message Types

### Inbound Messages (Client → Exchange)

#### 1. New Order Message (64 bytes)

Submits a new order to NYSE.

**Structure:**
```cpp
struct PillarOrderEntryMessage {
    uint64_t clientOrderId;      // 8 bytes: unique client order ID
    char symbol[8];              // 8 bytes: stock symbol (space-padded)
    uint32_t shares;             // 4 bytes: order quantity
    uint32_t price;              // 4 bytes: price in 1/10000 dollars
    char side;                   // 1 byte: 'B'=buy, 'S'=sell
    char orderType;              // 1 byte: 'L'=limit, 'M'=market
    char tif;                    // 1 byte: time in force
    char capacity;               // 1 byte: trading capacity
    uint16_t reserved;           // 2 bytes: reserved
    char _padding[38];           // 38 bytes: padding to 64 bytes
};
```

**Field Details:**

| Field | Type | Description | Valid Values |
|-------|------|-------------|--------------|
| `clientOrderId` | uint64_t | Unique order identifier | 1 - 2^64-1 |
| `symbol` | char[8] | Stock symbol, space-padded | "AAPL    ", "MSFT    " |
| `shares` | uint32_t | Number of shares | 1 - 99,999,999 |
| `price` | uint32_t | Price in 1/10000 dollars | 0 (market order) or 1-999,999,999 |
| `side` | char | Buy or sell | 'B' = Buy, 'S' = Sell |
| `orderType` | char | Order type | 'L' = Limit, 'M' = Market |
| `tif` | char | Time in force | '0' = Day, '3' = IOC, '4' = FOK, 'G' = GTC |
| `capacity` | char | Trading capacity | 'A' = Agency, 'P' = Principal |
| `reserved` | uint16_t | Reserved for future use | Set to 0 |

**Example:**
```
Order: BUY 500 MSFT @ $375.50, Day Order, Limit
clientOrderId: 67890
symbol: "MSFT    "
shares: 500
price: 3755000 (represents $375.50)
side: 'B'
orderType: 'L'
tif: '0'
capacity: 'A'
```

#### 2. Cancel Order Message (64 bytes)

Cancels an existing order.

**Structure:**
```cpp
struct PillarCancelMessage {
    uint64_t clientOrderId;      // 8 bytes: order ID to cancel
    uint32_t cancelShares;       // 4 bytes: shares to cancel (0 = all)
    uint16_t reserved;           // 2 bytes: reserved
    char _padding[50];           // 50 bytes: padding
};
```

#### 3. Modify Order Message (64 bytes)

Modifies price or quantity of an existing order.

**Structure:**
```cpp
struct PillarModifyMessage {
    uint64_t clientOrderId;      // 8 bytes: original order ID
    uint64_t newOrderId;         // 8 bytes: new order ID (must be unique)
    uint32_t newShares;          // 4 bytes: new share count
    uint32_t newPrice;           // 4 bytes: new price
    uint16_t reserved;           // 2 bytes: reserved
    char _padding[36];           // 36 bytes: padding
};
```

---

### Outbound Messages (Exchange → Client)

#### 1. Order Acknowledgment

Confirms order has been received and entered into the order book.

**Fields:**
- Client Order ID
- Exchange Order ID (assigned by NYSE)
- Symbol
- Shares
- Price
- Timestamp (nanoseconds since midnight)
- Order Book ID

#### 2. Order Reject

Indicates order was rejected before entering the book.

**Rejection Codes:**
```cpp
enum PillarRejectReason {
    INVALID_SYMBOL = 1,
    INVALID_PRICE = 2,
    INVALID_QUANTITY = 3,
    MARKET_CLOSED = 4,
    HALTED = 5,
    DUPLICATE_ORDER_ID = 6,
    PRICE_OUT_OF_RANGE = 7,
    LIMIT_UP_LIMIT_DOWN_VIOLATION = 8,
    INSUFFICIENT_BUYING_POWER = 9,
    SHORT_SALE_RESTRICTION = 10
};
```

#### 3. Trade Execution

Reports a full or partial fill.

**Fields:**
- Client Order ID
- Exchange Order ID
- Executed shares
- Execution price
- Trade ID (unique match number)
- Contra party (anonymized)
- Liquidity indicator (maker/taker)
- Timestamp

#### 4. Order Cancelled

Confirms order cancellation.

**Cancellation Reasons:**
- User requested
- IOC/FOK expiration
- Exchange-initiated (regulatory halt, etc.)
- Market close
- Self-match prevention

---

## Price Encoding

Pillar uses the same **fixed-point encoding** as OUCH:

```
Stored Value = Price × 10,000
```

**Examples:**
| Decimal Price | Stored Value | Calculation |
|---------------|--------------|-------------|
| $250.00 | 2,500,000 | 250.00 × 10,000 |
| $375.50 | 3,755,000 | 375.50 × 10,000 |
| $0.0001 | 1 | 0.0001 × 10,000 (minimum tick) |
| $99,999.99 | 999,999,900 | 99,999.99 × 10,000 |

**Special Values:**
- `0` = Market order (no price specified)
- `0xFFFFFFFF` = Pegged order (price derived from market)

---

## Order Types

| Code | Name | Description | Price Required |
|------|------|-------------|----------------|
| 'L' | Limit | Execute at specified price or better | Yes |
| 'M' | Market | Execute at best available price | No (set to 0) |
| 'P' | Pegged | Price tracks market (e.g., midpoint) | Special |
| 'S' | Stop | Becomes market order when triggered | Trigger price |
| 'T' | Stop Limit | Becomes limit order when triggered | Both trigger and limit |

---

## Time In Force (TIF) Options

| Code | Name | Description | Pillar-Specific Notes |
|------|------|-------------|----------------------|
| '0' | Day | Valid until market close | Cancels at 4:00 PM ET |
| '3' | IOC | Immediate or Cancel | Unfilled portion cancelled immediately |
| '4' | FOK | Fill or Kill | All-or-nothing execution |
| 'G' | GTC | Good Till Cancel | Persists across sessions |
| 'O' | Opening Only | Execute in opening auction only | NYSE-specific |
| 'C' | Closing Only | Execute in closing auction only | NYSE-specific |

---

## Trading Capacity

| Code | Name | Description | Regulatory Impact |
|------|------|-------------|-------------------|
| 'A' | Agency | Acting as agent for customer | Order marked for best execution |
| 'P' | Principal | Trading for firm's own account | Different reporting requirements |
| 'R' | Riskless Principal | Simultaneous offsetting trade | Must be marked correctly for compliance |

---

## Symbol Formatting

Symbols are **8 bytes, space-padded on the right**.

**NYSE-Specific Examples:**
```
"IBM     "   // IBM Corp
"GE      "   // General Electric
"C       "   // Citigroup
"BAC     "   // Bank of America
"XOM     "   // Exxon Mobil
"SPY     "   // SPDR S&P 500 ETF (NYSE Arca)
"VXX     "   // VIX ETF (NYSE Arca)
```

**Preferred Stock Notation:**
```
"BAC-PL  "   // Bank of America Preferred Series L
"JPM-PC  "   // JPMorgan Chase Preferred Series C
```

---

## Connection and Session Management

### Session Lifecycle

1. **TCP Connection**: Client establishes TCP connection to Pillar Gateway
2. **Login Request**: Client sends credentials + firm ID
3. **Login Response**: Exchange confirms with session ID
4. **Heartbeat**: Bidirectional heartbeats every 1 second
5. **Active Trading**: Orders accepted
6. **Logout**: Clean disconnect or timeout

### Sequence Numbers

Pillar uses **sequence numbers** for message ordering and gap detection:

```cpp
struct PillarMessageHeader {
    uint16_t messageLength;      // Total message length
    uint16_t messageType;        // Message type code
    uint64_t sequenceNumber;     // Incrementing sequence
    uint64_t timestamp;          // Nanoseconds since midnight
};
```

**Gap Detection:**
- Client tracks expected sequence number
- If gap detected, request retransmission
- Exchange can replay messages from sequence N

---

## Market Phases

NYSE operates in distinct trading phases:

| Phase | Time (ET) | Description | Order Entry Allowed |
|-------|-----------|-------------|---------------------|
| Pre-Opening | 4:00 AM - 9:30 AM | Accumulate orders for opening auction | Yes (Day, Opening Only) |
| Opening Auction | ~9:30 AM | Price discovery, single opening price | No (queued orders execute) |
| Core Trading | 9:30 AM - 4:00 PM | Continuous trading | Yes (all order types) |
| Closing Auction | ~4:00 PM | Single closing price | No (queued orders execute) |
| After Hours | 4:00 PM - 8:00 PM | Extended hours trading | Yes (limited) |

**Order Behavior by Phase:**
- **Day orders**: Cancelled at market close (4:00 PM)
- **GTC orders**: Persist to next trading day
- **Opening/Closing Only**: Execute only in respective auctions

---

## Error Handling

### Client-Side Validation

Before sending order:
- ✓ Symbol is listed on NYSE/Arca/American
- ✓ Price adheres to tick size rules ($0.01 for stocks > $1.00)
- ✓ Quantity is round lot multiple (100 shares) or odd lot
- ✓ Order type is valid for current market phase
- ✓ Short sale restrictions respected (uptick rule)

### NYSE-Specific Rejections

```cpp
enum NyseRejectReason {
    LULD_VIOLATION = 8,          // Limit Up Limit Down
    SHORT_SALE_RESTRICTION = 10, // Reg SHO violation
    SELF_MATCH_PREVENTED = 11,   // Would trade with own order
    AUCTION_LOCKOUT = 12,        // Can't cancel during auction
    REGULATORY_HALT = 13         // Trading halted by SEC/NYSE
};
```

---

## Performance Considerations

### Latency

**Typical Round-Trip Times (RTT):**
- Co-located in Mahwah, NJ data center: **50-200 microseconds**
- New York City: **500-1000 microseconds**
- Chicago: **5-10 milliseconds**
- West Coast: **30-50 milliseconds**

### Optimization Techniques

1. **Co-location**: Place servers in NYSE Mahwah data center
2. **Kernel Bypass**: Use DPDK or Solarflare OpenOnload
3. **Lock-Free Queues**: Zero-copy message passing
4. **CPU Pinning**: Dedicate cores to trading threads
5. **Huge Pages**: Reduce TLB misses

```cpp
// Example: CPU pinning
cpu_set_t cpuset;
CPU_ZERO(&cpuset);
CPU_SET(2, &cpuset);  // Pin to core 2
pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
```

---

## Regulatory Considerations

### Order Marking

**Rule 15c3-5 (Market Access Rule):**
- All orders must have pre-trade risk checks
- Firms must prevent erroneous orders (price, quantity)
- Maximum order size limits enforced

**Reg SHO (Short Sale Rule):**
- Short sales must be marked as 'S' (short) or 'L' (long)
- Short sales prohibited below bid during downtick

### Audit Trail Requirements

**Rule 613 (Consolidated Audit Trail):**
- All orders must be reported to CAT
- Client order IDs must be traceable
- Timestamps must be synchronized to atomic clock

---

## Code Examples

### Sending a Pillar Order

```cpp
#include "protocol_adapters.h"

int sendPillarOrder(int sock) {
    PillarOrderEntryMessage order;
    std::memset(&order, 0, sizeof(order));
    
    order.clientOrderId = 123456;
    std::memcpy(order.symbol, "IBM     ", 8);
    order.shares = 500;
    order.price = 1575000;  // $157.50
    order.side = 'B';
    order.orderType = 'L';  // Limit
    order.tif = '0';        // Day
    order.capacity = 'A';   // Agency
    
    // Send order
    ssize_t sent = send(sock, &order, sizeof(order), 0);
    
    if (sent != sizeof(order)) {
        std::cerr << "Failed to send order\n";
        return -1;
    }
    
    std::cout << "Pillar order sent: BUY " << order.shares 
              << " IBM @ $" << (order.price / 10000.0) << "\n";
    
    return 0;
}
```

### Receiving Pillar Executions

```cpp
void receivePillarExecutions(int sock) {
    ExecutionReport exec;
    
    while (true) {
        ssize_t bytes = recv(sock, &exec, sizeof(exec), 0);
        
        if (bytes <= 0) {
            std::cerr << "Connection closed\n";
            break;
        }
        
        if (bytes == sizeof(exec) && exec.msgType == 3) {
            std::cout << "PILLAR FILL: "
                      << "OrderID=" << exec.orderId
                      << " Qty=" << exec.execQty
                      << " Price=$" << (exec.execPrice / 10000.0)
                      << " Status=" << static_cast<int>(exec.status)
                      << "\n";
        }
    }
}
```

---

## Testing

### Unit Tests

```cpp
TEST(PillarProtocol, MessageSize) {
    EXPECT_EQ(sizeof(PillarOrderEntryMessage), 64);
}

TEST(PillarProtocol, PriceEncoding) {
    uint32_t price = 3755000;  // $375.50
    double decimal = price / 10000.0;
    EXPECT_DOUBLE_EQ(decimal, 375.50);
}

TEST(PillarProtocol, SymbolPadding) {
    PillarOrderEntryMessage order;
    std::memcpy(order.symbol, "IBM     ", 8);
    EXPECT_EQ(order.symbol[0], 'I');
    EXPECT_EQ(order.symbol[1], 'B');
    EXPECT_EQ(order.symbol[2], 'M');
    EXPECT_EQ(order.symbol[3], ' ');
}
```

### Integration Test

Run the Pillar test client:
```bash
# Start matching engine
cd exchange_matching_engine
./build/exchange_matching_engine 54321 pillar

# In another terminal, run Pillar test client
cd client_algorithm
./build/test_pillar 127.0.0.1 54321 20
```

**Expected Output:**
```
Pillar Test Client Started
Connecting to 127.0.0.1:54321...
Connected successfully!

Sending order 1: BUY 47 AAPL @ $1582.73
Received fill: 47 @ $1582.73

Sending order 2: SELL 83 MSFT @ $1531.09
Received fill: 83 @ $1531.09

...

Test complete: 20/20 orders filled (100%)
```

---

## Differences from NASDAQ OUCH

| Feature | OUCH | Pillar | Notes |
|---------|------|--------|-------|
| Protocol | Binary | Binary | Both use fixed-length |
| Message Size | 64 bytes | 64 bytes | Identical in this implementation |
| Price Encoding | 1/10000 dollars | 1/10000 dollars | Same fixed-point |
| Sequence Numbers | Optional | Required | Pillar tracks gaps |
| Market Phases | Simpler | Complex (auctions) | NYSE has opening/closing auctions |
| Capacity Field | 3 values (A/P/R) | 3 values (A/P/R) | Same regulatory requirements |
| Symbol Format | Space-padded | Space-padded | Both 8 bytes |
| Order Type Codes | 'O' | 'L'/'M' | Different identifiers |

---

## Beacon-Specific Implementation Notes

### Current Status

✅ **Implemented:**
- New Order Message (64 bytes)
- Execution Reports (32 bytes)
- Protocol adapter for normalization
- Test client for validation

⏳ **Not Yet Implemented:**
- Cancel Order
- Modify Order
- Sequence number tracking
- Market phase awareness
- Opening/Closing auction handling
- Regulatory order marking (short sale, etc.)

### File Locations

```
src/apps/exchange_matching_engine/
├── protocol_adapters.h          # Pillar decoder
└── main.cpp                     # Multi-protocol matching engine

src/apps/client_algorithm/
├── test_pillar.cpp              # Pillar test client
└── CMakeLists.txt               # Build configuration

docs/exchange_protocols/
└── PILLAR_PROTOCOL.md           # This document
```

### Protocol Adapter Integration

```cpp
// From protocol_adapters.h
NormalizedOrder ProtocolAdapter::decodePillar(const void* buffer) {
    const PillarOrderEntryMessage* msg = 
        static_cast<const PillarOrderEntryMessage*>(buffer);
    
    NormalizedOrder order;
    order.orderId = msg->clientOrderId;
    std::memcpy(order.symbol, msg->symbol, 8);
    order.quantity = msg->shares;
    order.price = msg->price;
    order.side = msg->side;
    order.timeInForce = msg->tif;
    order.orderType = msg->orderType;
    order.capacity = msg->capacity;
    order.protocol = 2;  // Pillar
    
    return order;
}
```

---

## References

- NYSE Pillar Gateway Specification v3.2
- NYSE Integrated Feed v2.1 Specification
- SEC Rule 611 (Order Protection Rule)
- SEC Rule 15c3-5 (Market Access Rule)
- Beacon Implementation: `src/apps/client_algorithm/test_pillar.cpp`

---

**Last Updated**: November 5, 2025  
**Author**: Bryan Camp  
**Version**: 1.0
