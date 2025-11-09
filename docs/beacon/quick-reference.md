# Beacon Trading System - Quick Reference

## Build Commands

### Build Everything
```bash
./build_all.sh
```

### Build Individual Apps
```bash
# Market Data Generator
cd src/apps/exchange_market_data_generator && ./build.sh

# Market Data Playback
cd src/apps/exchange_market_data_playback && ./build.sh

# Matching Engine
cd src/apps/exchange_matching_engine && ./build.sh

# Client Algorithm
cd src/apps/client_algorithm && ./build.sh
```

### CMake Build (all apps)
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

---

## Run Commands

### Full System (Automated)
```bash
# With market data generator
./run_system.sh generator

# With market data playback
./run_system.sh playback
```

### Manual Launch (3 terminals)

**Terminal 1: Matching Engine**
```bash
cd src/apps/exchange_matching_engine
./build/matching_engine
```

**Terminal 2: Market Data (choose one)**
```bash
# Generator
cd src/apps/exchange_market_data_generator
./run.sh

# OR Playback
cd src/apps/exchange_market_data_playback
./build/md_playback <file> 239.255.0.1 12345
```

**Terminal 3: Trading Algorithm**
```bash
cd src/apps/client_algorithm
./build/client_algo --config sample_config.json
```

---

## Configuration Files

### Market Data Generator
**Location**: `src/apps/exchange_market_data_generator/sample_config.json`

**Key Settings**:
- `symbols`: List of symbols to generate
- `message_rate`: Messages per second
- `protocol`: "NASDAQ", "NYSE", or "CME"
- `multicast_address`: UDP multicast address
- `multicast_port`: UDP multicast port

### Client Algorithm
**Location**: `src/apps/client_algorithm/sample_config.json`

**Key Settings**:
```json
{
  "market_data": {
    "host": "127.0.0.1",
    "port": 12345
  },
  "exchange": {
    "host": "127.0.0.1",
    "port": 54321
  },
  "risk_management": {
    "pnl_drawdown_hard_stop": -15000.0,
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

## Performance Monitoring

### Check Latency
```bash
# Add to any component
auto start = std::chrono::high_resolution_clock::now();
// ... operation ...
auto end = std::chrono::high_resolution_clock::now();
auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
```

### Monitor Network Traffic
```bash
# UDP multicast (market data)
sudo tcpdump -i any udp port 12345 -v

# TCP orders
sudo tcpdump -i any tcp port 54321 -v

# Packet rates
iftop -i lo -f "port 12345 or port 54321"
```

### System Performance
```bash
# CPU usage
top -p $(pgrep -d',' matching_engine)

# Profile with perf (Linux)
perf record -g ./build/matching_engine
perf report

# Profile with Instruments (macOS)
instruments -t "Time Profiler" ./build/matching_engine
```

---

## Network Configuration

### Multicast Setup (if needed)

**Linux**:
```bash
# Add multicast route
sudo route add -net 239.255.0.0 netmask 255.255.0.0 dev lo

# Check multicast groups
netstat -g
```

**macOS**:
```bash
# Add multicast route
sudo route add -net 239.255.0.0/16 127.0.0.1

# Check multicast groups
netstat -g
```

### Firewall (if needed)

**Linux**:
```bash
# Allow UDP port 12345
sudo iptables -A INPUT -p udp --dport 12345 -j ACCEPT

# Allow TCP port 54321
sudo iptables -A INPUT -p tcp --dport 54321 -j ACCEPT
```

**macOS**:
```bash
# Add firewall rules (if needed)
sudo pfctl -e
sudo pfctl -f /etc/pf.conf
```

---

## Protocol Selection

### NASDAQ OUCH
- Market data: NASDAQ ITCH 5.0
- Orders: OUCH 5.0
- Port: 54321

### NYSE Pillar
- Market data: NYSE Pillar
- Orders: Pillar Gateway 3.2
- Port: 54321

### CME iLink 3
- Market data: CME MDP 3.0
- Orders: iLink 3
- Port: 54321

**Note**: Matching engine auto-detects protocol based on incoming message format.

---

## Troubleshooting

### No Market Data
```bash
# Check if generator/playback is running
ps aux | grep md_generator

