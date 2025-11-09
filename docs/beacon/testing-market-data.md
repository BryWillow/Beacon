# Market Data Testing Platform - Complete Guide

## 🎯 What This Platform Does

Generates realistic market data → Replays it with flexible timing → Receives it in your algo

Perfect for testing trading algorithms locally without connecting to real exchanges.

## ⚡ Quick Start (5 Minutes)

### 1. Generate Test Data
```bash
cd src/apps/exchange_market_data_generator
./build.sh
./build/exchange_market_data_generator --config cme_config.json
```

### 2. Test Reception (Choose One)

**Option A: TCP (Easiest - Always Works)**
```bash
# Terminal 1
cd src/apps/exchange_market_data_playback
./build.sh
./build/exchange_market_data_playback --config config_tcp.json ../exchange_market_data_generator/output.mdp

# Terminal 2  
cd src/apps/client_algorithm
./build.sh
./build/client_algorithm tcp 127.0.0.1 12345
```

**Option B: UDP Multicast (Production-Grade)**
```bash
# Terminal 1 - Start client FIRST
cd src/apps/client_algorithm
./build/client_algorithm udp 239.255.0.1 12345

# Terminal 2 - Then start server
cd src/apps/exchange_market_data_playback
./build/exchange_market_data_playback --config config_udp_slow.json ../exchange_market_data_generator/output.mdp
```

✅ **Both work on localhost!** (TCP is more reliable for testing)

## 📦 The Three Applications

### 1. exchange_market_data_generator
**Creates realistic binary market data**

- **Protocols**: NASDAQ ITCH, NYSE Pillar, CME MDP
- **Output**: Binary files (32-byte messages)
- **Features**: Realistic spreads, price walks, configurable symbols
- **Docs**: `src/apps/exchange_market_data_generator/README.md`

**Example Output**:
```
Generated 10000 messages for 3 symbols
ESZ4 | Avg Bid: $5245.44 | Avg Ask: $5245.50 | Spread: 0.11%
```

### 2. exchange_market_data_playback
**Replays data with configurable timing**

- **TCP Output**: Localhost-friendly, ordered delivery ✅
- **UDP Multicast**: Production-grade, works on localhost ✅
- **Rules**: Burst patterns, rate limits, packet loss, jitter
- **Speed Control**: 0.1x (slow-mo) to 100x (time compression)
- **Docs**: `src/apps/exchange_market_data_playback/README.md`

**Example Output**:
```
Progress: 20000/20000 (100.0%) | Rate: 7291 msg/s | Sent: 20000
Duration: 2.7s | Average Rate: 7291 msg/s
```

### 3. client_algorithm
**Test receiver / your algo foundation**

- **TCP Mode**: Connect to TCP server
- **UDP Mode**: Join multicast group
- **Features**: Progress tracking, statistics, hex dumps
- **Docs**: `src/apps/client_algorithm/README.md`

**Example Output**:
```
✓ Connected successfully!
Received 1000 messages (32000 bytes)
Received 2000 messages (64000 bytes)
...
Total messages: 20000 | Total bytes: 640000
```

## 🔧 The UDP Multicast Fix (IMPORTANT!)

### The Problem
macOS doesn't loop back multicast on localhost by default.

### The Solution
Two critical socket options in the playback app:

```cpp
// 1. Enable loopback (packets come back to same machine)
unsigned char loopback = 1;
setsockopt(socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loopback, sizeof(loopback));

// 2. Use default interface (macOS routes via en0, not lo0)
in_addr localInterface;
localInterface.s_addr = INADDR_ANY;  // KEY: Use INADDR_ANY, not 127.0.0.1!
setsockopt(socket, IPPROTO_IP, IP_MULTICAST_IF, &localInterface, sizeof(localInterface));
```

### Why This Works
- macOS routes multicast through your network interface (`en0`)
- `INADDR_ANY` lets OS choose the default interface
- `IP_MULTICAST_LOOP=1` enables loopback to same machine
- Flow: app → en0 → loop back → app ✅

**Verify it's working**: `netstat -rn | grep 224` should show routes via `en0`

## 📋 Complete Testing Workflow

### Step 1: Generate Data
```bash
cd src/apps/exchange_market_data_generator
./build/exchange_market_data_generator --config cme_config.json
```
Output: `output.mdp` (CME futures: ESZ4, NQZ4, YMZ4)

### Step 2: Choose Testing Mode

**For Debugging (TCP)**:
- ✅ Reliable, ordered
- ✅ No packet loss
- ✅ Perfect for development

```bash
# Terminal 1
./build/exchange_market_data_playback --config config_tcp.json output.mdp

# Terminal 2
./build/client_algorithm tcp 127.0.0.1 12345
```

**For Production Testing (UDP)**:
- ✅ Realistic (like real exchanges)
- ✅ Tests packet loss handling
- ✅ Multicast support

```bash
# Terminal 1 - CLIENT FIRST!
./build/client_algorithm udp 239.255.0.1 12345

# Terminal 2
./build/exchange_market_data_playback --config config_udp_slow.json output.mdp
```

### Step 3: Build Your Algo

Extend `client_algorithm/main.cpp`:
```cpp
// Current code shows how to receive messages
// Add your logic:
// 1. Parse message format (ITCH/Pillar/MDP)
// 2. Maintain order book
// 3. Calculate signals
// 4. Execute orders
```

## 🎛️ Configuration Examples

### Slow Feed (Debugging)
```json
{
  "output": "tcp",
  "tcp_port": 12345,
  "mode": "continuous",
  "rate_msgs_per_sec": 1000,
  "speed_factor": 0.1
}
```

