# VWAP Algorithm - Reference Implementation

## Overview

**VWAP (Volume-Weighted Average Price)** is a more sophisticated execution algorithm that distributes orders proportionally to expected market volume patterns.

### What is VWAP?

Unlike TWAP (which executes evenly over time), VWAP executes more during high-volume periods and less during low-volume periods, matching the market's natural rhythm.

**Goal:** Match or beat the volume-weighted average price benchmark  
**Use Case:** Large institutional orders where you want to "blend in" with market flow

## Key Difference from TWAP

| Aspect | TWAP | VWAP |
|--------|------|------|
| **Distribution** | Even over time | Proportional to volume |
| **Market awareness** | Ignores volume | Follows volume patterns |
| **Complexity** | Simple | Moderate (needs volume profile) |
| **Stealth** | Predictable pattern | Blends with market |

## Algorithm Logic

```
1. Load historical volume profile for the symbol
   Example: [10%, 15%, 20%, 25%, 20%, 10%] for each 10-minute bucket

2. Calculate target shares for each time bucket
   Example: 10,000 shares × 25% = 2,500 shares in high-volume hour

3. Within each bucket, distribute proportionally
   
4. Track cumulative volume vs target participation rate

5. Adjust dynamically if market volume differs from historical
```

## Volume Profile

VWAP requires a **volume profile** - historical data showing when volume typically occurs:

### Typical Intraday Volume Pattern (U.S. Equities):

```
Hour    | Volume % | Description
--------|----------|-------------
 9:30   |   25%    | Opening surge
10:00   |   15%    | Morning activity  
11:00   |   10%    | Mid-morning lull
12:00   |    8%    | Lunch doldrums
 1:00   |    8%    | Afternoon quiet
 2:00   |   10%    | Late day pickup
 3:00   |   14%    | Closing ramp
 4:00   |   10%    | Final push
```

### Sample Volume Profile (Simplified):

For demonstration, our implementation uses a simplified 6-bucket profile representing typical patterns.

## Usage

```bash
# Note: Full VWAP implementation requires volume profile data
# This is a placeholder showing the architecture

./build/algo_vwap <symbol> <side> <shares> <duration_sec> \
                  <participation_rate> <mcast_addr> <md_port> <ex_host> <ex_port>
```

### Parameters:

| Parameter | Description | Example |
|-----------|-------------|---------|
| `symbol` | Stock symbol | `AAPL` |
| `side` | Buy ('B') or Sell ('S') | `B` |
| `shares` | Total shares | `100000` |
| `duration_sec` | Execution window | `3600` (1 hour) |
| `participation_rate` | Target % of market volume | `0.10` (10%) |
| `mcast_addr` | Market data address | `239.255.0.1` |
| `md_port` | Market data port | `12345` |
| `ex_host` | Exchange host | `127.0.0.1` |
| `ex_port` | Exchange port | `9000` |

## Example

```bash
# Execute 100,000 shares over 1 hour, targeting 10% of market volume
./build/algo_vwap AAPL B 100000 3600 0.10 239.255.0.1 12345 127.0.0.1 9000
```

## VWAP Calculation

The VWAP benchmark is calculated as:

```
VWAP = Σ(Price × Volume) / Σ(Volume)
```

**Example:**
```
Trade 1: 100 shares @ $150.00 = $15,000
Trade 2: 200 shares @ $150.50 = $30,100
Trade 3: 150 shares @ $149.75 = $22,462.50

VWAP = ($15,000 + $30,100 + $22,462.50) / (100 + 200 + 150)
     = $67,562.50 / 450
     = $150.14
```

## Advantages of VWAP

✅ **Market-aware** - Adapts to volume patterns  
✅ **Stealth** - Blends with market flow  
✅ **Benchmark** - Industry standard for performance  
✅ **Reduces impact** - Executes when market is most liquid  

## Disadvantages of VWAP

❌ **Requires data** - Needs historical volume profiles  
❌ **Backward-looking** - Uses historical, not real-time volume  
❌ **Complex** - More sophisticated than TWAP  
❌ **Profile risk** - If volume pattern changes, can miss target  

## Implementation Status

**Note:** Full VWAP implementation requires:
1. Historical volume profile data
2. Real-time volume tracking
3. Adaptive participation rate logic
4. Bucket-based execution scheduling

This would add significant complexity. The current Beacon system provides TWAP as a complete reference implementation. VWAP can be added as an extension once volume profiling infrastructure is in place.

## Recommended Approach for VWAP

To implement full VWAP in Beacon:

### Phase 1: Volume Profile Builder
```cpp
// Analyze historical market data
// Output: volume_profile_AAPL.json
{
    "symbol": "AAPL",
    "buckets": [
        {"time": "09:30", "volume_pct": 0.25},
        {"time": "10:00", "volume_pct": 0.15},
        ...
    ]
}
```

### Phase 2: Real-time Volume Tracker
```cpp
// Track cumulative volume in real-time
struct VolumeTracker {
    uint64_t totalVolume;
    uint64_t bucketVolume;
    double participationRate;
};
```

### Phase 3: Adaptive VWAP Logic
```cpp
// Adjust execution rate based on realized volume
if (actualVolume > expectedVolume) {
    // Speed up to maintain participation rate
    increaseOrderRate();
} else {
    // Slow down to avoid over-participation
    decreaseOrderRate();
}
```

## Alternative: POV (Percent of Volume)

A simpler volume-aware strategy is **POV (Percent of Volume)**:

```cpp
// Target: Execute X% of market volume in real-time
while (sharesRemaining > 0) {
    observe marketVolume in last N seconds
    targetShares = marketVolume * participationRate
    sendOrder(targetShares)
}
```

POV is easier to implement than VWAP (no historical profile needed) and may be a better next step after TWAP.

## References

- **"VWAP Strategies"** - Kissell & Glantz (2003)
- **"Algorithmic Trading"** - Chan (2009)
- **"Market Microstructure in Practice"** - Lehalle & Laruelle (2013)

---

**Project**: Beacon  
**Component**: Reference Algorithms  
**Author**: Bryan Camp  
**Strategy**: VWAP (Volume-Weighted Average Price) - Documentation  
**Status**: Architecture defined, full implementation pending volume infrastructure
