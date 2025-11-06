# Beacon Trading System

## Overview

The Beacon Trading System is a complete, multi-protocol electronic trading ecosystem designed for ultra-low latency algorithmic trading. The system supports multiple exchange protocols (NASDAQ OUCH, NYSE Pillar, CME iLink 3) and provides end-to-end infrastructure for market data generation, distribution, order execution, and algorithmic trading.

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         BEACON TRADING SYSTEM                            │
└─────────────────────────────────────────────────────────────────────────┘

┌──────────────────────┐
│ Market Data Generator│  Generates synthetic market data
│  (md_generator)      │  • NASDAQ ITCH 5.0
└──────┬───────────────┘  • NYSE Pillar
       │ UDP Multicast    • CME MDP 3.0
       │ 239.255.0.1      Configurable rates, symbols, prices
       ▼
┌──────────────────────┐
│ Market Data Playback │  Replays historical or synthetic data
│  (md_playback)       │  • UDP multicast streaming
└──────┬───────────────┘  • Configurable playback speed
       │                  • Statistics tracking
       │
       ├──────────────────────────────────┐
       │                                  │
       ▼                                  ▼
┌──────────────────────┐          ┌──────────────────────┐
│  Client Algorithm    │          │  Exchange Matching   │
│  (client_algo)       │◄────────►│     Engine           │
│                      │   TCP    │  (matching_engine)   │
│  • Market data sub   │  Orders  │                      │
│  • Order generation  │ 54321    │  • Multi-protocol    │
│  • Risk management   │          │  • Order matching    │
│  • Position tracking │          │  • Fill generation   │
│  • PnL calculation   │          │  • Market simulation │
└──────────────────────┘          └──────────────────────┘
       │                                  │
       │                                  │
       ▼                                  ▼
   Statistics                        Statistics
   • Latency metrics                • Order rate
   • Order flow                     • Match rate
   • Risk thresholds                • Market depth
   • Position limits                • Fill rate
```

## System Components

### 1. Market Data Generator (`exchange_market_data_generator`)

**Purpose**: Generates synthetic market data in exchange-native formats

**Key Features**:
- Multi-protocol support: NASDAQ ITCH, NYSE Pillar, CME MDP
- Configurable parameters:
  - Symbol list and behavior
  - Price ranges and volatility
  - Message rates (orders/sec)
  - Market hours simulation
- Realistic market microstructure:
  - Bid/ask spreads
  - Order book depth
  - Trade flow patterns
  - Price movements

**Communication**:
- Output: UDP multicast (239.255.0.1:12345)
- Protocols: ITCH 5.0, Pillar, MDP 3.0

**Configuration**: `sample_config.json`

**Performance Metrics**:
- Message generation rate (msgs/sec)
- CPU utilization
- Memory footprint
- Network throughput

---

### 2. Market Data Playback (`exchange_market_data_playback`)

**Purpose**: Replays pre-recorded or synthetic market data for testing and backtesting

**Key Features**:
- File-based playback
- UDP multicast streaming
- Speed control (1x, 10x, 100x, etc.)
- Loop/repeat functionality
- Timestamped message replay
- Statistics collection

**Communication**:
- Input: Binary files (PCAP, custom format)
- Output: UDP multicast (239.255.0.1:12345)

**Use Cases**:
- Algorithm backtesting
- Latency testing
- Regression testing
- Demo/training scenarios

**Performance Metrics**:
- Playback rate accuracy
- Message timing precision
- Network throughput
- Timestamp drift

---

### 3. Exchange Matching Engine (`exchange_matching_engine`)

**Purpose**: Simulates exchange order matching and execution

**Key Features**:
- **Multi-Protocol Support**:
  - NASDAQ OUCH 5.0 (orders)
  - NYSE Pillar Gateway 3.2 (orders)
  - CME iLink 3 (futures orders)
- **Order Management**:
  - Price-time priority matching
  - Multiple order types (Limit, Market, IOC, FOK, etc.)
  - Order modifications and cancellations
- **Market Simulation**:
  - Real-time order book maintenance
  - Fill generation and reporting
  - Trade confirmations
  - Reject handling
- **Protocol Detection**: Automatically detects incoming protocol

**Communication**:
- Input: TCP connections (port 54321)
- Protocols: OUCH, Pillar, iLink 3
- Output: Execution reports (fills, acks, rejects)

**Performance Metrics**:
- Order processing latency (μs)
- Match rate (orders/sec)
- Order book depth
- Fill rate percentage
- Protocol detection overhead

**Threading Model**:
- Accept thread: Handles new connections
- Connection threads: One per client
- Matching engine: Lock-free order processing

---

### 4. Client Algorithm (`client_algorithm`)

**Purpose**: Algorithmic trading client with risk management

**Key Features**:
- **Market Data Processing**:
  - Subscribes to UDP multicast market data
  - Parses multiple exchange protocols
  - Real-time book building
- **Order Generation**:
  - Configurable order frequency
  - Multiple order types
  - Per-product trading logic
- **Risk Management**:
  - **PnL-based limits**: Warning/Alert/Hard Stop
  - **Order reject monitoring**: Cumulative reject tracking
  - **Burst rate control**: Orders per second limits
  - **Position limits**: Per-product algo and firm limits
- **Configuration System**:
  - JSON-based configuration
  - Per-product settings
  - Hot-reloadable risk parameters

**Communication**:
- Input: UDP multicast (239.255.0.1:12345) for market data
- Output: TCP to matching engine (127.0.0.1:54321) for orders
- Protocols: OUCH, Pillar, or iLink 3

**Risk Management Layers**:
1. **Product-level**: `algo_position_limit`, `firm_position_limit`
2. **Algorithm-level PnL**: Drawdown thresholds
3. **Algorithm-level Rejects**: Rejection count thresholds
4. **Algorithm-level Burst**: Order rate thresholds

**Performance Metrics**:
- Market data processing latency (μs)
- Order decision latency (μs)
- Fill processing latency (μs)
- PnL calculation overhead
- Risk check overhead

---

## Communication Flow

### Market Data Flow
```
Generator/Playback → UDP Multicast (239.255.0.1:12345)
                     ↓
                  Algorithm (subscribes)
