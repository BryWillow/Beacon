# How to Plug In Your Own Trading Algorithm

## Quick Start (5 Steps)

### Step 1: Copy the Template

```bash
cd src/apps/client_algorithm
cp algo_template.cpp algo_my_strategy.cpp
```

### Step 2: Edit CMakeLists.txt

Add your algorithm to the build:

```cmake
# Add this to CMakeLists.txt
add_executable(algo_my_strategy
    algo_my_strategy.cpp
)

# Link threading
target_link_libraries(algo_my_strategy Threads::Threads)
```

### Step 3: Implement Your Trading Logic

Open `algo_my_strategy.cpp` and find this section:

```cpp
// ===== YOUR TRADING LOGIC GOES HERE =====
if (mdQueue.tryPop(md)) {
    // YOUR CODE HERE
    // Examples:
    // - if (price > moving_average) sell();
    // - if (volatility > threshold) reduce_position();
    // - if (spread < limit) arbitrage();
}
// ===== END YOUR TRADING LOGIC =====
```

### Step 4: Build

```bash
cd build
cmake ..
make algo_my_strategy
```

### Step 5: Run

```bash
./algo_my_strategy 239.255.0.1 12345 127.0.0.1 9000 60
```

---

## Architecture Overview

Your algorithm runs in a **3-core threaded architecture**:

```
┌─────────────────┐
│   Core 0: UDP   │  ← Receives market data (minimal latency)
│   Receiver      │    Just: recv() → push to queue
└────────┬────────┘
         │ Lock-free
         │ SPSC Queue
         ▼
┌─────────────────┐
│   Core 1: YOUR  │  ← YOUR TRADING LOGIC GOES HERE
│   ALGORITHM     │    Process data → Make decisions → Send orders
└────────┬────────┘
         │ TCP Orders
         ▼
┌─────────────────┐
│   Core 2: TCP   │  ← Receives fills/acks from exchange
│   Exec Reports  │    recv() → push to queue (blocking, never drops)
└─────────────────┘
         │ Lock-free
         │ SPSC Queue
         ▼
    (back to Core 1)
```

## What You Get (Infrastructure)

### ✅ Lock-Free Queues
```cpp
ringbuffer::SpScRingBuffer<MarketDataMessage, 32768> mdQueue;
```
- **Zero locks** - Single producer, single consumer
- **Cache optimized** - Aligned to cache lines
- **Blocking/non-blocking** - `tryPush()` or `push()`

### ✅ Thread Pinning (Linux)
```cpp
concurrency::PinnedThread thread(lambda, core_id);
```
- Pins thread to dedicated CPU core
- Reduces context switching
- Improves cache locality

### ✅ Efficient Spinning
```cpp
core::cpu_pause();  // x86 PAUSE instruction
```
- Tells CPU you're in a spin loop
- Reduces power consumption
- Improves hyperthreading

### ✅ Zero-Overhead Networking
```cpp
networking::UdpMulticastReceiver udp("239.255.0.1", 12345);
networking::TcpClient tcp("127.0.0.1", 9000);
```
- RAII socket management
- Inline recv/send (compiles to direct syscalls)
- TCP_NODELAY enabled by default

---

## Example Strategies

### Example 1: Simple Moving Average Crossover

```cpp
// Add to AlgoState
struct AlgoState {
    double ema_fast = 0.0;
    double ema_slow = 0.0;
    double alpha_fast = 0.1;  // Fast EMA weight
    double alpha_slow = 0.02; // Slow EMA weight
    bool hasPosition = false;
};

// In tradingLogicThread()
if (mdQueue.tryPop(md)) {
    double price = md.price / 10000.0;
    
    // Update EMAs
    if (g_algoState.ema_fast == 0.0) {
        g_algoState.ema_fast = price;
        g_algoState.ema_slow = price;
    } else {
        g_algoState.ema_fast = g_algoState.alpha_fast * price + 
                               (1 - g_algoState.alpha_fast) * g_algoState.ema_fast;
        g_algoState.ema_slow = g_algoState.alpha_slow * price + 
                               (1 - g_algoState.alpha_slow) * g_algoState.ema_slow;
    }
    
    // Trading signal
    if (g_algoState.ema_fast > g_algoState.ema_slow && !g_algoState.hasPosition) {
        // Fast EMA crossed above slow EMA → BUY
        sendOrder('B', 100, md.price);
        g_algoState.hasPosition = true;
    } else if (g_algoState.ema_fast < g_algoState.ema_slow && g_algoState.hasPosition) {
        // Fast EMA crossed below slow EMA → SELL
        sendOrder('S', 100, md.price);
        g_algoState.hasPosition = false;
    }
}
```

### Example 2: Mean Reversion with Bollinger Bands

```cpp
struct AlgoState {
    std::deque<double> prices;
    size_t window = 20;
    double stddev_mult = 2.0;
};

// In trading logic
if (mdQueue.tryPop(md)) {
    double price = md.price / 10000.0;
    g_algoState.prices.push_back(price);
    
    if (g_algoState.prices.size() > g_algoState.window) {
        g_algoState.prices.pop_front();
        
        // Calculate mean and stddev
        double mean = std::accumulate(g_algoState.prices.begin(), 
                                      g_algoState.prices.end(), 0.0) / window;
        double sq_sum = 0.0;
        for (double p : g_algoState.prices) {
            sq_sum += (p - mean) * (p - mean);
        }
        double stddev = std::sqrt(sq_sum / window);
        
        double upper_band = mean + stddev_mult * stddev;
        double lower_band = mean - stddev_mult * stddev;
        
        // Mean reversion signals
        if (price < lower_band) {
            sendOrder('B', 100, md.price);  // Oversold → Buy
        } else if (price > upper_band) {
            sendOrder('S', 100, md.price);  // Overbought → Sell
        }
    }
}
```

