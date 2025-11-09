# CME (Chicago Mercantile Exchange) Protocol Documentation

## Overview

**CME Globex** is the CME Group's electronic trading platform for derivatives. It handles futures and options contracts across multiple asset classes including equity indexes, interest rates, foreign exchange, energy, agriculture, and metals.

## Protocol Characteristics

- **Type**: Binary, fixed-length messages (simplified implementation)
- **Transport**: TCP (order entry), UDP Multicast (market data)
- **Byte Order**: Network byte order (big-endian)
- **Message Size**: 64 bytes for order entry
- **Platform**: iLink 3 (CME's FIX-based protocol, simplified here)
- **Asset Classes**: Futures, Options, Spreads

---

## Message Types

### Inbound Messages (Client → Exchange)

#### 1. New Order Single Message (64 bytes)

Submits a new order for a futures or options contract.

**Structure:**
```cpp
struct CmeOrderEntryMessage {
    uint64_t clientOrderId;      // 8 bytes: unique client order ID
    char symbol[8];              // 8 bytes: futures symbol (e.g., "ESZ4    ")
    uint32_t quantity;           // 4 bytes: number of contracts
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
| `symbol` | char[8] | Futures contract symbol | "ESZ4    ", "NQH5    ", "ZNZ4    " |
| `quantity` | uint32_t | Number of contracts | 1 - 999,999 |
| `price` | uint32_t | Price in 1/10000 dollars | 0 (market) or 1-999,999,999 |
| `side` | char | Buy or sell | 'B' = Buy (long), 'S' = Sell (short) |
| `orderType` | char | Order type | 'L' = Limit, 'M' = Market, 'S' = Stop |
| `tif` | char | Time in force | '0' = Day, '3' = IOC, '4' = FOK, 'G' = GTC |
| `capacity` | char | Trading capacity | 'A' = Agency, 'P' = Principal, 'M' = Market Maker |
| `reserved` | uint16_t | Reserved for future use | Set to 0 |

**Example:**
```
Order: BUY 5 ESZ4 @ $4,550.00, Day Order, Limit
clientOrderId: 99887766
symbol: "ESZ4    " (E-mini S&P 500 December 2024)
quantity: 5 (5 contracts = $227,500 notional at $4,550/point)
price: 45500000 (represents $4,550.00)
side: 'B'
orderType: 'L'
tif: '0'
capacity: 'P' (Principal)
```

---

## CME Futures Symbol Format

CME symbols use a **root + month code + year** format:

### Structure: `[ROOT][MONTH][YEAR]`

**Examples:**
```
ESZ4     = E-mini S&P 500, December 2024
NQH5     = E-mini Nasdaq-100, March 2025
YMM4     = E-mini Dow, June 2024
RTY4     = E-mini Russell 2000 (year only, perpetual)
ZNZ4     = 10-Year T-Note, December 2024
GCZ4     = Gold, December 2024
CLZ4     = Crude Oil, December 2024
```

### Month Codes

| Code | Month | Code | Month | Code | Month | Code | Month |
|------|-------|------|-------|------|-------|------|-------|
| F | January | G | February | H | March | J | April |
| K | May | M | June | N | July | Q | August |
| U | September | V | October | X | November | Z | December |

**Mnemonic**: "**F**ancy **G**irls **H**aving **J**oyful **K**itchen **M**eetings **N**early **Q**uite **U**nique **V**ery **X**tra **Z**ealous"

### Popular CME Futures Contracts

| Symbol Root | Description | Contract Size | Tick Size |
|-------------|-------------|---------------|-----------|
| ES | E-mini S&P 500 | $50 × S&P 500 Index | 0.25 points = $12.50 |
| NQ | E-mini Nasdaq-100 | $20 × Nasdaq-100 Index | 0.25 points = $5.00 |
| YM | E-mini Dow | $5 × DJIA Index | 1.00 point = $5.00 |
| RTY | E-mini Russell 2000 | $50 × Russell 2000 Index | 0.10 points = $5.00 |
| ZN | 10-Year T-Note | $100,000 face value | 1/64 point = $15.625 |
| ZB | 30-Year T-Bond | $100,000 face value | 1/32 point = $31.25 |
| GC | Gold | 100 troy ounces | $0.10/oz = $10.00 |
| CL | Crude Oil (WTI) | 1,000 barrels | $0.01/barrel = $10.00 |
| 6E | Euro FX | €125,000 | 0.00005 = $6.25 |

---

## Price Encoding

### Standard Pricing (Most Futures)

Like stocks, CME uses **fixed-point encoding** for index futures:

```
Stored Value = Price × 10,000
```

**Examples:**
| Decimal Price | Stored Value | Contract | Calculation |
|---------------|--------------|----------|-------------|
| $4,550.00 | 45,500,000 | ES (S&P 500) | 4,550.00 × 10,000 |
| $15,750.25 | 157,502,500 | NQ (Nasdaq) | 15,750.25 × 10,000 |
| $2,000.00 | 20,000,000 | Gold (GC) | 2,000.00 × 10,000 |
| $85.50 | 855,000 | Crude Oil (CL) | 85.50 × 10,000 |

### Treasury Futures (Special Pricing)

**Treasury futures use fractional notation** (e.g., 1/32, 1/64):

```
10-Year T-Note (ZN): Price in 1/64ths
30-Year T-Bond (ZB): Price in 1/32nds
```

**Example Conversion (ZN):**
```
Price: 110-16    (110 and 16/32 = 110.50)
Stored: 1105000  (110.50 × 10,000)

Price: 110-16+   (110 and 16.5/32 = 110.515625)
Stored: 1105156  (110.515625 × 10,000)
```

---

## Contract Multipliers and Notional Value

Understanding notional exposure is critical for risk management:

### Equity Index Futures

```
Notional Value = Price × Multiplier × Quantity

ES @ 4,550.00:
Notional = 4,550 × $50 × 5 contracts = $1,137,500

NQ @ 15,750.25:
Notional = 15,750.25 × $20 × 10 contracts = $3,150,050
```

### Per-Tick Value

```
Tick Value = Tick Size × Multiplier

ES: 0.25 points × $50 = $12.50 per tick per contract
NQ: 0.25 points × $20 = $5.00 per tick per contract
```

**Example P&L:**
```
Trade: BUY 5 ES @ 4,550.00
Fill:  SELL 5 ES @ 4,555.00 (exit)

Points: 5.00 points profit
Ticks: 5.00 / 0.25 = 20 ticks
P&L: 20 ticks × $12.50 × 5 contracts = $1,250 profit
```

---

## Order Types

| Code | Name | Description | Use Case |
|------|------|-------------|----------|
| 'L' | Limit | Execute at specified price or better | Control entry price |
| 'M' | Market | Execute immediately at best price | Quick entry/exit |
| 'S' | Stop | Becomes market order when price hit | Stop loss protection |
| 'T' | Stop Limit | Becomes limit order when triggered | Stop with price control |
| 'K' | Market-to-Limit | Market order that converts to limit if not filled | Aggressive but controlled |

---

## Time In Force (TIF) Options

| Code | Name | Description | CME-Specific Notes |
|------|------|-------------|-------------------|
| '0' | Day | Valid until end of trading day | Cancels at session close |
| '3' | IOC | Immediate or Cancel | Partial fills allowed |
| '4' | FOK | Fill or Kill | All-or-nothing execution |
| 'G' | GTC | Good Till Cancel | Persists across sessions (max 90 days) |
| 'T' | GTD | Good Till Date | Expires on specified date |

---

## Trading Capacity

| Code | Name | Description | CME Context |
|------|------|-------------|-------------|
| 'A' | Agency | Acting for customer | Order marked for customer account |
| 'P' | Principal | Trading for firm account | Proprietary trading |
| 'M' | Market Maker | Designated market maker | Special obligations/rights |
| 'H' | Hedge | Hedging physical position | Different margin requirements |

---

## Trading Sessions

CME operates **nearly 24 hours** with brief maintenance windows:

### E-mini Equity Index Futures (ES, NQ, YM, RTY)

| Session | Time (CT) | Description | Liquidity |
|---------|-----------|-------------|-----------|
| Globex Evening | 5:00 PM - 8:30 AM | Overnight electronic trading | Lower volume |
| Pre-Open | 8:30 AM - 9:30 AM | Order accumulation | Building |
| RTH (Regular Trading Hours) | 9:30 AM - 4:00 PM | Pit and electronic | Highest volume |
| Post-Close | 4:00 PM - 5:00 PM | After-hours trading | Lower volume |
| Maintenance | 5:00 PM - 6:00 PM | System maintenance | NO TRADING |

### Treasury Futures (ZN, ZB)

- Trade nearly 24 hours (6:00 PM Sunday - 5:00 PM Friday CT)
- Brief maintenance: 5:00 PM - 6:00 PM CT daily

---

## Margin and Risk

### Initial Margin Requirements

Approximate initial margins (subject to change):

| Contract | Symbol | Initial Margin | Maintenance Margin |
|----------|--------|----------------|--------------------|
| E-mini S&P 500 | ES | $12,650 | $11,500 |
| E-mini Nasdaq-100 | NQ | $17,600 | $16,000 |
| E-mini Dow | YM | $11,000 | $10,000 |
| E-mini Russell 2000 | RTY | $6,600 | $6,000 |
| 10-Year T-Note | ZN | $1,650 | $1,500 |

**Leverage:**
```
ES @ 4,550.00:
Notional = 4,550 × $50 = $227,500 per contract
Margin = $12,650
Leverage = 227,500 / 12,650 = 18:1
```

### Position Limits

CME enforces position limits to prevent market manipulation:

```cpp
enum PositionLimits {
    ES_SPECULATIVE_LIMIT = 20000,  // 20,000 contracts
    NQ_SPECULATIVE_LIMIT = 8000,   // 8,000 contracts
    CL_SPECULATIVE_LIMIT = 6000,   // 6,000 contracts (crude oil)
};
```

---

## Auto-Detection in Beacon

The Beacon matching engine auto-detects CME messages using **symbol patterns**:

```cpp
// From protocol_adapters.h
bool isCmeSymbol(const char* symbol) {
    // CME futures have format: [ROOT][MONTH][YEAR]
    // Month codes: F,G,H,J,K,M,N,Q,U,V,X,Z (positions 2-3)
    // Year codes: 0-9 (position 3 or 4)
    
    char monthCode = symbol[2];
    char yearCode = symbol[3];
    
    return (monthCode >= 'F' && monthCode <= 'Z') &&
           (yearCode >= '0' && yearCode <= '9');
}
```

**Detection Examples:**
```
"ESZ4    " → Month='Z', Year='4' → CME ✓
"NQH5    " → Month='H', Year='5' → CME ✓
"AAPL    " → Month='P', Year='L' → NOT CME (stock)
"MSFT    " → Month='F', Year='T' → NOT CME (stock)
```

---

## Error Handling

### Pre-Trade Risk Checks

CME enforces strict risk controls:

```cpp
enum CmeRiskCheck {
    MAX_ORDER_QUANTITY = 10000,     // Per order
    PRICE_COLLAR_PERCENT = 5,       // ±5% from last price
    MAX_DAILY_LOSS = 50000,         // $50K per account
    MAX_POSITION = 500,             // Max open contracts
};
```

### Rejection Reasons

```cpp
enum CmeRejectReason {
    INVALID_SYMBOL = 1,
    CONTRACT_EXPIRED = 2,           // Trading expired contract
    PRICE_OUT_OF_BAND = 3,          // Outside daily limit
    INSUFFICIENT_MARGIN = 4,
    POSITION_LIMIT_EXCEEDED = 5,
    SELF_TRADE_PREVENTION = 6,      // Would trade with own order
    MARKET_CLOSED = 7,
    DUPLICATE_ORDER_ID = 8,
    RISK_LIMIT_EXCEEDED = 9,
};
```

---

## Performance Considerations

### Latency Targets

**CME Co-location (Aurora, IL):**
- Order-to-acknowledgment: **100-500 microseconds**
- Market data latency: **50-200 microseconds**
- Round-trip (order → fill): **200-800 microseconds**

### Optimization Techniques

1. **Kernel Bypass Networking**: Solarflare, Mellanox
2. **FPGA-based Trading**: Hardware order generation
3. **CPU Affinity**: Pin threads to specific cores
4. **Lock-Free Programming**: Zero-copy, wait-free data structures
5. **Market Data Consolidation**: Subscribe only to relevant contracts

```cpp
// Example: Setting thread priority for CME trading
#include <sched.h>

void setHighPriority() {
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler");
    }
}
```

---

## Code Examples

### Sending a CME Order

```cpp
#include "protocol_adapters.h"

