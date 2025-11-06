# 🚀 Quick Start - Beacon Trading System

## Run the System (Easy Mode)

```bash
# From the project root:
python3 scripts/beacon-run.py 30
```

That's it! The system will:
1. Start matching engine
2. Start your algorithm  
3. Play market data
4. Show statistics

**See `scripts/README.md` for all available scripts.**

---

## Directory Organization

```
experimental/
├── scripts/                       # ← ALL SYSTEM SCRIPTS HERE (Python)
│   ├── beacon-run.py             # ← Main entry point
│   ├── beacon-previous-prices.py # Fetch previous day's prices
│   ├── beacon-build.py           # Build everything
│   ├── beacon-kill.py            # Stop all processes
│   ├── beacon-test-udp.py        # Test UDP multicast
│   └── README.md                 # Detailed script documentation
│
├── src/apps/                   # Application code
│   ├── exchange_matching_engine/
│   │   └── build.sh           # Build just this component
│   ├── exchange_market_data_generator/
│   │   ├── build.sh
│   │   └── run.sh             # Generate market data file
│   ├── exchange_market_data_playback/
│   │   ├── build.sh
│   │   └── run.sh             # Play market data
│   └── client_algorithm/
│       └── build.sh           # Build algorithms
│
├── include/                    # Header files
├── docs/                       # Documentation
└── README.md                   # This file
```

**Key Points:**
- ✅ **System scripts** → `scripts/` directory (all use `beacon-` prefix)
- ✅ **Per-app build/run** → Stay in each app's directory
- ✅ **One command** → `python3 scripts/beacon-run.py`

---

## First Time Setup

```bash
# 1. Build everything
cd scripts
python3 beacon-build.py

# 2. Generate market data (one time)
cd ../src/apps/exchange_market_data_generator
./build.sh
./run.sh

# 3. Run the system!
cd ../../..
python3 scripts/beacon-run.py 30
```

---

## Daily Workflow

```bash
# Optional: Fetch previous day's prices
python3 scripts/beacon-previous-prices.py

# Run the system
python3 scripts/beacon-run.py 60
```

---

## Troubleshooting

```bash
# Kill stuck processes
python3 scripts/beacon-kill.py

# Rebuild everything
python3 scripts/beacon-build.py

# Test UDP multicast
python3 scripts/beacon-test-udp.py 239.255.0.1 12345
```

---

## Documentation

- **📂 Script Details:** `scripts/README.md` ← **Start here for all scripts**
- **📘 System Architecture:** `README_SYSTEM.md`
- **📖 Quick Reference:** `QUICK_REFERENCE.md`
- **🧪 Testing Guide:** `MARKET_DATA_TESTING_GUIDE.md`

---

## Key Metrics (From Last Run)

```
Market Data Received:  95,425
Orders Sent:           954
Fills Received:        1,584

Tick-to-Trade Latency:
  Mean:     5.188 μs    ← Average response time
  p99:      18.041 μs   ← 99th percentile
```

**5 μs mean latency on a MacBook!** That's competitive HFT performance. 🚀

---

## What's Next?

1. **Understand the system:** Read `scripts/README.md`
2. **Customize your algorithm:** Edit `src/apps/client_algorithm/algo_template.cpp`
3. **Test changes:** `python3 scripts/beacon-run.py 30`
4. **Get real prices:** `python3 scripts/beacon-previous-prices.py`

---

**Questions? Check `scripts/README.md` for detailed documentation of every script.**
