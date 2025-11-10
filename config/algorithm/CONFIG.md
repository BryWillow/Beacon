# Client Algorithm Configuration

## Overview

The client algorithm now supports JSON-based configuration for per-product position management and trading parameters.

## Configuration File Structure

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
  "trading": {
    "order_frequency": 1000,
    "default_time_in_force": "0",
    "default_capacity": "A"
  },
  "risk_management": {
    "pnl_drawdown_warning": -5000.0,
    "pnl_drawdown_alert": -10000.0,
    "pnl_drawdown_hard_stop": -15000.0,
    "order_reject_warning": 10,
    "order_reject_alert": 25,
    "order_reject_hard_stop": 50
  },
  "products": [
    {
      "symbol": "AAPL",
      "starting_position": 0,
      "maximum_quantity": 10000,
      "enabled": true
    }
  ]
}
```

## Configuration Sections

### 1. Market Data (`market_data`)

Specifies connection to the market data server.

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `host` | string | Yes | Market data server hostname/IP |
| `port` | number | Yes | Market data server port |

**Example:**
```json
"market_data": {
  "host": "127.0.0.1",
  "port": 12345
}
```

### 2. Exchange (`exchange`)

Specifies connection to the matching engine/exchange.

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `host` | string | Yes | Exchange hostname/IP |
| `port` | number | Yes | Exchange port |

**Example:**
```json
"exchange": {
  "host": "127.0.0.1",
  "port": 54321
}
```

### 3. Trading Parameters (`trading`)

General trading behavior configuration.

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `order_frequency` | number | No | 1000 | Send order every N market data messages |
| `default_time_in_force` | string | No | "0" | Default TIF: "0"=Day, "3"=IOC, "4"=FOK, "G"=GTC |
| `default_capacity` | string | No | "A" | Default capacity: "A"=Agency, "P"=Principal |

**Example:**
```json
"trading": {
  "order_frequency": 1000,
  "default_time_in_force": "0",
  "default_capacity": "A"
}
```

**Time In Force Options:**
- `"0"` - Day order (cancel at market close)
- `"3"` - IOC (Immediate or Cancel)
- `"4"` - FOK (Fill or Kill)
- `"G"` - GTC (Good Till Cancel)

**Capacity Options:**
- `"A"` - Agency (trading for customer)
- `"P"` - Principal (trading for own account)

### 4. Risk Management (`risk_management`)

Algorithm-level PnL drawdown and order reject thresholds for risk control.

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `pnl_drawdown_warning` | number | No | -5000.0 | Warning threshold (log warning at this loss) |
| `pnl_drawdown_alert` | number | No | -10000.0 | Alert threshold (send alert at this loss) |
| `pnl_drawdown_hard_stop` | number | No | -15000.0 | Hard stop threshold (halt trading at this loss) |
| `order_reject_warning` | number | No | 10 | Warning threshold (log warning at this reject count) |
| `order_reject_alert` | number | No | 25 | Alert threshold (send alert at this reject count) |
| `order_reject_hard_stop` | number | No | 50 | Hard stop threshold (halt trading at this reject count) |

**Example:**
```json
"risk_management": {
  "pnl_drawdown_warning": -5000.0,
  "pnl_drawdown_alert": -10000.0,
  "pnl_drawdown_hard_stop": -15000.0,
  "order_reject_warning": 10,
  "order_reject_alert": 25,
  "order_reject_hard_stop": 50,
  "order_messaging_burst_warning": 100,
  "order_messaging_burst_alert": 250,
  "order_messaging_burst_hard_stop": 500
}
```

#### PnL Drawdown Thresholds

All PnL values are **negative numbers** representing losses:

- **Warning Level** (`-5000.0`): Log warning message
  - Algorithm continues trading
  - Useful for monitoring and early detection
  - Example: "WARNING: PnL at -$5,123.45, approaching alert level"

- **Alert Level** (`-10000.0`): Send alert notification
  - Algorithm continues trading (for now)
  - Should trigger immediate human review
  - Example: "ALERT: PnL at -$10,567.89, approaching hard stop"

- **Hard Stop** (`-15000.0`): Immediately halt all trading
  - No new orders sent
  - Existing orders may be cancelled
  - Requires manual intervention to restart
  - Example: "HARD STOP: PnL at -$15,234.56, trading halted"

**Validation:**

Thresholds must be in correct order (all negative):
```
warning >= alert >= hard_stop