int sendCmeOrder(int sock) {
    CmeOrderEntryMessage order;
    std::memset(&order, 0, sizeof(order));
    
    order.clientOrderId = 445566;
    std::memcpy(order.symbol, "ESZ4    ", 8);  // E-mini S&P 500 Dec 2024
    order.quantity = 5;                         // 5 contracts
    order.price = 45500000;                     // $4,550.00
    order.side = 'B';                           // Buy (long)
    order.orderType = 'L';                      // Limit
    order.tif = '0';                            // Day
    order.capacity = 'P';                       // Principal
    
    // Send order
    ssize_t sent = send(sock, &order, sizeof(order), 0);
    
    if (sent != sizeof(order)) {
        std::cerr << "Failed to send CME order\n";
        return -1;
    }
    
    double priceDecimal = order.price / 10000.0;
    std::cout << "CME order sent: "
              << (order.side == 'B' ? "BUY" : "SELL") << " "
              << order.quantity << " "
              << std::string(order.symbol, 8) << " @ $"
              << priceDecimal << "\n";
    
    return 0;
}
```

### Calculating Notional Value

```cpp
double calculateNotional(const char* symbol, uint32_t price, uint32_t qty) {
    // Symbol → Multiplier mapping
    std::map<std::string, double> multipliers = {
        {"ES", 50.0},    // E-mini S&P 500
        {"NQ", 20.0},    // E-mini Nasdaq-100
        {"YM", 5.0},     // E-mini Dow
        {"RTY", 50.0},   // E-mini Russell 2000
        {"ZN", 1000.0},  // 10-Year T-Note
        {"GC", 100.0},   // Gold
        {"CL", 1000.0},  // Crude Oil
    };
    
    std::string root(symbol, 2);  // First 2 chars
    double multiplier = multipliers[root];
    double priceDecimal = price / 10000.0;
    
    return priceDecimal * multiplier * qty;
}

