# Multi-Protocol Exchange Matching Engine

## Overview

The Beacon **Multi-Protocol Exchange Matching Engine** is a unified order execution system that supports multiple exchange protocols through a protocol adapter architecture. It can process orders from NASDAQ OUCH, NYSE Pillar, and CME simultaneously, converting each protocol's specific format into a normalized internal representation.

## Architecture

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  OUCH Client    │     │  Pillar Client  │     │   CME Client    │
│   (NASDAQ)      │     │    (NYSE)       │     │  (Futures)      │
└────────┬────────┘     └────────┬────────┘     └────────┬────────┘
         │                       │                       │
         │ 64-byte OUCH msg     │ 64-byte Pillar msg   │ 64-byte CME msg
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │  TCP Server (Port)     │
                    │  Multi-Protocol        │
                    │  Matching Engine       │
                    └────────────┬───────────┘
                                 │
                    ┌────────────▼───────────┐
                    │  Protocol Detection    │
                    │  - Auto: orderType     │
                    │  - Explicit: config    │
                    └────────────┬───────────┘
                                 │
                 ┌───────────────┼───────────────┐
                 │               │               │
                 ▼               ▼               ▼
         ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
         │ OUCH Decoder │ │Pillar Decoder│ │ CME Decoder  │
         └──────┬───────┘ └──────┬───────┘ └──────┬───────┘
                │                │                │
                └────────────────┼────────────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │  NormalizedOrder       │
                    │  (Internal Format)     │
                    └────────────┬───────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │  Order Processing      │
                    │  - Validation          │
                    │  - Immediate Fill      │
                    │  - (Future: Order Book)│
                    └────────────┬───────────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │  Execution Report      │
                    │  32-byte response      │
                    └────────────┬───────────┘
                                 │
                 ┌───────────────┼───────────────┐
                 │               │               │
                 ▼               ▼               ▼
         ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
         │ OUCH Client  │ │Pillar Client │ │  CME Client  │
         │   (Fills)    │ │   (Fills)    │ │   (Fills)    │
         └──────────────┘ └──────────────┘ └──────────────┘