Example (correct):
-5000 >= -10000 >= -15000  ✓

Example (incorrect):
-10000 >= -5000 >= -15000  ✗ (warning worse than alert)
```

**PnL Calculation:**

The algorithm tracks cumulative PnL across all products:
```
PnL = Σ(fills) - Σ(costs)

For each fill:
  Buy:  PnL -= (quantity × price)    // Cash outflow
  Sell: PnL += (quantity × price)    // Cash inflow
```

#### Order Reject Thresholds

All reject values are **positive numbers** representing cumulative reject count:

- **Warning Level** (`10`): Log warning message
  - Algorithm continues trading
  - Useful for detecting connectivity or configuration issues
  - Example: "WARNING: 10 order rejects, check exchange connection"

- **Alert Level** (`25`): Send alert notification
  - Algorithm continues trading (for now)
  - Indicates serious issue with orders or exchange
  - Example: "ALERT: 25 order rejects, investigate immediately"

- **Hard Stop** (`50`): Immediately halt all trading
  - No new orders sent
  - Prevents further rejects and potential issues
  - Requires manual intervention to restart
  - Example: "HARD STOP: 50 order rejects, trading halted"

**Common Reject Reasons:**
- Invalid symbol
- Invalid price (outside price bands)
- Insufficient buying power
- Duplicate order ID
- Market closed
- Halted symbol
- Invalid message format

**Reject Counter Behavior:**
```
Order 1 sent → Rejected (reason: invalid price)  → reject_count = 1
Order 2 sent → Filled                            → reject_count = 1 (no change)
Order 3 sent → Rejected (reason: duplicate ID)  → reject_count = 2
Order 4 sent → Rejected (reason: market closed) → reject_count = 3
...
Order N sent → Rejected                          → reject_count = 10 (WARNING)
```

**Validation:**

PnL thresholds must be in correct order (all negative):
```
pnl_warning >= pnl_alert >= pnl_hard_stop

Example (correct):
-5000 >= -10000 >= -15000  ✓

Example (incorrect):
-10000 >= -5000 >= -15000  ✗ (warning worse than alert)
```

Reject thresholds must be in correct order (all positive):
```
reject_warning <= reject_alert <= reject_hard_stop

Example (correct):
10 <= 25 <= 50  ✓

Example (incorrect):
25 <= 10 <= 50  ✗ (warning higher than alert)
```

Burst thresholds must be in correct order (all positive):
```
burst_warning <= burst_alert <= burst_hard_stop

Example (correct):
100 <= 250 <= 500  ✓

Example (incorrect):
250 <= 100 <= 500  ✗ (warning higher than alert)
```

#### Order Messaging Burst Thresholds

The algorithm tracks the **rate of order submissions** (orders per second) over a sliding time window to detect and control rapid-fire trading patterns that could:
- Trigger exchange throttling or penalties
- Indicate a runaway algorithm
- Violate exchange rate limits
- Cause network congestion

**Behavior:**
- The system measures orders submitted within the last second (rolling window)
- When the rate crosses thresholds, graduated responses occur:
  - **Warning:** Log the high rate, continue trading
  - **Alert:** Notify monitoring systems, continue with caution
  - **Hard Stop:** Immediately halt all order submission to prevent exchange penalties

**Typical Burst Scenarios:**
- Market open surge (high volatility, many simultaneous opportunities)
- Algorithm malfunction (infinite loop, logic error)
- Network recovery (queued orders released all at once)
- Flash crash response (rapid order cancels and replacements)

**Example - Burst Rate Tracking:**
```
Time         Orders Sent    Rolling 1-sec Count    Status
10:00:00.000    1                1                Normal
10:00:00.100    1                2                Normal
10:00:00.200    1                3                Normal
...
10:00:00.900    1               10                Normal
10:00:01.000    1                1 (window reset) Normal

# Burst scenario
10:00:05.000   50               50                Normal
10:00:05.100   50              100                WARNING (≥100)
10:00:05.200   50              150                WARNING
10:00:05.300  100              250                ALERT (≥250)
10:00:05.400  100              350                ALERT
10:00:05.500  150              500                HARD STOP (≥500)
```

**Implementation Considerations:**
- Use rolling time window (not fixed 1-second buckets)
- Track timestamp of each order
- Remove orders older than 1 second from count
- Consider exchange-specific rate limits (e.g., NASDAQ: 500 orders/sec, NYSE: 1000 orders/sec)

---

**PnL Calculation:**
```