// Example usage:
double notional = calculateNotional("ESZ4    ", 45500000, 5);
// Returns: 4550.00 × $50 × 5 = $1,137,500
```

---

## Testing

### Unit Tests

```cpp
TEST(CmeProtocol, MessageSize) {
    EXPECT_EQ(sizeof(CmeOrderEntryMessage), 64);
}

TEST(CmeProtocol, SymbolDetection) {
    EXPECT_TRUE(isCmeSymbol("ESZ4    "));
    EXPECT_TRUE(isCmeSymbol("NQH5    "));
    EXPECT_FALSE(isCmeSymbol("AAPL    "));
    EXPECT_FALSE(isCmeSymbol("MSFT    "));
}

TEST(CmeProtocol, PriceEncoding) {
    uint32_t price = 45500000;  // $4,550.00
    double decimal = price / 10000.0;
    EXPECT_DOUBLE_EQ(decimal, 4550.00);
}

TEST(CmeProtocol, NotionalCalculation) {
    double notional = calculateNotional("ESZ4    ", 45500000, 5);
    EXPECT_DOUBLE_EQ(notional, 1137500.0);
}
```

### Integration Test

Run the CME test client:
```bash
# Start matching engine in CME mode
cd exchange_matching_engine
./build/exchange_matching_engine 54321 cme