```

### Order Flow
```
Algorithm → TCP (127.0.0.1:54321) → Matching Engine
           ← Execution Reports ←
```

### Complete Trading Loop
```
1. Market Data Generator → UDP Multicast
2. Algorithm receives market data (< 1 μs)
3. Algorithm makes trading decision (< 5 μs)
4. Algorithm sends order via TCP (< 10 μs)
5. Matching Engine receives and matches order (< 5 μs)
6. Matching Engine sends fill report (< 5 μs)
7. Algorithm receives fill, updates position/PnL (< 2 μs)
```

**Total Round-Trip Latency Target**: < 30 μs

---

## Protocol Support Matrix

| Component | NASDAQ OUCH | NYSE Pillar | CME iLink 3 |
|-----------|-------------|-------------|-------------|
| Market Data Generator | ✅ ITCH 5.0 | ✅ Pillar | ✅ MDP 3.0 |
| Market Data Playback | ✅ | ✅ | ✅ |
| Matching Engine | ✅ Order entry | ✅ Order entry | ✅ Order entry |
| Client Algorithm | ✅ | ✅ | ✅ |

---

## Building the System

### Prerequisites
- CMake 3.26+
- C++20 compiler (GCC 11+, Clang 14+)
- nlohmann/json library (included in vendor/)

### Build All Applications

```bash
cd /path/to/Beacon/experimental
mkdir build && cd build
cmake ..
make -j$(nproc)
```

**Build Output**:
```
build/
├── src/apps/exchange_market_data_generator/md_generator
├── src/apps/exchange_market_data_playback/md_playback
├── src/apps/exchange_matching_engine/matching_engine
└── src/apps/client_algorithm/client_algo
```

### Build Individual Applications

```bash
# Market Data Generator
cd src/apps/exchange_market_data_generator
./build.sh

# Market Data Playback
cd src/apps/exchange_market_data_playback
./build.sh

# Matching Engine
cd src/apps/exchange_matching_engine
./build.sh

