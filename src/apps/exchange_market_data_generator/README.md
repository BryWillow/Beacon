# Exchange Market Data Generator

A high-performance C++20 application for generating realistic exchange-specific market data in binary format. Produces configurable volumes of order book updates and trade messages for NASDAQ ITCH, NYSE Pillar, and CME MDP protocols.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Quick Start](#quick-start)
- [Architecture](#architecture)
- [Building](#building)
- [Usage](#usage)
- [Configuration](#configuration)
- [Output Format](#output-format)
- [Market Microstructure](#market-microstructure)
- [Performance](#performance)
- [Project Structure](#project-structure)
- [Dependencies](#dependencies)
- [Troubleshooting](#troubleshooting)

## Overview

This tool generates synthetic market data that closely mimics real exchange message streams. It's designed for:

- **Testing and Development**: Test market data processing systems without live exchange connections
- **Performance Testing**: Generate high-volume message streams to benchmark data handlers
- **Protocol Implementation**: Validate exchange protocol parsers and decoders
- **Algorithm Development**: Create reproducible market scenarios for trading algorithm testing

The generator maintains realistic market microstructure including proper bid-ask spreads, weighted price distributions, and configurable trade frequencies.

## Features

### Exchange Protocol Support

- **NASDAQ ITCH v5.0**: Industry-standard market data protocol
- **NYSE Pillar v1.9**: NYSE's proprietary market data format
- **CME MDP MBO v4.0**: CME's Market Data Protocol (Market-By-Order)

### Configuration Options

- **Per-Symbol Parameters**: Customize price ranges, quantities, and spreads for each symbol
- **Message Distribution**: Configure the mix of orders vs. trades
- **Spread Control**: Set realistic bid-ask spreads as percentage of price
- **Volume Control**: Specify exact message counts or let the generator distribute intelligently
- **Performance Tuning**: Configurable flush intervals for optimal I/O performance

### Market Realism

- **Weighted Price Generation**: Prices cluster around the midpoint with configurable distribution
- **Spread Maintenance**: Automatic bid-ask spread calculation based on price levels
- **Trade Probability**: Configurable likelihood of trades vs. passive orders
- **Safety Checks**: Prevents bid-ask inversions and out-of-range prices

### Output & Monitoring

- **Binary Format**: Generates exchange-native binary message streams
- **Real-time Statistics**: Per-symbol tracking of orders, trades, volumes, and spreads
- **Clean Output**: Professional formatting with minimal noise
- **Fast Generation**: Typically 3-10M messages/second

## Quick Start

```bash
# Build the application
./build.sh

# Run with default configuration
./run.sh

# Or specify a custom configuration
./build/exchange_market_data_generator path/to/config.json
```

**Example Output:**
```
Generating messages (using sample_config.json)...

>>> File generation complete: output.itch <<<


═══════════════════════════════════════════════════════════════════════════════════════════════════════
                              MARKET DATA GENERATION SUMMARY                                           
═══════════════════════════════════════════════════════════════════════════════════════════════════════

  Symbol  Orders    Trades    TotalVol    AvgBid      AvgAsk      Spread      Spread%     Min-Max     
  ----------------------------------------------------------------------------------------------------
  GOOG    901       99        45873       1249.39     1250.02     0.64        0.05        99.79       
  AAPL    2728      272       136530      198.92      199.68      0.76        0.38        159.69      
  MSFT    5432      567       274748      149.19      150.28      1.09        0.72        114.87      
  ----------------------------------------                                                              
  TOTAL   9061      938       457151      

═══════════════════════════════════════════════════════════════════════════════════════════════════════
```

### Understanding the Output Summary

After generation completes, a statistics table is displayed with the following columns:

- **Symbol**: The ticker symbol
- **Orders**: Number of order messages (bid/ask quotes) generated
- **Trades**: Number of trade execution messages generated
- **TotalVol**: Total quantity across all orders for this symbol
- **AvgBid**: Volume-weighted average bid price (buy orders)
- **AvgAsk**: Volume-weighted average ask price (sell orders)
- **Spread**: Absolute price difference between AvgAsk and AvgBid (always positive)
- **Spread%**: Spread as a percentage of AvgAsk (measures market tightness)
- **Min-Max**: Price range observed (Max - Min)

The **spread** represents the bid-ask spread, a key measure of market liquidity. Tighter spreads (lower percentages) indicate more liquid markets. The spread is calculated as the difference between the average ask price and average bid price across all orders.

## Architecture

### Core Components

```
┌─────────────────┐
│   main.cpp      │  Entry point, argument parsing
└────────┬────────┘
         │
         ▼
┌─────────────────────────┐
│  ConfigProvider         │  Loads JSON config, creates serializer
└────────┬────────────────┘
         │
         ├──────────────────────────────────────┐
         │                                      │
         ▼                                      ▼
┌──────────────────────┐           ┌─────────────────────────┐
│  MessageGenerator    │           │  IMarketDataSerializer  │
│                      │           │  (Interface)            │
│ - Price generation   │           └──────────┬──────────────┘
│ - Order/trade logic  │                      │
│ - Market structure   │         ┌────────────┼────────────┐
└──────────┬───────────┘         │            │            │
           │                     ▼            ▼            ▼
           │              NsdqSerializer  CmeSerializer  NyseSerializer
           │                     │            │            │
           │                     └────────────┴────────────┘
           │                              │
           ▼                              ▼
    ┌──────────────┐           ┌──────────────────┐
    │ StatsManager │           │ Binary Output    │
    │              │           │ (.itch, .pillar, │
    │ - Order count│           │  .mdp)           │
    │ - Trade count│           └──────────────────┘
    │ - Volumes    │
    │ - Spreads    │
    └──────────────┘
```

### Key Design Patterns

- **Strategy Pattern**: `IMarketDataSerializer` interface allows pluggable exchange-specific serializers
- **Factory Pattern**: `ConfigProvider` instantiates the appropriate serializer based on configuration
- **Single Responsibility**: Each class has a focused purpose (generation, serialization, statistics, configuration)

## Building

### Prerequisites

- **Compiler**: C++20 compatible (GCC 10+, Clang 13+, AppleClang 12+, MSVC 19.29+)
- **CMake**: Version 3.26 or higher
- **Platform**: macOS, Linux (Windows untested but should work)

### Build Steps

```bash
# From the exchange_market_data_generator directory
./build.sh

# Or manually:
mkdir -p build && cd build
cmake ..
cmake --build .
```

The build script creates the `exchange_market_data_generator` executable in the `build/` directory.

## Usage

### Basic Usage

```bash
# Use default configuration (sample_config.json)
./run.sh

# Or run directly:
./build/exchange_market_data_generator

# Specify a custom configuration file
./build/exchange_market_data_generator path/to/config.json
```

### Provided Files

- **`sample_config.json`**: Example configuration with 3 symbols (GOOG, AAPL, MSFT)
- **`build.sh`**: Automated build script
- **`run.sh`**: Quick-run script using default configuration

## Configuration

Configuration is specified in JSON format. The generator supports both global defaults and per-symbol overrides.

### Configuration Schema

```json
{
  "exchange": "nsdq",
  "output_file": "output.itch",
  "total_messages": 10000,
  "flush_interval": 1000,
  "default_trade_probability": 0.1,
  "default_spread_percent": 0.5,
  "symbols": [
    {
      "symbol": "AAPL",
      "min_price": 150.0,
      "max_price": 200.0,
      "min_quantity": 100,
      "max_quantity": 1000,
      "message_count": 3000,
      "trade_probability": 0.12,
      "spread_percent": 0.3
    }
  ]
}
```

### Configuration Fields

#### Global Settings

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `exchange` | string | Yes | - | Exchange protocol: `"nsdq"`, `"nyse"`, or `"cme"` |
| `output_file` | string | Yes | - | Output filename (extension should match exchange) |
| `total_messages` | integer | Yes | - | Total number of messages to generate |
| `flush_interval` | integer | No | 1000 | Number of messages between buffer flushes |
| `default_trade_probability` | float | No | 0.1 | Default probability of generating trades (0.0-1.0) |
| `default_spread_percent` | float | No | 0.5 | Default bid-ask spread as percentage (e.g., 0.5 = 0.5%) |

#### Symbol Settings

| Field | Type | Required | Default | Description |
|-------|------|----------|---------|-------------|
| `symbol` | string | Yes | - | Symbol ticker (e.g., "AAPL") |
| `min_price` | float | Yes | - | Minimum price in dollars |
| `max_price` | float | Yes | - | Maximum price in dollars |
| `min_quantity` | integer | Yes | - | Minimum order/trade quantity |
| `max_quantity` | integer | Yes | - | Maximum order/trade quantity |
| `message_count` | integer | No | auto | Messages for this symbol (auto-distributed if omitted) |
| `trade_probability` | float | No | global | Override trade probability for this symbol |
| `spread_percent` | float | No | global | Override spread percentage for this symbol |

### Example Configurations

#### High-Frequency Trading Scenario

```json
{
  "exchange": "nsdq",
  "output_file": "hft_scenario.itch",
  "total_messages": 1000000,
  "flush_interval": 5000,
  "default_trade_probability": 0.05,
  "default_spread_percent": 0.1,
  "symbols": [
    {
      "symbol": "SPY",
      "min_price": 450.0,
      "max_price": 455.0,
      "min_quantity": 100,
      "max_quantity": 500,
      "spread_percent": 0.01
    }
  ]
}
```

#### Multi-Asset Portfolio

```json
{
  "exchange": "nyse",
  "output_file": "portfolio.pillar",
  "total_messages": 50000,
  "symbols": [
    {
      "symbol": "AAPL",
      "min_price": 170.0,
      "max_price": 180.0,
      "min_quantity": 100,
      "max_quantity": 1000,
      "message_count": 15000,
      "trade_probability": 0.15
    },
    {
      "symbol": "GOOGL",
      "min_price": 140.0,
      "max_price": 150.0,
      "min_quantity": 50,
      "max_quantity": 500,
      "message_count": 15000,
      "trade_probability": 0.10
    },
    {
      "symbol": "MSFT",
      "min_price": 400.0,
      "max_price": 430.0,
      "min_quantity": 100,
      "max_quantity": 800,
      "message_count": 20000,
      "trade_probability": 0.12
    }
  ]
}
```

## Output Format

### File Naming Conventions

- NASDAQ: `.itch` extension
- NYSE: `.pillar` extension
- CME: `.mdp` extension

### Binary Message Structure

Each message is serialized according to the exchange's native binary format:

- **NASDAQ ITCH**: Big-endian, fixed-width fields per ITCH v5.0 specification
- **NYSE Pillar**: Little-endian, variable-length messages per Pillar v1.9 specification
- **CME MDP**: Little-endian, SBE-encoded messages per MDP MBO v4.0 specification

### Price Encoding

Prices are encoded as fixed-point integers with 1/10,000 dollar precision:
- Example: $150.2534 → 1,502,534 (units of $0.0001)
- Range: $0.0001 to $429,496.7295 (32-bit unsigned integer)

## Market Microstructure

### Price Generation Algorithm

The generator uses a weighted random distribution to create realistic price clustering:

1. **Midpoint Calculation**: `mid = (min_price + max_price) / 2`
2. **Weighted Random**: Prices cluster around midpoint with configurable variance
3. **Bid-Ask Positioning**: 
   - Orders: Randomly assigned as bid or ask
   - Bids: Positioned slightly below generated price
   - Asks: Positioned slightly above generated price
4. **Spread Application**: Dynamic spread based on price level

### Spread Calculation

Spreads are calculated proportionally to price:

```
spread = price × (spread_percent / 100)
```

**Example**: For a $100 stock with 0.5% spread:
- Spread: $0.50
- If mid = $100.00, bid might be $99.75, ask might be $100.25

### Safety Mechanisms

- **Underflow Prevention**: Carefully handles unsigned integer arithmetic to avoid wraparound
- **Bid-Ask Validation**: Ensures bid < ask (though random generation may occasionally invert)
- **Range Clamping**: Prices are clamped to configured min/max bounds

## Performance

### Generation Speed

Typical performance on modern hardware (Apple M1/M2, Intel i7/i9):

- **1M messages**: ~0.1-0.3 seconds
- **10M messages**: ~1-3 seconds
- **100M messages**: ~10-30 seconds

Performance varies based on:
- Flush interval (higher = faster but more memory usage)
- Number of symbols (more symbols = more overhead)
- Trade probability (trades are slightly more expensive)

### Memory Usage

- **Minimal Footprint**: Streaming generation with configurable buffer sizes
- **Typical Usage**: <50MB for most scenarios
- **Large Runs**: Memory usage scales linearly with flush interval

### Optimization Tips

1. **Increase Flush Interval**: Reduce I/O overhead (default: 1000, try 5000-10000)
2. **Simplify Configuration**: Fewer symbols = faster generation
3. **Reduce Trade Probability**: Trades require additional calculations
4. **Use SSD Storage**: Faster disk writes improve overall performance

## Project Structure

```
exchange_market_data_generator/
├── README.md                    # This file
├── CMakeLists.txt              # Build configuration
├── build.sh                    # Build script
├── run.sh                      # Run script with default config
├── sample_config.json          # Example configuration
│
├── main.cpp                    # Entry point
├── config_provider.h/.cpp      # Configuration management
├── message_generator.h/.cpp    # Core generation logic
├── stats_manager.h/.cpp        # Statistics tracking
├── message.h                   # In-memory message structure
├── symbol_parameters.h         # Parameter structures
├── market_data_serializer.h    # Serializer interface
│
├── serializers/
│   ├── nsdq_serializer.h/.cpp  # NASDAQ ITCH serializer
│   ├── cme_serializer.h/.cpp   # CME MDP serializer
│   └── nyse_serializer.h/.cpp  # NYSE Pillar serializer
│
└── build/
    └── exchange_market_data_generator  # Compiled executable
```

## Dependencies

### Required Libraries

- **nlohmann/json**: JSON parsing (header-only, included in `/vendor/nlohmann/`)
- **Exchange Protocol Headers**: Located in `/src/libs/exchange_protocols/`
  - `nsdq/market_data/itch_v5/current/messages_active.h`
  - `nyse/market_data/pillar_market_data/current/messages_active.h`
  - `cme/market_data/mdp_mbo_v4/current/messages_active.h`

### System Requirements

- **C++ Standard Library**: Full C++20 support
- **POSIX**: For file I/O (standard on macOS/Linux)
- **CMake Modules**: Standard CMake installation

## Troubleshooting

### Build Issues

**Problem**: CMake version error
```
CMake Error: CMake 3.26 or higher is required
```
**Solution**: Install newer CMake via Homebrew (`brew install cmake`) or from cmake.org

**Problem**: C++20 features not available
```
error: 'concept' does not name a type
```
**Solution**: Ensure compiler supports C++20 (update compiler or check CMakeLists.txt)

**Problem**: Exchange protocol headers not found
```
fatal error: 'nsdq/market_data/itch_v5/current/messages_active.h' file not found
```
**Solution**: Ensure exchange protocol library is built first (run main project build)

### Runtime Issues

**Problem**: Config file not found
```
Error: Unable to open config file: config.json
```
**Solution**: Provide full path or ensure file exists in current directory

**Problem**: Invalid exchange type
```
Error: Unknown exchange type: 'nasdaq'
```
**Solution**: Use exact strings: `"nsdq"`, `"nyse"`, or `"cme"` (lowercase)

**Problem**: Unusually wide or narrow spreads
```
║ AAPL  ... Spread: 5.23% ║
```
**Solution**: Spreads are determined by the `spread_percent` configuration parameter. Adjust this value per symbol or globally to achieve desired spread characteristics. Typical values: 0.01%-1.0% for liquid stocks, 1.0%-5.0% for less liquid stocks.

### Performance Issues

**Problem**: Slow generation
**Solutions**:
- Increase `flush_interval` in configuration
- Reduce number of symbols
- Lower `trade_probability`
- Ensure output is to SSD, not network drive

**Problem**: High memory usage
**Solution**: Decrease `flush_interval` to flush buffers more frequently

## Related Projects

- **exchange_market_data_playback**: Companion tool for reading and replaying generated binary files
- **Exchange Protocol Library**: Core message definitions and utilities (`/src/libs/exchange_protocols/`)

## Contributing

When modifying this project, please maintain:

1. **Header Blocks**: All files should have the standard Beacon project header
2. **Include Order**: System → Third-party → Project libs → Local headers
3. **Documentation**: Complex algorithms should have inline comments
4. **Code Style**: Follow existing C++20 patterns and idioms

## License

Copyright © 2025 Bryan Camp. All rights reserved.

---

**Project**: Beacon  
**Application**: exchange_market_data_generator  
**Author**: Bryan Camp  
**Last Updated**: November 5, 2025