### Fast Burst (Stress Test)
```json
{
  "output": "udp_multicast",
  "multicast_address": "239.255.0.1",
  "multicast_port": 12345,
  "mode": "burst",
  "burst_size": 10000,
  "burst_interval_ms": 50,
  "speed_factor": 10.0
}
```

### Chaos Testing
```json
{
  "mode": "continuous",
  "rate_msgs_per_sec": 50000,
  "packet_loss_rate": 0.05,
  "max_jitter_us": 5000
}
```

## 🔍 Troubleshooting

### TCP: Connection Refused
**Fix**: Start server BEFORE client
```bash
./build/exchange_market_data_playback --config config_tcp.json output.itch  # 1. Server
./build/client_algorithm tcp 127.0.0.1 12345                                # 2. Client
```

### UDP: No Messages Received
**Check**:
1. ✅ Client started BEFORE server?
2. ✅ Using rebuilt playback (with loopback fix)?
3. ✅ Multicast routes via en0? (`netstat -rn | grep 224`)

**Fix**: Kill old clients and restart in correct order
```bash
pkill client_algorithm
./build/client_algorithm udp 239.255.0.1 12345  # Start first!
```

### "No Buffer Space Available"
**Fix**: Reduce rate or increase buffers
```json
{"rate_msgs_per_sec": 5000}  // Lower rate
```

Or increase OS buffers:
```bash
# macOS
sudo sysctl -w kern.ipc.maxsockbuf=8388608
sudo sysctl -w net.inet.udp.recvspace=2097152
```

### Receiving More Messages Than Sent
**Cause**: Old client still running from previous test

**Fix**: Kill all clients before testing
```bash
pkill client_algorithm
```

## 📊 Message Format

All protocols use **32-byte** binary messages:

```
Bytes 0-3:    Message type
Bytes 4-7:    Timestamp/Sequence
Bytes 8-15:   Symbol (8 chars, null-padded)
Bytes 16-31:  Protocol-specific data
```

**Example (CME ESZ4)**:
```
01 00 00 00  00 00 00 00  01 00 00 00  45 53 5a 34  ...
│           │           │           │
Type=1      TS=0        Seq=1       Symbol="ESZ4"
```

## 📁 File Structure

```
experimental/
├── MARKET_DATA_TESTING_GUIDE.md (this file)
└── src/apps/
    ├── exchange_market_data_generator/
    │   ├── README.md
    │   ├── build.sh
    │   ├── sample_config.json    (NASDAQ)
    │   ├── cme_config.json       (CME Futures)
    │   └── pillar_config.json    (NYSE)
    │
    ├── exchange_market_data_playback/
    │   ├── README.md
    │   ├── ARCHITECTURE.md
    │   ├── build.sh
    │   ├── config_tcp.json       (TCP output)
    │   ├── config_udp_slow.json  (UDP at 10K msg/s)
    │   ├── config_burst.json     (Burst pattern)
    │   └── config_chaos.json     (Packet loss + jitter)
    │
    └── client_algorithm/
        ├── README.md
        ├── build.sh
        └── main.cpp              (TCP + UDP receiver)
```

## 🚀 Next Steps

### For Testing:
1. **Try all protocols**: NASDAQ, NYSE, CME
2. **Test burst modes**: Simulate market opens
3. **Add chaos**: Packet loss, jitter
4. **Time compression**: Replay hours in minutes

### For Development:
1. **Parse messages**: Add protocol-specific parsers
2. **Build order book**: Track bids/asks
3. **Add strategy**: Your signal logic
4. **Test at scale**: High rates, long runs

### For Production:
1. **UDP multicast**: Connect to real exchange feeds
2. **Capture data**: Save production feeds for replay
3. **Backtest**: Replay historical data
4. **Go live**: Shadow mode with real feeds

## 🎓 Key Concepts

### TCP vs UDP for Testing

| Feature | TCP | UDP Multicast |
|---------|-----|---------------|
| Reliability | 100% delivery | May drop packets |
| Order | Always ordered | Ordered within flow |
| Localhost | ✅ Always works | ✅ Works (with fix) |
| Production-like | ❌ Not realistic | ✅ Like real exchanges |
| Debugging | ✅ Easier | ❌ Harder |
| **Recommendation** | Use for development | Use for final testing |

### Rules Engine

Playback uses a priority-based rules engine:

```
Priority 0 (SAFETY)  → Rate limits (never bypassed)
Priority 1 (CONTROL) → Burst patterns, flow control
Priority 2 (TIMING)  → Speed factor, delays
Priority 3 (CHAOS)   → Packet loss, jitter
```

Rules compose: `Burst + Speed Factor + Rate Limit` all work together.

### Speed Factor

Compress or expand time:
- `0.1x` - Slow motion (1 second becomes 10 seconds)
- `1.0x` - Real time
- `10x` - Fast forward (1 second becomes 100ms)
- `100x` - Time compression (hours → minutes)

## 📚 Additional Documentation

- **Generator**: `src/apps/exchange_market_data_generator/README.md`
- **Playback**: `src/apps/exchange_market_data_playback/README.md`
- **Playback Architecture**: `src/apps/exchange_market_data_playback/ARCHITECTURE.md`
- **Priority System**: `src/apps/exchange_market_data_playback/PRIORITY_SYSTEM.md`
- **Client**: `src/apps/client_algorithm/README.md`

## ✅ System Status

- **Generator**: ✅ Production-ready (NASDAQ, NYSE, CME)
- **Playback**: ✅ Production-ready (TCP, UDP, File)
- **Client**: ✅ Ready for algo development
- **UDP Localhost**: ✅ Working (with loopback fix)
- **Documentation**: ✅ Complete

**You're ready to build and test your trading algorithms!** 🎉

---

**Project**: Beacon  
**Module**: Experimental - Market Data Testing Platform  
**Author**: Bryan Camp  
**Last Updated**: November 5, 2025