# Client Algorithm
cd src/apps/client_algorithm
./build.sh
```

---

## Running the System

### Quick Start - Full System Test

**Terminal 1**: Start Matching Engine
```bash
cd src/apps/exchange_matching_engine
./build/matching_engine
# Listening on port 54321...
```

**Terminal 2**: Start Market Data Generator
```bash
cd src/apps/exchange_market_data_generator
./run.sh
# Generating NASDAQ ITCH messages...
# Multicast: 239.255.0.1:12345
```

**Terminal 3**: Start Client Algorithm
```bash
cd src/apps/client_algorithm
./build/client_algo --config sample_config.json
# Connected to market data: 239.255.0.1:12345
# Connected to exchange: 127.0.0.1:54321
# Trading enabled...
```

### Alternative: Use Playback Instead of Generator

**Terminal 2** (alternative):
```bash
cd src/apps/exchange_market_data_playback
./build/md_playback <input_file> 239.255.0.1 12345
# Playing back market data at 1.0x speed...
```

---

## Performance Monitoring

### Real-Time Latency Measurement

Each component includes built-in performance instrumentation:

#### 1. Market Data Generator
```cpp
// Track message generation rate
std::cout << "Generated " << msg_count << " messages in " << elapsed_ms << "ms\n";
std::cout << "Rate: " << (msg_count * 1000.0 / elapsed_ms) << " msgs/sec\n";
```

**Metrics**:
- Messages generated per second
- CPU utilization
- Memory usage
- Network send rate

#### 2. Market Data Playback
```cpp
// Track playback accuracy
std::cout << "Playback rate: " << actual_rate << " msgs/sec (target: " << target_rate << ")\n";
std::cout << "Timing drift: " << drift_us << " μs\n";
```

**Metrics**:
- Playback rate accuracy
- Timestamp precision
- Message loss rate
- Buffer utilization

#### 3. Matching Engine
```cpp
// Track order processing latency
auto start = std::chrono::high_resolution_clock::now();
// ... process order ...
auto end = std::chrono::high_resolution_clock::now();
auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
std::cout << "Order processing latency: " << latency_us << " μs\n";
```

**Metrics**:
- Order-to-ack latency (p50, p99, p99.9)
- Order-to-fill latency
- Orders processed per second
- Active order book depth
- Protocol detection overhead

#### 4. Client Algorithm
```cpp
// Track end-to-end latency
auto md_recv_time = std::chrono::high_resolution_clock::now();
// ... make trading decision ...
auto order_sent_time = std::chrono::high_resolution_clock::now();
// ... receive fill ...
auto fill_recv_time = std::chrono::high_resolution_clock::now();

auto decision_latency = order_sent_time - md_recv_time;
auto roundtrip_latency = fill_recv_time - md_recv_time;
```

**Metrics**:
- Market data processing latency
- Trading decision latency
- Order submission latency
- Fill processing latency
- End-to-end round-trip latency
- Risk check overhead
- PnL calculation overhead

### Performance Profiling Tools

#### Using perf (Linux)
```bash
# Profile matching engine
perf record -g ./build/matching_engine
perf report

# Find hot paths
perf stat -e cycles,instructions,cache-misses ./build/matching_engine
```

#### Using Instruments (macOS)
```bash
# Launch with Instruments
instruments -t "Time Profiler" ./build/matching_engine
```

#### Using Valgrind (Memory profiling)
```bash
valgrind --tool=massif ./build/client_algo --config sample_config.json
ms_print massif.out.xxxxx
```

### Latency Histograms

Add to any component:
```cpp
#include <map>

std::map<uint64_t, uint64_t> latency_histogram;

// Bucket latency in microseconds
uint64_t bucket = latency_us / 10;  // 10μs buckets
latency_histogram[bucket]++;

// Print histogram
for (const auto& [bucket, count] : latency_histogram) {
    std::cout << (bucket * 10) << "μs: " << count << " samples\n";
}
```

### Network Monitoring

```bash
# Monitor UDP multicast traffic
sudo tcpdump -i any udp port 12345 -v

# Monitor TCP order flow
sudo tcpdump -i any tcp port 54321 -A