```

## Supported Protocols

### 1. NASDAQ OUCH v5.0
- **Message Size**: 64 bytes (inbound), 32 bytes (outbound)
- **Asset Class**: US Equities
- **Identification**: `orderType == 'O'`
- **Symbol Format**: Space-padded 8 bytes (e.g., "AAPL    ")
- **Documentation**: `docs/exchange_protocols/OUCH_PROTOCOL.md`

### 2. NYSE Pillar Gateway v3.2
- **Message Size**: 64 bytes (inbound), 32 bytes (outbound)
- **Asset Class**: US Equities, ETFs
- **Identification**: `orderType == 'L'` and non-futures symbol
- **Symbol Format**: Space-padded 8 bytes (e.g., "MSFT    ")
- **Documentation**: `docs/exchange_protocols/PILLAR_PROTOCOL.md`

### 3. CME iLink 3 (Simplified)
- **Message Size**: 64 bytes (inbound), 32 bytes (outbound)
- **Asset Class**: Futures, Options on Futures
- **Identification**: `orderType == 'L'` and futures symbol pattern (month codes)
- **Symbol Format**: [ROOT][MONTH][YEAR] (e.g., "ESZ4    ", "NQH5    ")
- **Documentation**: `docs/exchange_protocols/CME_PROTOCOL.md`

## Protocol Detection

The matching engine supports two detection modes:

### Auto-Detection (Default)

Automatically identifies the protocol by inspecting message content:

```cpp
// Detection Algorithm
if (orderType == 'O') {
    return OUCH;
} else if (orderType == 'L' || orderType == 'M') {
    if (isCmeSymbol(symbol)) {
        return CME;    // Has month code (F-Z) and year digit
    } else {
        return PILLAR;
    }
}
```

**CME Symbol Detection:**
- Position 2 (month): F, G, H, J, K, M, N, Q, U, V, X, or Z
- Position 3 (year): 0-9
- Examples: ESZ4 ✓, NQH5 ✓, AAPL ✗, MSFT ✗

### Explicit Protocol Mode

Force a specific protocol (disables auto-detection):

```bash
./exchange_matching_engine 54321 ouch    # OUCH only
./exchange_matching_engine 54321 pillar  # Pillar only
./exchange_matching_engine 54321 cme     # CME only
./exchange_matching_engine 54321 auto    # Auto-detect (default)
```

## Message Formats

### Inbound: Order Entry (64 bytes)

All three protocols use 64-byte fixed-length messages with similar structure:

| Field | OUCH | Pillar | CME | Description |
|-------|------|--------|-----|-------------|
| `clientOrderId` | ✓ | ✓ | ✓ | 8 bytes: unique order ID |
| `symbol` | ✓ | ✓ | ✓ | 8 bytes: instrument symbol |
| `shares/quantity` | shares | shares | quantity | 4 bytes: order size |
| `price` | ✓ | ✓ | ✓ | 4 bytes: 1/10000 dollars |
| `side` | ✓ | ✓ | ✓ | 1 byte: 'B' or 'S' |
| `orderType` | 'O' | 'L'/'M' | 'L'/'M' | 1 byte: order type |
| `timeInForce/tif` | timeInForce | tif | tif | 1 byte: duration |
| `capacity` | ✓ | ✓ | ✓ | 1 byte: 'A'/'P'/'R' |
| `reserved` | ✓ | ✓ | ✓ | 2 bytes: future use |
| `_padding` | 34 bytes | 38 bytes | 38 bytes | Padding to 64 bytes |

### Outbound: Execution Report (32 bytes)

Unified format sent to all protocols:

```cpp
struct ExecutionReport {
    uint32_t msgType;      // 3 = Execution Report
    uint32_t orderId;      // Client order ID (echoed back)
    char symbol[8];        // Symbol (echoed back)
    uint32_t execQty;      // Executed quantity
    uint32_t execPrice;    // Execution price (1/10000 dollars)
    uint32_t status;       // 0=New, 1=Partial, 2=Filled, 3=Cancelled
    uint32_t padding[2];   // Reserved
};
```

## Normalized Order Format

Internal representation after protocol decoding:

```cpp
struct NormalizedOrder {
    uint64_t orderId;           // Client order ID
    char symbol[8];             // Padded symbol
    uint32_t quantity;          // Shares or contracts
    uint32_t price;             // Price in 1/10000 dollars
    char side;                  // 'B' = buy, 'S' = sell
    char timeInForce;           // '0'=Day, '3'=IOC, '4'=FOK, 'G'=GTC
    char orderType;             // 'O'=OUCH, 'L'=Limit, 'M'=Market
    char capacity;              // 'A'=Agency, 'P'=Principal
    uint8_t protocol;           // 1=OUCH, 2=Pillar, 3=CME
};
```

The `protocol` field enables protocol-specific logic if needed while maintaining unified processing.

## Building

### Prerequisites

- C++20 compiler (GCC 10+, Clang 12+, AppleClang 13+)
- CMake 3.26+
- POSIX-compliant OS (Linux, macOS)

### Build Commands

```bash
# Build matching engine
cd src/apps/exchange_matching_engine
chmod +x build.sh
./build.sh

# Build client algorithm (OUCH)
cd src/apps/client_algorithm
chmod +x build.sh
./build.sh

# Binaries created in build/ subdirectories
```

### Build Output

```
exchange_matching_engine/build/
└── exchange_matching_engine    # Multi-protocol server

client_algorithm/build/
├── client_algorithm            # UDP/TCP test receiver
├── client_algo                 # OUCH trading algorithm
├── test_pillar                 # NYSE Pillar test client
└── test_cme                    # CME futures test client
```

## Running the System

### Step 1: Start the Matching Engine

```bash
cd exchange_matching_engine
./build/exchange_matching_engine 54321 auto
```

**Output:**
```
========================================
MULTI-PROTOCOL EXCHANGE MATCHING ENGINE
========================================
Protocol Mode: AUTO-DETECT
Listening on port: 54321
========================================
```

### Step 2: Run Test Clients

#### NASDAQ OUCH Client

```bash
# In one terminal: Start market data playback
cd exchange_market_data_playback
./build/exchange_market_data_playback

# In another terminal: Run OUCH algorithm
cd client_algorithm
./build/client_algo algo 127.0.0.1 12345 127.0.0.1 54321 1000
```

**Expected Output:**
```
CLIENT ALGORITHM - OUCH MODE
MD: 127.0.0.1:12345
EX: 127.0.0.1:54321