**Use Cases:**

1. **Conservative Trading:**
   ```json
   {
     "pnl_drawdown_warning": -1000.0,
     "pnl_drawdown_alert": -2000.0,
     "pnl_drawdown_hard_stop": -3000.0,
     "order_reject_warning": 5,
     "order_reject_alert": 10,
     "order_reject_hard_stop": 20,
     "order_messaging_burst_warning": 50,
     "order_messaging_burst_alert": 100,
     "order_messaging_burst_hard_stop": 200
   }
   ```
   Small drawdown tolerance, quick stop on rejects, moderate burst limits.

2. **Moderate Risk:**
   ```json
   {
     "pnl_drawdown_warning": -5000.0,
     "pnl_drawdown_alert": -10000.0,
     "pnl_drawdown_hard_stop": -15000.0,
     "order_reject_warning": 10,
     "order_reject_alert": 25,
     "order_reject_hard_stop": 50,
     "order_messaging_burst_warning": 100,
     "order_messaging_burst_alert": 250,
     "order_messaging_burst_hard_stop": 500
   }
   ```
   Default configuration, balanced approach.

3. **Aggressive Trading / High-Frequency:**
   ```json
   {
     "pnl_drawdown_warning": -25000.0,
     "pnl_drawdown_alert": -50000.0,
     "pnl_drawdown_hard_stop": -75000.0,
     "order_reject_warning": 50,
     "order_reject_alert": 100,
     "order_reject_hard_stop": 200,
     "order_messaging_burst_warning": 500,
     "order_messaging_burst_alert": 1000,
     "order_messaging_burst_hard_stop": 2000
   }
   ```
   Large drawdown tolerance, high reject tolerance, high burst limits for high-volume strategies.

### 5. Products (`products`)

Array of per-product configuration with position limits.

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `symbol` | string | Yes | - | Product symbol (e.g., "AAPL", "MSFT") |
| `starting_position` | number | Yes | - | Initial position (positive=long, negative=short, 0=flat) |
| `algo_position_limit` | number | Yes | - | Algorithm's position limit (must be <= firm_position_limit) |
| `firm_position_limit` | number | No | = algo_position_limit | Firm-wide regulatory/risk position limit (hard cap) |
| `enabled` | boolean | No | true | Whether trading is enabled for this symbol |

**Example:**
```json
"products": [
  {
    "symbol": "AAPL",
    "starting_position": 0,
    "algo_position_limit": 10000,
    "firm_position_limit": 15000,
    "enabled": true
  },
  {
    "symbol": "MSFT",
    "starting_position": 500,
    "algo_position_limit": 4000,
    "firm_position_limit": 5000,
    "enabled": true
  },
  {
    "symbol": "GOOGL",
    "starting_position": -200,
    "algo_position_limit": 1500,
    "firm_position_limit": 2000,
    "enabled": false
  }
]
```

## Product Configuration Details

### Starting Position (`starting_position`)

The initial position for the symbol when the algorithm starts.

- **Positive values**: Long position (e.g., `500` = long 500 shares)
- **Negative values**: Short position (e.g., `-200` = short 200 shares)
- **Zero**: Flat/no position (e.g., `0` = no position)

**Use Cases:**
- Start flat and build position: `starting_position: 0`
- Continue with existing long: `starting_position: 1000`
- Continue with existing short: `starting_position: -500`

### Maximum Quantity (`algo_position_limit`)

The algorithm's maximum position size - this is the trading limit for your specific algorithm.

- **Limits both long and short**: If `algo_position_limit: 5000`, position can range from `-5000` to `+5000`
- **Algorithm-specific**: Each algorithm instance has its own limits
- **Must be <= firm_position_limit**: Cannot exceed the firm-wide regulatory limit
- **Per-symbol limit**: Each symbol has independent position limit

**Examples:**
```json
{
  "symbol": "AAPL",
  "starting_position": 0,
  "algo_position_limit": 10000,
  "firm_position_limit": 15000
}
```
- Algorithm can trade up to 10,000 shares (position: -10,000 to +10,000)
- Firm limit is 15,000 shares (allows other algos/traders to use remaining 5,000)