# Measure packet rates
iftop -i lo -f "port 12345 or port 54321"
```

---

## Testing Scenarios

### 1. Latency Test
- Generate market data at 10,000 msgs/sec
- Run algorithm with instant decision logic
- Measure end-to-end latency percentiles

### 2. Throughput Test
- Generate market data at maximum rate
- Measure maximum sustainable order rate
- Monitor CPU/memory under load

### 3. Risk Management Test
- Configure aggressive PnL limits
- Generate volatile market data
- Verify risk controls trigger correctly

### 4. Protocol Compatibility Test
- Run tests with OUCH protocol
- Run tests with Pillar protocol
- Run tests with iLink 3 protocol
- Verify identical behavior

### 5. Stress Test
- Multiple simultaneous algorithms
- Burst order submission
- High reject scenarios
- Network congestion simulation

---

## Configuration

### Market Data Generator
**File**: `src/apps/exchange_market_data_generator/sample_config.json`
```json
{
  "symbols": ["AAPL", "MSFT", "GOOGL"],
  "message_rate": 10000,
  "price_volatility": 0.1,
  "multicast_address": "239.255.0.1",
  "multicast_port": 12345
}
```

### Client Algorithm
**File**: `src/apps/client_algorithm/sample_config.json`
```json
{
  "market_data": {"host": "127.0.0.1", "port": 12345},
  "exchange": {"host": "127.0.0.1", "port": 54321},
  "trading": {"order_frequency": 1000},
  "risk_management": {
    "pnl_drawdown_warning": -5000.0,
    "order_reject_hard_stop": 50,
    "order_messaging_burst_hard_stop": 500
  },
  "products": [
    {
      "symbol": "AAPL",
      "algo_position_limit": 10000,
      "firm_position_limit": 15000
    }
  ]
}
```

---

## Troubleshooting

### No Market Data Received
```bash
# Check multicast group membership
netstat -g | grep 239.255.0.1

# Verify UDP traffic
sudo tcpdump -i any udp port 12345

# Check firewall rules
sudo iptables -L | grep 12345  # Linux
sudo pfctl -sr | grep 12345    # macOS
```

### Orders Not Executing
```bash
# Verify matching engine is running
netstat -an | grep 54321

# Check TCP connection
telnet 127.0.0.1 54321

# Review matching engine logs
tail -f matching_engine.log
```

### High Latency
```bash
# Check CPU frequency scaling
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Set performance mode
sudo cpupower frequency-set -g performance

# Disable SMT/Hyper-Threading
echo off | sudo tee /sys/devices/system/cpu/smt/control
```

### Risk Limits Triggering
```bash
# Review algorithm configuration
cat src/apps/client_algorithm/sample_config.json

# Check PnL calculation
# Monitor position tracking
# Verify reject counts
```

---

## Documentation

- **Protocol Specifications**:
  - [NASDAQ OUCH Protocol](docs/exchange_protocols/OUCH_PROTOCOL.md)
  - [NYSE Pillar Protocol](docs/exchange_protocols/PILLAR_PROTOCOL.md)
  - [CME iLink 3 Protocol](docs/exchange_protocols/CME_PROTOCOL.md)

- **Application Guides**:
  - [Market Data Generator](src/apps/exchange_market_data_generator/README.md)
  - [Matching Engine](src/apps/exchange_matching_engine/README.md)
  - [Client Algorithm Configuration](src/apps/client_algorithm/CONFIG.md)

- **Build System**:
  - [Build Process](docs/wiki/build/build_process.md)

---

## Architecture Highlights

### Lock-Free Design
- Market data processing: Lock-free ring buffers
- Order book updates: SPSC queues
- Message serialization: Zero-copy techniques

### Memory Efficiency
- Pre-allocated message buffers
- Object pooling for orders
- Cache-line aligned data structures

### Network Optimization
- UDP multicast for market data (one-to-many)
- TCP for reliable order transmission
- Non-blocking I/O throughout
- Kernel bypass options (DPDK, OpenOnload)

### Observability
- Built-in latency instrumentation
- Configurable logging levels
- Statistics collection
- Performance counters

---

## Performance Targets

| Metric | Target | Typical |
|--------|--------|---------|
| Market Data Processing | < 1 μs | 0.5 μs |
| Trading Decision | < 5 μs | 3 μs |
| Order Submission | < 10 μs | 7 μs |
| Order Matching | < 5 μs | 3 μs |
| Fill Processing | < 2 μs | 1 μs |
| **Total Round-Trip** | **< 30 μs** | **~20 μs** |
| Throughput (orders/sec) | > 100,000 | 150,000 |
| Market Data Rate | > 100,000 msgs/sec | 200,000+ |

---

## Future Enhancements

- [ ] FIX protocol support
- [ ] Historical data storage (timeseries DB)
- [ ] Web-based monitoring dashboard
- [ ] Machine learning integration
- [ ] Multi-venue routing
- [ ] Smart order routing (SOR)
- [ ] Advanced order types (iceberg, TWAP, VWAP)
- [ ] Real-time P&L analytics
- [ ] Compliance reporting

---

## License

MIT License - See LICENSE file for details

## Author

Bryan Camp

## Contributing

Contributions welcome! Please submit pull requests with:
- Comprehensive tests
- Documentation updates
- Performance benchmarks
