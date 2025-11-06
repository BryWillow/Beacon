# TWAP Algorithm - Reference Implementation

## Overview

**TWAP (Time-Weighted Average Price)** is an industry-standard execution algorithm used by institutional traders to execute large orders with minimal market impact.

### What is TWAP?

TWAP splits a large "parent" order into many smaller "child" orders that are executed evenly over a specified time window.

**Goal:** Minimize market impact by spreading execution over time  
**Use Case:** Executing large institutional orders without moving the market

## Algorithm Logic

```
1. Divide time window into N equal slices
   Example: 60 seconds ÷ 1000ms = 60 slices

2. Calculate shares per slice
   Example: 10,000 shares ÷ 60 slices = ~167 shares/slice

3. Every slice interval, send an order at current market price
   
4. Track fills and adjust for partial executions

5. Stop when: target reached OR time window complete
```

## Performance Architecture

This implementation uses production HFT patterns:

- **Lock-free SPSC queues** for inter-thread communication
- **Thread pinning** to dedicated cores (Linux)
- **Microsecond-precision timing** for accurate slicing
- **Zero-copy** market data path
- **Atomic counters** for thread-safe state

### Threading Model:
- **Core 0**: Market data receiver (UDP multicast, minimal latency)
- **Core 1**: TWAP logic (time-slicing, order generation)
- **Core 2**: Execution report receiver (TCP, blocking push)

## Usage

```bash
./build/algo_twap <symbol> <side> <shares> <duration_sec> <slice_ms> \
                  <mcast_addr> <md_port> <ex_host> <ex_port>
```

### Parameters:

| Parameter | Description | Example |
|-----------|-------------|---------|
| `symbol` | Stock symbol (8 chars) | `AAPL` |
| `side` | Buy ('B') or Sell ('S') | `B` |
| `shares` | Total shares to execute | `10000` |
| `duration_sec` | Execution window (seconds) | `60` |
| `slice_ms` | Time between orders (milliseconds) | `1000` |
| `mcast_addr` | Market data multicast address | `239.255.0.1` |
| `md_port` | Market data port | `12345` |
| `ex_host` | Exchange host for order entry | `127.0.0.1` |
| `ex_port` | Exchange port for order entry | `9000` |

## Examples

### Example 1: Buy 10,000 shares of AAPL over 60 seconds

```bash
./build/algo_twap AAPL B 10000 60 1000 239.255.0.1 12345 127.0.0.1 9000
```

**What happens:**
- 60 slices (one every 1000ms = 1 second)
- ~167 shares per slice
- Total execution time: 60 seconds

### Example 2: Sell 5,000 shares of MSFT over 30 seconds (fast execution)

```bash
./build/algo_twap MSFT S 5000 30 500 239.255.0.1 12345 127.0.0.1 9000
```

**What happens:**
- 60 slices (one every 500ms)
- ~83 shares per slice
- More aggressive (2 orders per second)

### Example 3: Buy 100,000 shares over 10 minutes (slow, stealthy)

```bash
./build/algo_twap AAPL B 100000 600 5000 239.255.0.1 12345 127.0.0.1 9000
```

**What happens:**
- 120 slices (one every 5000ms = 5 seconds)
- ~833 shares per slice
- Very low market impact

## Sample Output

```
=============================================================================
  TWAP ALGORITHM CONFIGURATION
=============================================================================
Symbol:           AAPL
Side:             BUY
Total Shares:     10000
Duration:         60 seconds
Slice Interval:   1000 ms
Max Slice Size:   1000 shares
Calculated:
  Number of slices:     60
  Shares per slice:     166
  Orders per second:    1
=============================================================================

[INIT] Connecting to market data 239.255.0.1:12345...
[INIT] ✓ Market data connected
[INIT] Connecting to exchange 127.0.0.1:9000...
[INIT] ✓ Exchange connected

[Core 0] Market data receiver started
[Core 2] Execution report receiver started
[Core 1] TWAP logic thread started
[Core 1] TWAP starting execution:
         60 slices of 166 shares each
         Slice every 1000 ms

[FILL] Qty=166 Price=$145.2300 Total=166/10000
[FILL] Qty=166 Price=$145.2400 Total=332/10000
[FILL] Qty=166 Price=$145.2350 Total=498/10000
...
[FILL] Qty=166 Price=$145.2500 Total=9834/10000
[FILL] Qty=166 Price=$145.2450 Total=10000/10000

[TWAP] Target shares executed!

=============================================================================
                        TWAP EXECUTION SUMMARY
=============================================================================
Target Shares:     10000
Shares Executed:   10000 (100.00%)
Shares Outstanding:0
Orders Sent:       60
Fills Received:    60
Order Rejects:     0
Avg Fill Price:    $145.2389
Execution Time:    60123 ms
=============================================================================
```

