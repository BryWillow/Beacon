# Beacon Trading System


**A high-performance algorithmic trading system built in modern C++20, with ~4 microsecond tick-to-trade latency.**

---

## ⚡ Technical Highlights

- **Languages:** C++20, Python (for scripting)
- **Build System:** CMake - **entire repo and invididual applications**
- **CI/CD:** GitHub Actions
- **Testing:** [GoogleTest](https://google.github.io/googletest/)
- **Networking:** UDP/TCP for all communication
- **Testing:** Unit tests written using Google Test framework
- **Configuration:** Intelligent defaults, easily overridden via `.json` config files
- **Protocol Rollbacks:** Supports protocol version rollbacks (price/orders independently) via simple config changes

---

## ⚡ System Requirements

- [Linux](https://www.linux.org/pages/download/) or MacOS
- C++ 20 Compiler
- [CMake](https://cmake.org/download/)
- [Python](https://www.python.org/downloads/) 

---

## 🚀 Quick Start





Setting up Beacon is ***super simple!*** Just do the following from a terminal:

```bash
git clone https://github.com/bryanlcamp/beacon.git
python3 beacon-build.py
python3 beacon-run.py
```

That's it! This downloads and builds the codebase, and runs Beacon with system defaults. Now you can see Beacon in action!

**→ [Customizing Beacon](docs/getting-started.md)**

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


**Beacon is a modular HFT trading system with four distinct executable parts, each of which can be run independently:**

### 1. Generator
- Creates binary data files containing exchange-formatted messages
- Control over total messages, symbols, exchanges, open (seed) price, bid-ask spread range, and trading frequency per symbol
- Downloads previous day’s close price and trading data for intelligent suggestions
- Generate unlimited datasets (binary files)

### 2. Playback
- Reads generator-created files into memory at startup
- Broadcasts real exchange packets via UDP, simulating a real exchange
- Flexible playback controls: burst frequency, circuit breakers, trading halts, message throttles, price overrides
- Pluggable rule system—implement your own interface
- Supports repeated playback of the same file

### 3. Client Algorithm
- Receives raw packets from playback via UDP
- Ultra-fast market-data handler: lock-free data structures, cache-aware padded structs, thread affinity, SPSC queue
- Hot path constantly polls SPSC queue (mm_pause), pinned to a core, no threading
- Built-in 3-tier risk checking (warning, alert, hard-stop): per-product position, messaging frequency, PnL
- Simple TWAP and VWAP algos provided; pluggable algo system—implement your own
- Sends order messages to matching engine via TCP using proper exchange protocol
- Handles execution reports; test your algorithm under diverse market conditions

### 4. Matching Engine
- Receives raw exchange-protocol order instructions from client_algo via TCP
- Simple FIFO matching engine, extensible for future enhancements
- Sends execution reports using proper exchange protocol back to client_algo via TCP

---

---

## 🎯 Key Features


✅ **Built in C++20 for modern performance and safety**
✅ **Proper exchange protocol support for NSDQ (ITCH/OUCH), NYSE (Pillar), CME (TotalView)**
✅ **UDP/TCP networking for all components**
✅ **Straightforward CMake build system**
✅ **Google Test framework for unit tests**
✅ **Intelligent defaults, easily overridden in .json config files**
✅ **Protocol version rollbacks via config**
✅ **Modular executables—run independently or together**
✅ **Pluggable rule and algo systems—extend via simple interfaces**
✅ **Ultra-fast market data handling: lock-free, cache-aware, thread affinity**
✅ **3-tier risk management built-in**
✅ **TWAP/VWAP sample algos included**
✅ **Comprehensive statistics and latency histograms**
✅ **Competitive HFT performance on a laptop!**

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
m
---

# Beacon Exchange Market Data Playback

## Overview
Beacon is a market data playback tool for NSDQ, NYSE, CME binary files. It supports configurable timing, burst, wave, chaos, and safety rules, with modular configuration via JSON.

## Configuration
- All configs are in `config/playback/`
- `default.json` references sender, advisor, and authority configs.
- **Senders:** UDP/TCP/Console, located in `senders/`
- **Advisors:** Classifiers (priority, burst), located in `advisors/`
- **Authorities:** Rules (burst, chaos, wave, safety), located in `authorities/`

## Modes
- **Continuous:** Normal rate playback
- **Burst:** Bursts of messages at intervals
- **Wave:** Variable rate playback
- **Chaos:** Packet loss, jitter, etc.

## Authorities (Rules)
- BurstRule, ContinuousRule, WaveRule, RateLimitRule, PacketLossRule, JitterRule, etc.

## Advisors (Classifiers)
- SymbolPriorityClassifier, BurstDetectionClassifier, etc.

## Usage
```
./exchange_market_data_playback --config config/playback/default.json <input_file>
./exchange_market_data_playback --summary <input_file>
```

## Test Cases
- Valid/invalid binary file parsing
- Config file existence and parsing
- UDP packet sending (rate, format, modes)
- Console output
- Placeholder for TCP

## Extending
Add new sender, advisor, or authority configs and update `default.json` to reference them.

## License
MIT

---

**Built with ❤️ for high-frequency trading**