```json
{
  "symbol": "MSFT",
  "starting_position": 5000,
  "algo_position_limit": 8000,
  "firm_position_limit": 10000
}
```
- Starting with long 5,000 shares
- Algorithm can buy up to 3,000 more shares (5,000 → 8,000)
- Algorithm can sell up to 13,000 shares (5,000 → -8,000)
- Firm limit is 10,000 shares total

### Position Limit (`firm_position_limit`)

The firm-wide regulatory or risk management hard cap on positions.

- **Optional field**: If not specified, defaults to `algo_position_limit`
- **Must be >= algo_position_limit**: Regulatory/compliance requirement enforced at load time
- **Hard regulatory limit**: Often set by compliance/risk management
- **Shared across strategies**: Multiple algorithms may share this limit

**Use Cases:**

1. **Regulatory Compliance:**
   ```json
   {
     "symbol": "AAPL",
     "starting_position": 0,
     "algo_position_limit": 10000,
     "firm_position_limit": 15000
   }
   ```
   - Algorithm trades up to 10,000 shares
   - Firm's total exposure limited to 15,000 shares
   - Leaves 5,000 share capacity for other desks/algorithms

2. **Risk Allocation:**
   ```json
   {
     "symbol": "TSLA",
     "starting_position": 0,
     "algo_position_limit": 6000,
     "firm_position_limit": 20000
   }
   ```
   - This algorithm allocated 6,000 shares
   - Firm's total risk budget: 20,000 shares
   - Other algorithms can use remaining 14,000 share allocation

3. **Single Algorithm (Equal Limits):**
   ```json
   {
     "symbol": "MSFT",
     "starting_position": 0,
     "algo_position_limit": 5000,
     "firm_position_limit": 5000
   }
   ```
   - Algorithm uses entire firm allocation
   - Single unified position limit
   - Equivalent to not specifying `firm_position_limit`

**Validation:**
- **CRITICAL**: If `algo_position_limit` > `firm_position_limit`, configuration will be **REJECTED**
- This is enforced as an error (not just a warning) to prevent compliance violations
- Both limits are checked on every trade

### Enabled (`enabled`)

Controls whether the algorithm trades this symbol.

- `true`: Symbol is tradeable
- `false`: Symbol is ignored (no orders sent)

**Use Cases:**
- Temporarily disable trading: `"enabled": false`
- Keep configuration but pause trading
- A/B testing different symbol sets

## Position Management

The algorithm tracks position for each product:

### Position Tracking

```cpp
// Initial state
starting_position: 500
current_position: 500  // Starts at starting_position

// After buying 100 shares
current_position: 600

// After selling 300 shares
current_position: 300
```

### Position Checks

Before sending orders, the algorithm checks against **the more restrictive** of `algo_position_limit` and `firm_position_limit`:

**Can Buy?**
```cpp
effective_limit = min(algo_position_limit, firm_position_limit)
new_position = current_position + quantity
can_buy = (new_position <= effective_limit) && enabled
```

**Example:**
```
current_position: 6000
algo_position_limit: 7500
firm_position_limit: 10000
effective_limit: 7500 (more restrictive)
quantity: 2000
new_position: 8000 (exceeds algo_position_limit of 7500!)
Result: Cannot buy (order rejected)
```

**Can Sell?**
```cpp
effective_limit = min(algo_position_limit, firm_position_limit)
new_position = current_position - quantity
can_sell = (|new_position| <= effective_limit) && enabled
```

**Example:**
```
current_position: -5000 (short)
algo_position_limit: 6000
firm_position_limit: 10000
effective_limit: 6000 (more restrictive)
quantity: 2000
new_position: -7000 (exceeds algo_position_limit of 6000!)
Result: Cannot sell (order rejected)
```

**Key Points:**
- Both `algo_position_limit` and `firm_position_limit` are always enforced
- The **smaller value** determines the actual trading limit
- `algo_position_limit` must be <= `firm_position_limit` (enforced at config load)
- If `firm_position_limit` is not specified, it defaults to `algo_position_limit`
- This allows for firm-wide risk allocation across multiple algorithms

## Usage Examples