## Testing with Beacon System

### Step 1: Generate Market Data

```bash
cd ../exchange_market_data_generator
./build/exchange_market_data_generator config.json output.mdp
```

### Step 2: Start Market Data Playback (UDP)

```bash
cd ../exchange_market_data_playback
./build/exchange_market_data_playback --config config_udp_slow.json \
                                       ../exchange_market_data_generator/output.mdp
```

### Step 3: Start Matching Engine

```bash
cd ../exchange_matching_engine
./build/exchange_matching_engine 9000
```

### Step 4: Run TWAP Algorithm

```bash
cd ../client_algorithm
./build/algo_twap AAPL B 10000 60 1000 239.255.0.1 12345 127.0.0.1 9000
```

## TWAP vs Other Strategies

| Strategy | Description | When to Use |
|----------|-------------|-------------|
| **TWAP** | Even time distribution | When you don't know volume patterns |
| **VWAP** | Volume-weighted distribution | When you want to match market rhythm |
| **POV** | Percent of volume | When you want fixed % of market |
| **Implementation Shortfall** | Minimize cost vs arrival | When speed matters |

## Key Metrics

The algorithm tracks:
- **Fill rate**: What % of orders fill completely?
- **Average fill price**: vs TWAP benchmark
- **Execution time**: Did we finish on schedule?
- **Market impact**: How much did price move during execution?

## Advantages of TWAP

✅ **Simple** - Easy to understand and explain  
✅ **Predictable** - Execution schedule is deterministic  
✅ **Low impact** - Spreads order flow over time  
✅ **Benchmark** - Industry-standard performance metric  

## Disadvantages of TWAP

❌ **Ignores volume** - Doesn't adapt to market activity  
❌ **Predictable** - Sophisticated traders can detect pattern  
❌ **Inflexible** - Doesn't respond to price moves  

## Code Structure

```cpp
// Configuration
struct TWAPConfig {
    char symbol[8];
    uint32_t totalShares;
    uint32_t durationSeconds;
    char side;
    uint32_t sliceIntervalMs;
    uint32_t maxSliceSize;
};

// State tracking
struct TWAPState {
    std::atomic<uint32_t> sharesExecuted;
    std::atomic<uint32_t> sharesOutstanding;
    std::atomic<uint32_t> ordersSent;
    std::atomic<uint32_t> fillsReceived;
    // ... timing, pricing, etc.
};

// Core TWAP logic (simplified)
while (now < endTime && sharesExecuted < target) {
    if (now >= nextSliceTime) {
        uint32_t sliceSize = calculateSliceSize();
        uint32_t price = getCurrentMarketPrice();
        sendOrder(symbol, sliceSize, price, side);
        nextSliceTime += sliceInterval;
    }
    processExecutionReports();
}
```

## Performance Tuning

### For Low Latency:
- Use smaller slice intervals (100-500ms)
- Enable thread pinning on Linux
- Use IOC (Immediate-or-Cancel) orders

### For Stealth:
- Use longer slice intervals (5-30 seconds)
- Randomize order sizes slightly
- Add jitter to slice timing

### For Large Orders:
- Increase execution window
- Use smaller slices
- Monitor market impact

## Extensions

This TWAP implementation can be extended with:

1. **Adaptive sizing** - Adjust slice size based on fills
2. **Price limits** - Don't execute if price moves too far
3. **Randomization** - Add jitter to timing and sizing
4. **Liquidity detection** - Skip slices in thin markets
5. **Participation rate** - Cap at % of market volume

## References

- **"Optimal Execution of Portfolio Transactions"** - Almgren & Chriss (2000)
- **"Trading and Exchanges"** - Harris (2003)
- **"Algorithmic Trading & DMA"** - Johnson (2010)

---

**Project**: Beacon  
**Component**: Reference Algorithms  
**Author**: Bryan Camp  
**Strategy**: TWAP (Time-Weighted Average Price)
