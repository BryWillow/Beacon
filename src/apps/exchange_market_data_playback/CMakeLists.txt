# =============================================================================
# Project:      Beacon
# Application:  exchange_market_data_playback
# Purpose:      Build configuration for the market data playback application.
# Author:       Bryan Camp
# =============================================================================

# Minimum CMake version
cmake_minimum_required(VERSION 3.26)

# Project name
project(exchange_market_data_playback)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Determine the Beacon root directory
# If this is a standalone build, we need to go up 3 levels: app -> apps -> src -> experimental
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Standalone build
    set(BEACON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
else()
    # Part of larger build
    set(BEACON_ROOT "${CMAKE_SOURCE_DIR}")
endif()

# Add the executable target
add_executable(exchange_market_data_playback
    main.cpp
)

# Include directories relative to the Beacon root
target_include_directories(exchange_market_data_playback PRIVATE
    ${BEACON_ROOT}/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link any required libraries (if applicable)
# Example: target_link_libraries(exchange_market_data_playback <library_name>)
