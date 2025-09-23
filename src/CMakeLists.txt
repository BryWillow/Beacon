# ---------------------------------------------------------------------------
# File        : CMakeLists.txt
# Project     : Beacon
# Description : Build configuration for all Beacon source apps
# Author      : Bryan Camp
# Notes       :
#   - Header-only 'common' library for shared includes
#   - Separate executables for NSDQ market data generator, listener, and replayer
#   - Vendor and include paths configured for MacOS / Linux
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.24)
project(Beacon LANGUAGES CXX)

# ============================== #
# Compiler and standard settings
# ============================== #
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ============================== #
# Common include paths
# ============================== #
include_directories(
    ${PROJECT_SOURCE_DIR}/include        # Main project headers
)

# ============================== #
# Common header-only library
# ============================== #
add_library(common INTERFACE)
target_include_directories(common INTERFACE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/vendor/json  # Include for json.hpp
)

# ============================== #
# NSDQ Execution binary
# ============================== #
add_executable(nsdq_execution
    src/apps/nsdq/execution/main.cpp
)
target_link_libraries(nsdq_execution PRIVATE common)

# ============================== #
# NSDQ Market Data binaries
# ============================== #
add_executable(nsdq_generator
    src/apps/nsdq/md_generator/md_generator.cpp
)
target_link_libraries(nsdq_generator PRIVATE common)

add_executable(nsdq_listener
    src/apps/nsdq/md_listener/md_listener.cpp
)
target_link_libraries(nsdq_listener PRIVATE common)

add_executable(nsdq_replayer
    src/apps/nsdq/md_playback/md_replayer.cpp
)
target_link_libraries(nsdq_replayer PRIVATE common)

# ============================== #
# Notes:
#   - All headers and utilities in 'include/' are available to executables via 'common'
#   - Vendor libraries (e.g., json.hpp) are included via common INTERFACE include
#   - Build works on Linux and MacOS; MSVC support intentionally omitted
# ============================== #
