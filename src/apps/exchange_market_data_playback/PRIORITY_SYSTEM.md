# Message Priority System

## Overview

The priority system allows **important messages to bypass flow control and safety rules**. This is critical for realistic market simulation where high-priority events (market moves, critical symbols, important time windows) should always get through.

## Message Priority Levels

```
EMERGENCY (3)  →  Bypass ALL rules (even safety)
    ↓
CRITICAL (2)   →  Bypass flow control + chaos rules
    ↓
ELEVATED (1)   →  Bypass chaos rules, reduced delays
    ↓
NORMAL (0)     →  Subject to all rules
```

## Use Cases

### 1. Market Moves Bypass Rate Limits

**Problem**: During volatile market conditions, your rate limiter blocks important messages.

**Solution**: Burst detection classifier automatically elevates priority during high-volume periods.

```json
{
  "priority_config": {
    "classifier_type": "burst_detection",
    "burst_threshold": 2.0
  },
  "rate_limit": {
    "max_rate": 50000,
    "apply_to_priority": "normal_only"
  }
}
```

**Behavior**:
- Normal conditions: Messages limited to 50K/sec
- Market spike (>2x average rate): Messages marked ELEVATED, bypass rate limit
- System protects itself during normal flow, but allows market moves through

### 2. Critical Symbols Always Get Through

**Problem**: You're testing SPY (S&P 500 ETF) handling, but burst patterns are delaying critical SPY messages.

**Solution**: Symbol-based classifier marks SPY as CRITICAL.

```json
{
  "priority_config": {
    "critical_symbols": ["SPY", "QQQ", "VIX"]
  }
}
```

**Behavior**:
- SPY messages: CRITICAL priority, bypass burst delays
- Other symbols: NORMAL priority, subject to burst windows
- Tests system's ability to prioritize important symbols

### 3. Market Open/Close Are Critical Windows

**Problem**: Market open and close are the most important periods, but playback treats all times equally.

**Solution**: Time window classifier elevates priority during key periods.

```json
{
  "priority_config": {
    "critical_time_windows": [
      {
        "start_ms": 0,
        "end_ms": 60000,
        "priority": "critical"
      }
    ]
  }
}
```

**Behavior**:
- First 60 seconds: All messages CRITICAL
- Rest of replay: Normal priorities
- Tests system's handling of opening auction

## Classifiers

### BurstDetectionClassifier

Detects sudden rate spikes and elevates priority:

```cpp
auto classifier = std::make_shared<BurstDetectionClassifier>(2.0);  // 2x threshold
```

**Logic**:
- Tracks moving average of message rate
- If current rate > 2x average → ELEVATED priority
- Simulates real market volatility spikes

### SymbolPriorityClassifier

Marks specific symbols as high priority:

```cpp
auto classifier = std::make_shared<SymbolPriorityClassifier>();
classifier->addCriticalSymbol("SPY");
classifier->addCriticalSymbol("QQQ");
```

**Logic**:
- Extracts symbol from message (offset 12, 8 bytes)
- If symbol in critical list → CRITICAL priority
- Tests priority routing by symbol

### TimeWindowClassifier

Elevates priority during specific time windows:

```cpp
auto classifier = std::make_shared<TimeWindowClassifier>();
classifier->addCriticalWindow(0, 60000, MessagePriority::CRITICAL);  // First minute
```

**Logic**:
- Checks elapsed time since playback start
- If within window → specified priority
- Tests time-sensitive scenarios

## Priority-Aware Rules

### PriorityAwareRateLimitRule

Rate limit that respects message priority:

```cpp
auto classifier = std::make_shared<BurstDetectionClassifier>();
auto rateLimit = std::make_unique<PriorityAwareRateLimitRule>(50000, classifier);
```

**Behavior**:
- NORMAL: Full rate limit enforcement (may VETO)
- ELEVATED: Reduced backoff delay
- CRITICAL+: Bypass rate limit completely

### PriorityAwareBurstRule

Burst pattern that allows critical messages through:

```cpp
auto classifier = std::make_shared<SymbolPriorityClassifier>();
classifier->addCriticalSymbol("SPY");
auto burst = std::make_unique<PriorityAwareBurstRule>(5000, 100ms, classifier);
```

**Behavior**:
- NORMAL: Wait for burst window
- ELEVATED: Half the wait time
- CRITICAL+: Immediate send, bypass burst window

## Example Scenarios

### Scenario 1: VIX Spike Simulation

```json
{
  "mode": "priority_burst",
  "burst_size": 5000,
  "burst_interval_ms": 100,
  
  "priority_config": {
    "classifier_type": "symbol",
    "critical_symbols": ["VIX", "SPY", "QQQ"]
  }
}
```

**Test**: VIX messages always get through during volatility, even if burst window closed.

### Scenario 2: Market Open Rush

```json
{
  "mode": "priority_burst",
  "burst_size": 10000,
  "burst_interval_ms": 50,
  
  "priority_config": {
    "critical_time_windows": [
      {
        "start_ms": 0,
        "end_ms": 120000,
        "priority": "critical"
      }
    ]
  },
  
  "rate_limit": {
    "max_rate": 100000,
    "apply_to_priority": "normal_only"
  }
}
```

**Test**: First 2 minutes unrestricted (opening auction), then rate limited.

### Scenario 3: Adaptive Market Move

```json
{
  "mode": "continuous",
  "rate_msgs_per_sec": 10000,
  
  "priority_config": {
    "classifier_type": "burst_detection",
    "burst_threshold": 1.5
  },
  
  "rate_limit": {
    "max_rate": 50000,
    "apply_to_priority": "normal_only"
  }
}
```

**Test**: Normal 10K msgs/sec, but during market moves (>1.5x avg), rate limit lifts.

## Priority Decision Flow

```
Message arrives
    ↓
Classify priority (NORMAL, ELEVATED, CRITICAL, EMERGENCY)
    ↓
    ├─ EMERGENCY (3) ───────────────────────────┐
    │                                            │
    ├─ CRITICAL (2) ──────────────┐             │
    │                              │             │
    ├─ ELEVATED (1) ────┐          │             │
    │                   │          │             │
    └─ NORMAL (0)       │          │             │
         ↓              │          │             │
    [Apply all rules]   │          │             │
         ↓              │          │             │
    Rate limit?         │          │             │
    Burst delay?        │          │             │
    Packet loss?        │          │             │
         ↓              │          │             │
         └──────────────┴──────────┴─────────────┘
                        ↓
                   Send message
```

## Benefits

✅ **Realistic**: Markets prioritize important symbols/times  
✅ **Flexible**: Combine multiple classifiers  
✅ **Testable**: Verify system handles priority correctly  
✅ **Safe**: High-priority doesn't mean "break everything"  

## Future Enhancements

- **Composite classifiers**: Combine symbol + time + rate detection
- **Dynamic priority**: Adjust based on system load
- **Priority queues**: Reorder messages by priority before sending
- **Priority statistics**: Track which priorities are being used

---

**Key Insight**: Priority system lets you simulate **"important messages must get through"** scenarios without disabling all flow control.
