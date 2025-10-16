# ---------------------------------------------------------------------------
# @file      CMakeLists.txt
# @project   Beacon
# @component Top-Level Build
# @brief     Aggregates all app CMakeLists without modifying them
# @author    Bryan Camp
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.26)
project(Beacon LANGUAGES CXX)

# Default to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally allow build type from command line
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug/Release)" FORCE)
endif()

message(STATUS "Top-level CMake build type: ${CMAKE_BUILD_TYPE}")

# ---------------------------------------------------------------------------
# Include apps
# ---------------------------------------------------------------------------
add_subdirectory(src/apps/nsdq/itch/file_generator)

# Add more apps here:
# add_subdirectory(src/apps/nsdq/itch/matching_engine)
# add_subdirectory(src/apps/nsdq/itch/strategy_pipeline)
# add_subdirectory(src/apps/nsdq/itch/udp_replayer)

# ---------------------------------------------------------------------------
# Optionally: set global output directories
# ---------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../../bin)
