# Beacon Test Suite

Comprehensive GoogleTest suite for all Beacon components.

## Test Structure

```
tests/
├── core/                      # Core infrastructure tests
│   ├── test_ringbuffer.cpp           - SPSC ringbuffer tests
│   ├── test_tcp_server.cpp           - TCP server tests
│   ├── test_tcp_client.cpp           - TCP client tests
│   ├── test_udp_receiver.cpp         - UDP multicast receiver tests
│   ├── test_thread_utils.cpp         - Thread affinity & priority tests
│   ├── test_pinned_thread.cpp        - Pinned thread tests
│   └── test_latency_tracker.cpp      - Latency measurement tests
│
├── protocols/                 # Exchange protocol tests
│   ├── test_itch_messages.cpp        - NASDAQ ITCH message tests
│   ├── test_itch_serialization.cpp   - ITCH serialization tests
│   ├── test_ouch_messages.cpp        - NASDAQ OUCH message tests
│   ├── test_ouch_serialization.cpp   - OUCH serialization tests
│   ├── test_pillar_messages.cpp      - NYSE Pillar message tests
│   ├── test_pillar_serialization.cpp - Pillar serialization tests
│   ├── test_mdp_messages.cpp         - CME MDP message tests
│   └── test_mdp_serialization.cpp    - MDP serialization tests
│
├── matching_engine/           # Matching engine tests
│   ├── test_order_book.cpp           - Order book tests
│   ├── test_order_matching.cpp       - Matching logic tests
│   └── test_execution_reports.cpp    - Execution report tests
│
├── market_data/               # Market data generator tests
│   ├── test_message_generation.cpp   - Message generation tests
│   ├── test_serializers.cpp          - Serializer tests
│   └── test_symbol_parameters.cpp    - Symbol parameter tests
│
├── algorithm/                 # Client algorithm tests
│   ├── test_strategy_logic.cpp       - Strategy logic tests
│   ├── test_order_management.cpp     - Order management tests
│   └── test_risk_limits.cpp          - Risk limit tests
│
└── integration/               # Integration tests
    ├── test_end_to_end.cpp           - Full system integration
    └── test_tick_to_trade.cpp        - Latency integration tests
```

## Running Tests

### Run All Tests
```bash
./run_tests.sh
```

### Run Specific Test Suite
```bash
cd build
./tests/test_ringbuffer
./tests/test_networking
./tests/test_itch_protocol
```

### Run Tests with CTest
```bash
cd build
ctest --output-on-failure
```

### Run Specific Test with CTest
```bash
cd build
ctest -R ringbuffer --verbose
```

## Test Coverage

### Core Infrastructure ✅
- [x] SPSC Ringbuffer - Full coverage
- [x] TCP Server/Client - Basic coverage
- [x] UDP Multicast - Basic coverage
- [x] Threading utilities - Basic coverage
- [x] Latency tracker - Full coverage

### Exchange Protocols 🚧
- [x] ITCH Messages - Partial coverage
- [x] ITCH Serialization - Partial coverage
- [ ] OUCH Messages - TODO
- [ ] OUCH Serialization - TODO
- [ ] Pillar Messages - TODO
- [ ] Pillar Serialization - TODO
- [ ] MDP Messages - TODO
- [ ] MDP Serialization - TODO

### Matching Engine 📋
- [ ] Order Book - TODO
- [ ] Order Matching - TODO
- [ ] Execution Reports - TODO

### Market Data Generator 📋
- [ ] Message Generation - TODO
- [ ] Serializers - TODO
- [ ] Symbol Parameters - TODO

### Client Algorithm 📋
- [ ] Strategy Logic - TODO
- [ ] Order Management - TODO
- [ ] Risk Limits - TODO

### Integration Tests 📋
- [ ] End-to-End - TODO
- [ ] Tick-to-Trade Latency - TODO

## Adding New Tests

1. Create test file in appropriate directory:
```cpp
#include <gtest/gtest.h>

TEST(MyComponentTest, MyTestCase) {
    // Test implementation
    EXPECT_EQ(1, 1);
}
```

2. Add to `tests/CMakeLists.txt`:
```cmake
beacon_test(test_my_component
    category/test_my_component.cpp
)
```

3. Rebuild and run:
```bash
cd build
cmake ..
make -j8
ctest
```

## Performance Tests

The test suite includes performance benchmarks:
- **Ringbuffer throughput**: Should achieve >1M msgs/sec
- **Latency tracker overhead**: Minimal impact
- **Serialization speed**: Benchmarks for all protocols

## Continuous Integration

Tests are designed to run in CI/CD pipelines:
```bash
cmake -B build
cmake --build build -j8
cd build && ctest --output-on-failure
```

## Requirements

- CMake 3.26+
- C++20 compiler
- GoogleTest (vendored in `vendor/googletest`)
- Sufficient permissions for network tests (multicast)

## Notes

- Network tests use localhost multicast (239.255.0.x)
- Some tests require specific ports (19997-19999)
- Thread affinity tests require multi-core CPU
- Performance tests may vary by hardware