# In another terminal, run CME test client
cd client_algorithm
./build/test_cme 127.0.0.1 54321 10
```

**Expected Output:**
```
CME Test Client Started
Testing with CME futures contracts
Connecting to 127.0.0.1:54321...
Connected successfully!

Sending order 1: BUY 7 ESZ4 @ $4563.81
Received fill: 7 @ $4563.81

Sending order 2: SELL 3 NQZ4 @ $4527.19
Received fill: 3 @ $4527.19

Sending order 3: BUY 5 YMZ4 @ $4581.42
Received fill: 5 @ $4581.42

...

Test complete: 10/10 orders filled (100%)
```

---

## Differences from Stock Protocols

| Feature | OUCH/Pillar (Stocks) | CME (Futures) | Notes |
|---------|----------------------|---------------|-------|
| Symbol Format | "AAPL    " | "ESZ4    " | Futures have month/year codes |
| Asset Type | Equity shares | Derivative contracts | Futures have expiration |
| Quantity Field | `shares` | `quantity` (contracts) | Different terminology |
| Notional Value | Price × Shares | Price × Multiplier × Qty | Futures use multipliers |
| Trading Hours | 9:30 AM - 4:00 PM ET | Nearly 24 hours | CME trades overnight |
| Margin | 50% initial (stocks) | ~5-10% (futures) | Much higher leverage |
| Settlement | T+2 (physical shares) | Cash settled (most) | No share delivery |
| Position Limits | Generally unlimited | Exchange-imposed limits | Prevent manipulation |

---

## Beacon-Specific Implementation Notes

### Current Status

✅ **Implemented:**
- New Order Single (64 bytes)
- Execution Reports (32 bytes)
- CME symbol format support
- Auto-detection via symbol pattern matching
- Protocol adapter for normalization

⏳ **Not Yet Implemented:**
- Cancel Order
- Modify Order
- Contract expiration handling
- Margin calculations
- Position limit enforcement
- Multi-leg spread orders
- Options on futures

### File Locations

```
src/apps/exchange_matching_engine/
├── protocol_adapters.h          # CME decoder
└── main.cpp                     # Multi-protocol matching engine

src/apps/client_algorithm/
├── test_cme.cpp                 # CME test client
└── CMakeLists.txt               # Build configuration

docs/exchange_protocols/
└── CME_PROTOCOL.md              # This document
```

### Protocol Adapter Integration

```cpp
// From protocol_adapters.h
NormalizedOrder ProtocolAdapter::decodeCME(const void* buffer) {
    const CmeOrderEntryMessage* msg = 
        static_cast<const CmeOrderEntryMessage*>(buffer);
    
    NormalizedOrder order;
    order.orderId = msg->clientOrderId;
    std::memcpy(order.symbol, msg->symbol, 8);
    order.quantity = msg->quantity;  // Contracts, not shares
    order.price = msg->price;
    order.side = msg->side;
    order.timeInForce = msg->tif;
    order.orderType = msg->orderType;
    order.capacity = msg->capacity;
    order.protocol = 3;  // CME
    
    return order;
}
```

---

## References

- CME iLink 3 Specification
- CME Globex Market Data Platform
- CME Group Contract Specifications
- CFTC Position Limits Rules
- Beacon Implementation: `src/apps/client_algorithm/test_cme.cpp`

---

**Last Updated**: November 5, 2025  
**Author**: Bryan Camp  
**Version**: 1.0