### Example 1: Conservative Day Trading

```json
{
  "trading": {
    "order_frequency": 500,
    "default_time_in_force": "3",
    "default_capacity": "P"
  },
  "products": [
    {
      "symbol": "SPY",
      "starting_position": 0,
      "algo_position_limit": 1000,
      "firm_position_limit": 2000,
      "enabled": true
    },
    {
      "symbol": "QQQ",
      "starting_position": 0,
      "algo_position_limit": 500,
      "firm_position_limit": 1000,
      "enabled": true
    }
  ]
}
```

- Frequent orders (every 500 messages)
- IOC orders (no resting orders)
- Principal capacity (prop trading)
- Small algo position limits (conservative risk)
- Larger firm limits (allows other algorithms to trade same symbols)

### Example 2: Overnight Positions

```json
{
  "trading": {
    "order_frequency": 1000,
    "default_time_in_force": "0",
    "default_capacity": "A"
  },
  "products": [
    {
      "symbol": "AAPL",
      "starting_position": 5000,
      "algo_position_limit": 10000,
      "firm_position_limit": 15000,
      "enabled": true
    },
    {
      "symbol": "MSFT",
      "starting_position": -2000,
      "algo_position_limit": 8000,
      "firm_position_limit": 12000,
      "enabled": true
    }
  ]
}
```

- Starting with existing positions
- AAPL: Long 5,000, algo can add 5,000 more (up to 10K), firm limit 15K
- MSFT: Short 2,000, algo can cover or short 6,000 more (up to 8K total), firm limit 12K

### Example 3: Selective Trading

```json
{
  "products": [
    {
      "symbol": "AAPL",
      "starting_position": 0,
      "algo_position_limit": 10000,
      "firm_position_limit": 20000,
      "enabled": true
    },
    {
      "symbol": "MSFT",
      "starting_position": 0,
      "algo_position_limit": 5000,
      "firm_position_limit": 10000,
      "enabled": false
    },
    {
      "symbol": "GOOGL",
      "starting_position": 0,
      "algo_position_limit": 2000,
      "firm_position_limit": 5000,
      "enabled": true
    }
  ]
}
```

- AAPL: Active trading, algo uses 50% of firm allocation
- MSFT: Disabled (configured but not trading)
- GOOGL: Active trading, algo uses 40% of firm allocation

### Example 4: Tiered Risk Allocation

```json
{
  "products": [
    {
      "symbol": "AAPL",
      "starting_position": 0,
      "algo_position_limit": 7500,
      "firm_position_limit": 15000,
      "enabled": true
    },
    {
      "symbol": "TSLA",
      "starting_position": 0,
      "algo_position_limit": 5000,
      "firm_position_limit": 20000,
      "enabled": true
    },
    {
      "symbol": "NVDA",
      "starting_position": 2000,
      "algo_position_limit": 4000,
      "firm_position_limit": 10000,
      "enabled": true
    }
  ]
}
```

- **AAPL**: Algo uses 7,500 shares, firm limit 15,000 (50% allocation)
- **TSLA**: Algo uses 5,000 shares, firm limit 20,000 (25% allocation, volatile stock)
- **NVDA**: Starting with 2,000 shares long
  - Algo can reach 4,000 shares (2,000 more capacity)
  - Firm limit 10,000 shares (other strategies can use remaining 6,000)

## Configuration Validation

The config provider validates:

✅ **Required Fields**: All required fields must be present  
✅ **Type Checking**: Values must match expected types  
✅ **Range Validation**: Ports, quantities must be positive  
✅ **Position Limit Hierarchy**: `algo_position_limit` must be <= `firm_position_limit` **(ENFORCED AS ERROR)**
✅ **Position Consistency**: Warns if starting_position exceeds algo_position_limit  
✅ **Duplicate Symbols**: Not allowed (uses first occurrence)

**Critical Validation - Position Limit Hierarchy:**
```
algo_position_limit <= firm_position_limit  (REQUIRED)
```

If violated, configuration loading will **FAIL** with error:
```
ERROR: Product 'AAPL' algo_position_limit (15000) exceeds firm_position_limit (10000)
  algo_position_limit must be <= firm_position_limit
```

This ensures regulatory compliance and prevents algorithms from exceeding firm-wide risk limits.

