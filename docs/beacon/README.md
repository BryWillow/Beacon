# Beacon Trading System - Documentation

**Complete documentation for the Beacon high-frequency trading system.**

## 📚 Documentation Index

### 🚀 [Getting Started](getting-started.md)
**Start here!** Quick start guide to get the system running in minutes.
- First time setup
- Daily workflow  
- Troubleshooting
- One command to run everything

### 🏗️ [Architecture](architecture.md)
Deep dive into the system architecture and design.
- Component overview
- Threading model
- Network architecture
- Performance characteristics
- Design decisions

### ⚡ [Quick Reference](quick-reference.md)
Command cheatsheet and common operations.
- All beacon scripts
- Build commands
- Run commands
- Debug commands

### 🧪 [Testing Market Data](testing-market-data.md)
Guide to testing market data reception and playback.
- UDP multicast testing
- Market data generator
- Playback configuration
- Debugging reception issues

### 🔧 [Scripts Documentation](../../scripts/README.md)
Complete reference for all `beacon-*` scripts.
- beacon-run.py
- beacon-build.py
- beacon-kill.py
- beacon-previous-prices.py
- beacon-test-udp.py

---

## 📂 Additional Documentation

### Application-Specific Docs
Each application has its own README in its directory:

- **Matching Engine:** `src/apps/exchange_matching_engine/README.md`
- **Market Data Generator:** `src/apps/exchange_market_data_generator/README.md`
- **Market Data Playback:** `src/apps/exchange_market_data_playback/README.md`
- **Client Algorithms:** `src/apps/client_algorithm/README.md`

### Exchange Protocol Documentation
Protocol specifications and implementation guides:

- **CME:** `docs/exchange_protocols/cme/`
- **NASDAQ:** `docs/exchange_protocols/nsdq/`
- **NYSE:** `docs/exchange_protocols/nyse/`
- **ICE:** `docs/exchange_protocols/ice/`

---

## 🎯 Quick Navigation

**I want to...**

- **Run the system** → [Getting Started](getting-started.md)
- **Understand the architecture** → [Architecture](architecture.md)
- **Find a command** → [Quick Reference](quick-reference.md)
- **Debug market data** → [Testing Market Data](testing-market-data.md)
- **Learn about scripts** → [Scripts](../../scripts/README.md)
- **Modify an algorithm** → `src/apps/client_algorithm/README.md`

---

## 📊 Key Metrics

**System Performance (MacBook):**
```
Mean Latency:     ~4-5 μs     (FAST tier)
p99 Latency:      ~12-20 μs   (competitive)
Market Data:      95,000+ msgs/sec
Orders:           ~1,000/second
Fills:            ~1,500/second
```

**Technology Stack:**
- **Language:** C++17
- **Build:** CMake
- **Threading:** Lock-free ring buffers, core pinning
- **Networking:** UDP multicast, TCP for orders
- **Orchestration:** Python scripts
- **Platform:** Cross-platform (macOS, Linux)

---

## 🤝 Contributing

When adding new documentation:

1. Place in `docs/beacon/` directory
2. Use lowercase with hyphens: `my-new-doc.md`
3. Add to this index
4. Keep it concise and actionable

---

**Last Updated:** November 6, 2025
