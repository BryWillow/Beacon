# =============================================================================
# Project:      Beacon Tests
# Purpose:      CMake configuration for GoogleTest suite
# Author:       Bryan Camp
# =============================================================================

cmake_minimum_required(VERSION 3.26)

# Include GoogleTest
include_directories(${CMAKE_SOURCE_DIR}/vendor/googletest/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/vendor/googletest/googlemock/include)
include_directories(${CMAKE_SOURCE_DIR}/src)  # Add src for protocol headers
include_directories(${CMAKE_SOURCE_DIR}/src/libs/exchange_protocols)  # Add protocol root for includes

# Test executable macro
macro(beacon_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} gtest gtest_main pthread)
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

# =============================================================================
# Core Infrastructure Tests
# =============================================================================

# Ringbuffer Tests
beacon_test(test_ringbuffer
    core/test_ringbuffer.cpp
)

# Networking Tests
beacon_test(test_networking
    core/test_tcp_server.cpp
    core/test_tcp_client.cpp
    core/test_udp_receiver.cpp
)

# Threading Tests
beacon_test(test_threading
    core/test_thread_utils.cpp
    core/test_pinned_thread.cpp
)

# Latency Tracker Tests
beacon_test(test_latency_tracker
    core/test_latency_tracker.cpp
)

# =============================================================================
# Exchange Protocol Tests
# =============================================================================

# NASDAQ ITCH Tests
beacon_test(test_itch_protocol
    protocols/test_itch_messages.cpp
    protocols/test_itch_serialization.cpp
)

# NASDAQ OUCH Tests
beacon_test(test_ouch_protocol
    protocols/test_ouch_messages.cpp
    protocols/test_ouch_serialization.cpp
)

# NYSE Pillar Tests
beacon_test(test_pillar_protocol
    protocols/test_pillar_messages.cpp
    protocols/test_pillar_serialization.cpp
)

# CME MDP Tests
beacon_test(test_mdp_protocol
    protocols/test_mdp_messages.cpp
    protocols/test_mdp_serialization.cpp
)

# =============================================================================
# Matching Engine Tests
# =============================================================================

beacon_test(test_matching_engine
    matching_engine/test_order_book.cpp
    matching_engine/test_order_matching.cpp
    matching_engine/test_execution_reports.cpp
)

# =============================================================================
# Market Data Generator Tests
# =============================================================================

beacon_test(test_market_data_generator
    market_data/test_message_generation.cpp
    market_data/test_serializers.cpp
    market_data/test_symbol_parameters.cpp
)

# =============================================================================
# Client Algorithm Tests
# =============================================================================

beacon_test(test_client_algorithm
    algorithm/test_strategy_logic.cpp
    algorithm/test_order_management.cpp
    algorithm/test_risk_limits.cpp
)

# =============================================================================
# Integration Tests
# =============================================================================

beacon_test(test_integration
    integration/test_end_to_end.cpp
    integration/test_tick_to_trade.cpp
)