**Warning Examples:**
```
WARNING: Product 'AAPL' has algo_position_limit=0, will not be tradeable
WARNING: Product 'MSFT' starting_position (15000) exceeds algo_position_limit (10000)
```

**Error Examples (Configuration Rejected):**
```
ERROR: Product 'AAPL' algo_position_limit (15000) exceeds firm_position_limit (10000)
  algo_position_limit must be <= firm_position_limit
```

## API Usage

### Loading Configuration

```cpp
#include "config_provider.h"

using namespace beacon::client_algorithm;

ConfigProvider config;
if (!config.loadFromFile("sample_config.json")) {
    std::cerr << "Failed to load config\n";
    return 1;
}
```

### Accessing Configuration

```cpp
// Get connection settings
const auto& mdConfig = config.getMarketDataConfig();
std::cout << "MD: " << mdConfig.host << ":" << mdConfig.port << "\n";

// Get trading parameters
const auto& trading = config.getTradingConfig();
std::cout << "Order frequency: " << trading.order_frequency << "\n";

// Get risk management parameters
const auto& risk = config.getRiskManagementConfig();
std::cout << "Hard stop threshold: " << risk.pnl_drawdown_hard_stop << "\n";

// Get product configuration
const ProductConfig* appleConfig = config.getProductConfig("AAPL");
if (appleConfig && appleConfig->enabled) {
    std::cout << "AAPL max position: " << appleConfig->maximum_quantity << "\n";
}

// Get all enabled products
auto enabled = config.getEnabledProducts();
for (const auto& product : enabled) {
    std::cout << product.symbol << " is enabled\n";
}
```

### Position Management

```cpp
const ProductConfig* config = configProvider.getProductConfig("AAPL");

// Check if can buy
if (config->canBuy(100)) {
    // Send buy order
    sendOrder("AAPL", 100, 'B');
    
    // Update position after fill
    config->updatePosition(100, true);  // true = buy
}

// Check remaining capacity
int32_t buyCapacity = config->getRemainingBuyCapacity();
int32_t sellCapacity = config->getRemainingSellCapacity();

std::cout << "Can still buy: " << buyCapacity << " shares\n";
std::cout << "Can still sell: " << sellCapacity << " shares\n";
```

### Risk Management Checks

```cpp
const auto& risk = configProvider.getRiskManagementConfig();
double current_pnl = calculateCurrentPnL();  // Your PnL calculation

// Check risk levels
if (risk.isHardStop(current_pnl)) {
    std::cerr << "HARD STOP: PnL " << current_pnl 
              << " <= " << risk.pnl_drawdown_hard_stop << "\n";
    haltAllTrading();
    sendEmergencyNotification();
    
} else if (risk.isAlertLevel(current_pnl)) {
    std::cerr << "ALERT: PnL " << current_pnl 
              << " <= " << risk.pnl_drawdown_alert << "\n";
    sendAlertNotification();
    
} else if (risk.isWarningLevel(current_pnl)) {
    std::cout << "WARNING: PnL " << current_pnl 
              << " <= " << risk.pnl_drawdown_warning << "\n";
    logWarning();
}

// Check order reject count
if (risk.isRejectHardStop(reject_count)) {
    std::cerr << "HARD STOP: Too many rejections (" << reject_count << ")\n";
    haltAllTrading();
    
} else if (risk.isRejectAlertLevel(reject_count)) {
    std::cerr << "ALERT: High rejection count (" << reject_count << ")\n";
    
} else if (risk.isRejectWarningLevel(reject_count)) {
    std::cout << "WARNING: Elevated rejection count (" << reject_count << ")\n";
}

// Check order burst rate
if (risk.isBurstHardStop(orders_per_second)) {
    std::cerr << "HARD STOP: Order burst rate too high (" << orders_per_second << " orders/sec)\n";
    haltAllTrading();
    
} else if (risk.isBurstAlertLevel(orders_per_second)) {
    std::cerr << "ALERT: High order burst rate (" << orders_per_second << " orders/sec)\n";
    
} else if (risk.isBurstWarningLevel(orders_per_second)) {
    std::cout << "WARNING: Elevated order burst rate (" << orders_per_second << " orders/sec)\n";
}

// Example PnL tracking
class PnLTracker {
public:
    void onFill(uint32_t quantity, uint32_t price, bool isBuy) {
        double value = (quantity * price) / 10000.0;  // Convert from fixed-point
        
        if (isBuy) {
            total_pnl_ -= value;  // Cash outflow
        } else {
            total_pnl_ += value;  // Cash inflow
        }
        
        checkRiskLimits();
    }
    
    double getCurrentPnL() const { return total_pnl_; }
    
private:
    double total_pnl_ = 0.0;
    
    void checkRiskLimits() {
        const auto& risk = config_.getRiskManagementConfig();
        
        if (risk.isHardStop(total_pnl_)) {
            trading_enabled_ = false;  // Stop trading
        }
    }
};

// Example Burst Rate Tracker
class BurstRateTracker {
public:
    void onOrderSent() {
        auto now = std::chrono::steady_clock::now();
        order_timestamps_.push_back(now);
        
        // Remove timestamps older than 1 second
        auto cutoff = now - std::chrono::seconds(1);
        order_timestamps_.erase(
            std::remove_if(order_timestamps_.begin(), order_timestamps_.end(),
                [cutoff](const auto& ts) { return ts < cutoff; }),
            order_timestamps_.end()
        );
        
        checkBurstRate();
    }
    
    uint32_t getCurrentRate() const {
        return static_cast<uint32_t>(order_timestamps_.size());
    }
    
private:
    std::vector<std::chrono::steady_clock::time_point> order_timestamps_;
    
    void checkBurstRate() {
        const auto& risk = config_.getRiskManagementConfig();
        uint32_t rate = getCurrentRate();
        
        if (risk.isBurstHardStop(rate)) {
            trading_enabled_ = false;  // Stop trading
        }
    }
};
```

