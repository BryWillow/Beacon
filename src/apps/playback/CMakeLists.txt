# =============================================================================
# Project:      Beacon
# Application:  playback
# Purpose:      Build configuration for the market data playback application.
# Author:       Bryan Camp
# =============================================================================

# Minimum CMake version
cmake_minimum_required(VERSION 3.26)

# Project name
project(playback)

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
add_executable(playback
    main.cpp
    message_senders/message_sender_factory/message_sender_factory.cpp
    playback_types/playback_type_factory.cpp
)

# Include directories relative to the Beacon root
target_include_directories(playback
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${BEACON_ROOT}/hft
        ${BEACON_ROOT}/src/apps/playback/message_senders
        ${BEACON_ROOT}/src/apps/playback/message_senders/message_sender_factory
        ${BEACON_ROOT}/src/apps/playback/playback_advisors
        ${BEACON_ROOT}/src/apps/playback/playback_authorities
        ${BEACON_ROOT}/src/apps/playback/playback_types
        ${BEACON_ROOT}/vendor/nlohmann
        ${BEACON_ROOT}/vendor
)