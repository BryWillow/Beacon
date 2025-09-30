# ---------------------------------------------------------------------------
# File        : CMakeLists.txt
# Project     : Beacon
# Author      : Bryan Camp
# Description : Top-level build configuration for the Beacon Market Data Pipeline.
#               Cross-platform (Linux/MacOS) modern CMake setup for low-latency
#               HFT-style applications.
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.24)
project(Beacon LANGUAGES CXX)

# ------------------------------
# Compiler & language settings
# ------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------
# Include directories
# ------------------------------
# Add include/ to all targets
include_directories(${PROJECT_SOURCE_DIR}/include)

# Vendor includes (e.g., json.hpp)
include_directories(${PROJECT_SOURCE_DIR}/vendor/json)

# ------------------------------
# Common header-only library
# ------------------------------
add_library(common INTERFACE)
target_include_directories(common INTERFACE
    ${PROJECT_SOURCE_DIR}/include
)

# ------------------------------
# NSDQ Execution binary
# ------------------------------
add_executable(nsdq_execution
    src/apps/nsdq/execution/main.cpp
)
target_link_libraries(nsdq_execution PRIVATE common)

# ------------------------------
# NSDQ Market Data binaries
# ------------------------------
# Generator
add_executable(nsdq_generator
    src/apps/nsdq/md_generator/md_generator.cpp
)
target_link_libraries(nsdq_generator PRIVATE common)

# Listener
add_executable(nsdq_listener
    src/apps/nsdq/md_listener/md_listener.cpp
)
target_link_libraries(nsdq_listener PRIVATE common)

# Replayer
add_executable(nsdq_replayer
    src/apps/nsdq/md_playback/md_replayer.cpp
)
target_link_libraries(nsdq_replayer PRIVATE common)

# ------------------------------
# NSDQ Matching Engine binary
# ------------------------------
add_executable(nsdq_matching_engine
    src/apps/nsdq/matching_engine/main.cpp
)
target_link_libraries(nsdq_matching_engine PRIVATE common)

# ------------------------------
# Notes
# ------------------------------
# - All include paths use ${PROJECT_SOURCE_DIR}/include
# - Vendor headers (json.hpp) added explicitly to include path
# - Header-only "common" library provides shared utilities
# - Cross-platform: works on Linux and MacOS
# - Future: add multi-feed support by creating separate targets