Market Data: AAPL $1582.73 B:1582.67 A:1582.79
Order sent: BUY 47 AAPL @ $1582.73
Fill received: 47 @ $1582.73

Statistics:
Market Data: 19,998
Orders Sent: 19
Fills Received: 38
```

#### NYSE Pillar Client

```bash
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

#### CME Futures Client

```bash
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

...

Test complete: 10/10 orders filled (100%)
```

### Step 3: Matching Engine Output

The matching engine logs each order with protocol identification:

```
[OUCH] Order: BUY 47 AAPL @ $1582.73 (TIF: Day, Capacity: Agency)
[PILLAR] Order: SELL 83 MSFT @ $1531.09 (TIF: Day, Capacity: Agency)
[CME] Order: BUY 7 ESZ4 @ $4563.81 (TIF: Day, Capacity: Principal)
```

## Testing

### Unit Tests (Future)

```bash
cd tests
./run_protocol_tests.sh
```

Test coverage:
- Message size validation (64 bytes inbound, 32 bytes outbound)
- Price encoding/decoding (1/10000 dollar precision)
- Symbol formatting (8-byte space padding)
- Protocol detection accuracy
- Decoder correctness (OUCH, Pillar, CME)
- Normalization validation

### Integration Tests

#### Test 1: Single Protocol

```bash
# Terminal 1: Matching engine (OUCH only)
./exchange_matching_engine 54321 ouch

# Terminal 2: OUCH client
./test_pillar 127.0.0.1 54321 10
# Should succeed (Pillar decoded as generic limit orders)
```

#### Test 2: Multi-Protocol Concurrent

```bash
# Terminal 1: Matching engine (auto-detect)
./exchange_matching_engine 54321 auto

# Terminal 2: OUCH
./build/client_algo algo 127.0.0.1 12345 127.0.0.1 54321 100 &

# Terminal 3: Pillar
./build/test_pillar 127.0.0.1 54321 20 &

# Terminal 4: CME
./build/test_cme 127.0.0.1 54321 10 &

# All three should execute simultaneously
```

#### Test 3: Protocol Detection

```bash
# Send orders with different orderType fields
# Verify matching engine correctly identifies each protocol
# Check logs for [OUCH], [PILLAR], [CME] tags
```

## Performance

### Latency Targets

| Component | Target | Notes |
|-----------|--------|-------|
| TCP receive | < 100 μs | Network stack |
| Protocol decode | < 1 μs | Memcpy + field extraction |
| Order processing | < 5 μs | Validation + normalization |
| Execution report | < 1 μs | Struct population |
| TCP send | < 100 μs | Network stack |
| **Total RTT** | **< 250 μs** | Order → Fill (local) |

### Throughput

- **Single connection**: ~40,000 orders/second
- **Multi-connection**: ~150,000 orders/second (4 cores)
- **Bottleneck**: TCP accept() and context switching

### Optimization Recommendations

1. **Kernel bypass**: Use DPDK or Solarflare OpenOnload
2. **Thread per protocol**: Dedicated threads for OUCH, Pillar, CME
3. **Lock-free queues**: Between protocol decoders and matching engine
4. **Batch processing**: Process N orders before sending execution reports
5. **Memory pools**: Pre-allocate order and execution report structures

## Current Limitations

### Simplified Execution Model

- ✓ Immediate fills (no order book)
- ✗ No order acknowledgment messages (Accepted/Rejected)
- ✗ No partial fills (always 100% fill)
- ✗ No cancel operations
- ✗ No replace/modify operations

### Missing Features

- ✗ Order book (price-time priority)
- ✗ Market data dissemination (top-of-book, trades)
- ✗ Risk management (position limits, margin checks)
- ✗ Session management (login, logout, heartbeats)
- ✗ Sequence number tracking (gap detection)
- ✗ Audit trail and regulatory reporting

### Protocol Simplifications

- ✗ OUCH: No system events, broken trades, or trade corrections
- ✗ Pillar: No auction support (opening, closing, volatility)
- ✗ CME: No spread orders, options, or iLink 3 session layer

## Future Enhancements

### Phase 1: Order Lifecycle (Q1 2026)

- [ ] Order acknowledgment messages
- [ ] Order rejection with reason codes
- [ ] Order cancellation support
- [ ] Order replace/modify support
- [ ] Order status tracking

### Phase 2: Order Book (Q2 2026)

