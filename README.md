# Beacon Trading System

**A high-performance algorithmic trading system with ~4 microsecond tick-to-trade latency.**

## 🚀 Quick Start

```bash
# Run the complete system
python3 scripts/beacon-run.py 30
```

That's it! The system will start the matching engine, your algorithm, and market data playback.

**→ [Full Getting Started Guide](docs/getting-started.md)** *(if available)*

---

## 📖 Documentation

**Documentation is organized in the [`docs/`](docs/) directory**

- **[Getting Started](docs/getting-started.md)** - Quick start guide *(if available)*
- **[Architecture](docs/architecture.md)** - System design and components *(if available)*
- **[Scripts](scripts/README.md)** - All `beacon-*` script documentation *(if available)*
- **[Testing](tests/README.md)** - GoogleTest suite documentation *(if available)*

**→ [Documentation Index](docs/README.md)** *(if available)*

---

## 🏗️ Architecture

**Beacon is a modular HFT trading system with:**

- **Matching Engine** - OUCH protocol order matching
- **Market Data Generator** - Creates realistic ITCH market data
- **Market Data Playback** - UDP multicast streaming
- **Algorithm Framework** - Low-latency trading strategies

**Key Technologies:**
- Lock-free ring buffers (SPSC)
- Thread pinning for deterministic performance
- UDP multicast for market data
- TCP for order entry
- Professional Python orchestration

---

## 🎯 Key Features

✅ **~4-5 μs mean latency** (tick-to-trade on MacBook)  
✅ **Professional Python scripts** with `beacon-` prefix  
✅ **One command to run** the entire system  
✅ **Real market data** from previous day's prices  
✅ **Comprehensive statistics** with latency histograms  
✅ **Clean architecture** with clear separation of concerns  

---

## 📊 Performance

**Last Run (MacBook Air):**
```
Market Data Received:  97,252
Orders Sent:           972
Fills Received:        1,586

Tick-to-Trade Latency:
  Mean:     4.621 μs
  Median:   4.000 μs
  p95:      7.709 μs
  p99:      21.500 μs
  Max:      56.541 μs
```

**Competitive HFT performance on a laptop!**

---

## 🔧 Scripts

All system scripts are in the `scripts/` directory with consistent `beacon-` naming:

```bash
beacon-run.py              # Run the trading system
beacon-build.py            # Build all components
beacon-kill.py             # Stop all processes
beacon-previous-prices.py  # Fetch yesterday's market prices
beacon-test-udp.py         # Test UDP multicast
```

**→ [Full Scripts Documentation](scripts/README.md)**

---

## 📂 Project Structure

```
Beacon/
├── scripts/                   # All beacon-* scripts
├── docs/                     # Documentation
├── src/                      # Source code
│   ├── apps/                # Applications
│   │   ├── exchange_matching_engine/
│   │   ├── exchange_market_data_generator/
│   │   ├── exchange_market_data_playback/
│   │   └── client_algorithm/     # Your trading algorithms
│   └── libs/                # Protocol libraries
│       └── exchange_protocols/   # ITCH, Pillar, MDP, etc.
├── include/                  # Core HFT libraries
│   ├── networking/          # UDP/TCP networking
│   ├── ringbuffer/          # Lock-free data structures
│   └── concurrency/         # Thread pinning, etc.
├── tests/                    # GoogleTest suite
│   ├── core/                # Core component tests
│   ├── protocols/           # Protocol validation tests
│   ├── matching_engine/     # Order matching tests
│   └── integration/         # End-to-end tests
└── vendor/                   # Third-party dependencies
    ├── googletest/          # Testing framework
    └── nlohmann/            # JSON library
```

---

## 🎓 Why Beacon?

The word "Beacon" represents a **guiding light** - a source of clarity for solving complex challenges. This project demonstrates:

- **Low-latency C++ design patterns**
- **Lock-free concurrent programming**
- **High-frequency trading architecture**
- **Professional software engineering practices**

Built as both a **learning platform** and a **portfolio showcase** of production-quality HFT systems.

---

## 🤝 Getting Help

- **Documentation:** Check [`docs/beacon/`](docs/beacon/)
- **Scripts:** See [`scripts/README.md`](scripts/README.md)
- **App-specific:** Each app has its own README in `src/apps/*/`

---

## 📝 License

This is a personal portfolio project.

---

## Build Scripts
- Debug build: `beacon_build/beacon-build-debug.py`
- Release build: `beacon_build/beacon-build-release.py`

## Release Scripts
- Create release: `beacon_release_version/beacon-create-release.py`
- Package release: `beacon_release_version/beacon-package-release.py`
- Release notes: `beacon_release_version/beacon-release-notes.py`

## Run Scripts
- Run system: `beacon_run/run_system.py`
- Beacon run: `beacon_run/beacon-run.py`

## Utility Scripts
- Kill: `beacon_utils/beacon-kill.py`
- Previous prices: `beacon_utils/beacon-previous-prices.py`
- Test UDP: `beacon_utils/beacon-test-udp.py`

## Logs
- All build logs: `logs/`

---

**Built with ❤️ for high-frequency trading**
