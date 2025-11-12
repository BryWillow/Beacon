# =============================================================================
# Project:      Beacon
# Application:  playback
# Purpose:      Build configuration for the market data playback application.
# Author:       Bryan Camp
# =============================================================================

# Minimum CMake version
cmake_minimum_required(VERSION 3.16)

# Project name
project(playback LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect sources
add_executable(playback
    main.cpp
    market_data_playback.cpp
    message_buffer.cpp
    playback_state.cpp
    advisors/message_priority_classifier.cpp
    authorities/playback_rule_interface.cpp
    authorities/rules_engine.cpp
    replayers/replayer_factory/message_sender_factory.cpp
    replayers/replayer_type.cpp
    replayers/replayer_types/console_replayer.cpp
    replayers/replayer_types/null_replayer.cpp
    replayers/replayer_types/tcp_replayer.cpp
    replayers/replayer_types/udp_replayer.cpp
)

# Include directories
target_include_directories(playback PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/advisors
    ${CMAKE_CURRENT_SOURCE_DIR}/authorities
    ${CMAKE_CURRENT_SOURCE_DIR}/replayers
    ${CMAKE_CURRENT_SOURCE_DIR}/replayers/replayer_factory
    ${CMAKE_CURRENT_SOURCE_DIR}/replayers/replayer_types
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Link nlohmann_json if available
find_package(nlohmann_json QUIET)
if (nlohmann_json_FOUND)
    target_link_libraries(playback PRIVATE nlohmann_json::nlohmann_json)
endif()

# Enable warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(playback PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optionally add tests here
# add_subdirectory(tests)