## Command Line Usage

### With Configuration File (Recommended)

```bash
./build/client_algo --config sample_config.json
```

### With Command Line Arguments (Legacy)

```bash
./build/client_algo algo <md_host> <md_port> <ex_host> <ex_port> <num_messages>
```

**Example:**
```bash
./build/client_algo algo 127.0.0.1 12345 127.0.0.1 54321 1000
```

## Configuration File Location

Default search paths:
1. `./sample_config.json` (current directory)
2. `./config.json` (current directory)
3. `~/.beacon/client_algo_config.json` (home directory)
4. `/etc/beacon/client_algo_config.json` (system)

Or specify explicitly:
```bash
./build/client_algo --config /path/to/config.json
```

## Best Practices

### 1. Start Conservatively

```json
{
  "symbol": "AAPL",
  "starting_position": 0,
  "maximum_quantity": 100,
  "enabled": true
}
```

Start with small position limits and increase gradually.

### 2. Use Realistic Starting Positions

```json
{
  "symbol": "AAPL",
  "starting_position": 0,
  "maximum_quantity": 10000
}
```

Don't set `starting_position` that you don't actually have!

### 3. Disable Before Deleting

```json
{
  "symbol": "RISKY_STOCK",
  "starting_position": 0,
  "maximum_quantity": 1000,
  "enabled": false
}
```

Keep configuration but disable trading instead of deleting.

### 4. Document Your Limits

Add comments (JSON5 style) explaining position limits:

```json
{
  "symbol": "AAPL",
  "starting_position": 0,
  "maximum_quantity": 10000,  // $1.5M notional @ $150/share
  "enabled": true
}
```

### 5. Separate Configs for Different Strategies

```
configs/
├── day_trading.json       # Aggressive, IOC orders
├── overnight.json         # Conservative, Day orders
└── market_making.json     # High frequency, small size
```

## Troubleshooting

### Orders Not Being Sent

**Check:**
1. Product is enabled: `"enabled": true`
2. Position limits not exceeded
3. Symbol matches market data exactly (case-sensitive)

### Position Limit Exceeded Errors

```
Cannot buy AAPL: would exceed maximum position
Current: 9500, Requested: 1000, Max: 10000
```

**Solution:** Increase `maximum_quantity` or reduce order size

### Configuration Not Loading

**Common Issues:**
- JSON syntax error (missing comma, quote)
- Missing required field
- Invalid file path
- File permissions

**Debug:**
```bash
# Validate JSON syntax
cat sample_config.json | python3 -m json.tool
```

---

**Last Updated**: November 5, 2025  
**Author**: Bryan Camp  
**Version**: 1.0