# Check multicast membership
netstat -g | grep 239.255.0.1

# Test UDP reception
nc -u -l 12345
```

### Orders Not Executing
```bash
# Check if matching engine is running
ps aux | grep matching_engine

# Test TCP connection
telnet 127.0.0.1 54321

# Check port binding
netstat -an | grep 54321
```

### High Latency
```bash
# Check CPU frequency
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Set performance mode
sudo cpupower frequency-set -g performance

# Disable CPU throttling
sudo sh -c "echo performance > /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor"
```

### Algorithm Not Trading
```bash
# Check configuration
cat src/apps/client_algorithm/sample_config.json

# Verify product is enabled
jq '.products[] | select(.enabled == true)' sample_config.json

# Check risk limits
jq '.risk_management' sample_config.json
```

---

## Testing Checklist

- [ ] Build all applications successfully
- [ ] Launch matching engine (port 54321 listening)
- [ ] Start market data source (UDP multicast)
- [ ] Start algorithm (connects to both)
- [ ] Verify market data reception
- [ ] Verify order submission
- [ ] Verify fill reception
- [ ] Check latency metrics
- [ ] Test risk limit triggers
- [ ] Test position limit enforcement
- [ ] Test protocol switching (OUCH, Pillar, iLink 3)

---

## Performance Targets

| Metric | Target |
|--------|--------|
| Market data processing | < 1 μs |
| Trading decision | < 5 μs |
| Order submission | < 10 μs |
| Order matching | < 5 μs |
| Fill processing | < 2 μs |
| **Total round-trip** | **< 30 μs** |
| Throughput | > 100,000 orders/sec |

---

## Common Use Cases

### Backtesting
```bash
# Use playback with historical data
cd src/apps/exchange_market_data_playback
./build/md_playback historical_data.pcap 239.255.0.1 12345
```

### Live Trading Simulation
```bash
# Use generator with realistic parameters
cd src/apps/exchange_market_data_generator
# Edit sample_config.json with real market params
./run.sh
```

### Latency Testing
```bash
# Run all components on same machine
# Use loopback interface (minimal network latency)
# Add instrumentation points
# Measure end-to-end latency
```

### Stress Testing
```bash
# Increase market data rate to maximum
# Launch multiple algorithm instances
# Monitor system resources
# Test under load
```

---

## Log Files

When using `run_system.sh`, logs are written to:
```
logs/
├── matching_engine_YYYYMMDD_HHMMSS.log
├── md_generator_YYYYMMDD_HHMMSS.log
└── client_algo_YYYYMMDD_HHMMSS.log
```

To monitor:
```bash
tail -f logs/matching_engine_*.log
tail -f logs/md_generator_*.log
tail -f logs/client_algo_*.log
```

---

## Emergency Shutdown

**Graceful**:
```bash
# If using run_system.sh
Ctrl+C (in run_system.sh terminal)

# Manual
pkill -SIGTERM matching_engine
pkill -SIGTERM md_generator
pkill -SIGTERM client_algo
```

**Force**:
```bash
pkill -9 matching_engine
pkill -9 md_generator
pkill -9 client_algo
```

---

## Documentation Links

- [Full System Documentation](README_SYSTEM.md)
- [NASDAQ OUCH Protocol](docs/exchange_protocols/OUCH_PROTOCOL.md)
- [NYSE Pillar Protocol](docs/exchange_protocols/PILLAR_PROTOCOL.md)
- [CME iLink 3 Protocol](docs/exchange_protocols/CME_PROTOCOL.md)
- [Matching Engine Guide](src/apps/exchange_matching_engine/README.md)
- [Algorithm Configuration](src/apps/client_algorithm/CONFIG.md)
- [Build Process](docs/wiki/build/build_process.md)

---

## Quick Commands Summary

```bash
# Build everything
./build_all.sh

# Run complete system
./run_system.sh generator

# Monitor logs
tail -f logs/*.log

# Check processes
ps aux | grep -E "matching_engine|md_generator|client_algo"

# Network monitoring
sudo tcpdump -i any 'port 12345 or port 54321'

# Performance check
top -p $(pgrep -d',' -f "matching_engine|md_generator|client_algo")
```