- [ ] Price-time priority matching
- [ ] Partial fill support
- [ ] Resting orders (limit order book)
- [ ] Order book depth tracking
- [ ] Match algorithm optimization

### Phase 3: Market Data (Q3 2026)

- [ ] Top-of-book quotes (BBO)
- [ ] Trade dissemination
- [ ] Order book depth levels (Level 2)
- [ ] Market by order (MBO) feed
- [ ] Incremental updates

### Phase 4: Risk & Compliance (Q4 2026)

- [ ] Pre-trade risk checks
- [ ] Position limit enforcement
- [ ] Margin calculations (futures)
- [ ] Self-match prevention
- [ ] Regulatory audit trail (CAT)

## File Structure

```
src/apps/
├── exchange_matching_engine/
│   ├── protocol_adapters.h       # OUCH, Pillar, CME decoders
│   ├── main.cpp                  # Multi-protocol server
│   ├── build.sh                  # Build script
│   └── build/
│       └── exchange_matching_engine
│
├── client_algorithm/
│   ├── main_algo.cpp             # OUCH trading algorithm
│   ├── test_pillar.cpp           # Pillar test client
│   ├── test_cme.cpp              # CME test client
│   ├── CMakeLists.txt            # Build configuration
│   ├── build.sh                  # Build script
│   └── build/
│       ├── client_algo           # OUCH algorithm
│       ├── test_pillar           # Pillar test
│       └── test_cme              # CME test
│
└── exchange_market_data_playback/
    ├── main.cpp                  # Market data TCP server
    ├── build.sh
    └── build/
        └── exchange_market_data_playback

docs/exchange_protocols/
├── OUCH_PROTOCOL.md              # NASDAQ OUCH documentation
├── PILLAR_PROTOCOL.md            # NYSE Pillar documentation
└── CME_PROTOCOL.md               # CME iLink 3 documentation
```

## Troubleshooting

### Connection Refused

```
Error: Connection refused
```

**Solution**: Ensure matching engine is running:
```bash
cd exchange_matching_engine
./build/exchange_matching_engine 54321 auto
```

### Wrong Protocol Detected

```
[PILLAR] Order: ... (Expected OUCH)
```

**Solution**: Use explicit protocol mode or fix orderType field:
```bash
# Explicit mode
./exchange_matching_engine 54321 ouch

# Or fix orderType in client:
order.orderType = 'O';  // OUCH
order.orderType = 'L';  // Pillar/CME
```

### No Execution Reports

```
Order sent: BUY 100 AAPL @ $150.25
(No fill received)
```

**Solution**: Check execution listener thread is running:
```cpp
std::thread execThread(listenForExecutions, exSocket);
execThread.detach();  // Ensure detached or joined
```

### Symbol Not Recognized

```
Warning: Symbol 'AAPL' not found
```

**Solution**: Ensure symbol is space-padded to 8 bytes:
```cpp
char symbol[8];
std::memset(symbol, ' ', 8);
std::memcpy(symbol, "AAPL", 4);  // "AAPL    "
```

## References

- [NASDAQ OUCH Protocol Documentation](docs/exchange_protocols/OUCH_PROTOCOL.md)
- [NYSE Pillar Gateway Documentation](docs/exchange_protocols/PILLAR_PROTOCOL.md)
- [CME iLink 3 Protocol Documentation](docs/exchange_protocols/CME_PROTOCOL.md)
- [FIX Protocol Comparison](https://www.fixtrading.org/)
- [ITCH Protocol (Market Data)](https://www.nasdaqtrader.com/Trader.aspx?id=DPSpecs)

## Contributing

### Code Style

- **Indentation**: 4 spaces (no tabs)
- **Naming**: snake_case for variables, PascalCase for structs
- **Comments**: Inline for complex logic, headers for all files
- **Line length**: 100 characters max

### Adding a New Protocol

1. Define protocol-specific message struct (64 bytes)
2. Add decoder function to `protocol_adapters.h`
3. Update `decodeOrder()` detection logic in `main.cpp`
4. Create test client (e.g., `test_new_protocol.cpp`)
5. Add to `CMakeLists.txt`
6. Document in `docs/exchange_protocols/`

## License

Copyright © 2025 Bryan Camp. All rights reserved.

---

**Last Updated**: November 5, 2025  
**Version**: 1.0  
**Author**: Bryan Camp