### Example 3: Market Making (Spread Capture)

```cpp
struct AlgoState {
    uint32_t bestBid = 0;
    uint32_t bestAsk = 0;
    uint32_t spreadThreshold = 50;  // 0.50 cents minimum spread
    int32_t inventory = 0;
    int32_t maxInventory = 500;
};

// In trading logic
if (mdQueue.tryPop(md)) {
    // Update best bid/ask (would parse from order book messages)
    g_algoState.bestBid = md.price - 5;
    g_algoState.bestAsk = md.price + 5;
    
    uint32_t spread = g_algoState.bestAsk - g_algoState.bestBid;
    
    if (spread > g_algoState.spreadThreshold) {
        // Spread is wide enough - quote inside
        if (g_algoState.inventory < g_algoState.maxInventory) {
            // Quote bid (buy) to accumulate inventory
            sendOrder('B', 100, g_algoState.bestBid + 1);
        }
        
        if (g_algoState.inventory > -g_algoState.maxInventory) {
            // Quote ask (sell) to reduce inventory
            sendOrder('S', 100, g_algoState.bestAsk - 1);
        }
    }
}

// Update inventory from fills
while (execQueue.tryPop(exec)) {
    if (exec.status == 2) {  // Filled
        if (/* was buy order */) {
            g_algoState.inventory += exec.execQty;
        } else {
            g_algoState.inventory -= exec.execQty;
        }
    }
}
```

---

## Best Practices

### 1. **Keep Hot Path Minimal**
```cpp
// ❌ BAD: I/O in hot path
if (mdQueue.tryPop(md)) {
    std::cout << "Processing: " << md.symbol << "\n";  // Slow!
    // ... trading logic
}

// ✅ GOOD: Logging outside hot path or use counters
if (mdQueue.tryPop(md)) {
    g_mdProcessed.fetch_add(1);  // Fast atomic
    // ... trading logic
}
```

### 2. **Use Atomics for Shared State**
```cpp
// ❌ BAD: Regular variables (data race)
uint64_t orderCount = 0;

// ✅ GOOD: Atomic for thread-safe counters
std::atomic<uint64_t> orderCount{0};
orderCount.fetch_add(1, std::memory_order_relaxed);
```

### 3. **Handle Partial Fills**
```cpp
while (execQueue.tryPop(exec)) {
    if (exec.status == 1) {  // Partial fill
        // Update working order state
        // May need to send more
    } else if (exec.status == 2) {  // Filled
        // Order complete
    }
}
```

### 4. **Implement Risk Limits**
```cpp
struct RiskLimits {
    int32_t maxPosition = 10000;
    int32_t currentPosition = 0;
    double maxLoss = -5000.0;
    double currentPnL = 0.0;
};

// Before sending order
if (abs(g_risk.currentPosition + orderQty) > g_risk.maxPosition) {
    // Reject: Would exceed position limit
    return;
}

if (g_risk.currentPnL < g_risk.maxLoss) {
    // Halt: Loss limit hit
    g_running.store(false);
    return;
}
```

### 5. **Profile Before Optimizing**
```cpp
// Add timing to measure latency
auto start = std::chrono::high_resolution_clock::now();
// ... your trading logic ...
auto end = std::chrono::high_resolution_clock::now();
auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

if (latency_us > 100) {  // More than 100μs
    std::cerr << "WARNING: Slow decision: " << latency_us << "μs\n";
}
```

---

## Testing Your Algorithm

### 1. Start All Components

**Terminal 1: Matching Engine**
```bash
cd src/apps/exchange_matching_engine
./build/exchange_matching_engine 9000
```

**Terminal 2: Market Data Playback**
```bash
cd src/apps/exchange_market_data_playback
./build/exchange_market_data_playback --config config_udp_slow.json \
    ../exchange_market_data_generator/output.mdp
```

**Terminal 3: Your Algorithm**
```bash
cd src/apps/client_algorithm
./build/algo_my_strategy 239.255.0.1 12345 127.0.0.1 9000 60
```

### 2. Monitor Performance

Watch for:
- **Order rate**: Orders sent per second
- **Fill rate**: What % of orders get filled?
- **Latency**: Time from market data → order sent
- **Queue depth**: Are queues backing up?

### 3. Validate Logic

Add assertions:
```cpp
assert(orderQty > 0 && "Order quantity must be positive");
assert(price > 0 && "Price must be positive");
assert(currentPosition + orderQty <= maxPosition && "Position limit");
```

---

## Reference Implementations

See these for complete examples:

1. **`algo_template.cpp`** - Minimal starting point (this file)
2. **`algo_twap.cpp`** - Industry-standard TWAP implementation
3. **`client_algo_hft.cpp`** - Generic HFT framework

---

## Next Steps

1. ✅ Copy template
2. ✅ Implement your logic
3. ✅ Add risk management
4. ✅ Test with synthetic data
5. ⏭️ Add performance instrumentation
6. ⏭️ Profile and optimize hot paths
7. ⏭️ Test with realistic scenarios
8. ⏭️ Deploy to production (carefully!)

---

**Project**: Beacon  
**Component**: Algorithm Development Guide  
**Author**: Bryan Camp
