# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [udp_replayer]
# @file      CMakeLists.txt
# @brief     Builds the udp_replayer component in debug|release configurations.
#            Debug builds include sanitizers, cppcheck, and debug symbols.
#            Release builds are fully optimized.
#            Output binaries go to bin/<config>/udp_replayer.
# @author    Bryan Camp
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.26)

# Project name (local to this subfolder)
project(nsdq_itch_udp_replayer LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Absolute include paths
# ---------------------------------------------------------------------------
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(VENDOR_JSON_DIR ${CMAKE_SOURCE_DIR}/vendor/json)

include_directories(${INCLUDE_DIR})
include_directories(${VENDOR_JSON_DIR})

# ---------------------------------------------------------------------------
# Build type flags
# ---------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -Wpedantic -g3 -O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -DNDEBUG)
endif()

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(nsdq_itch_udp_replayer
    ${CMAKE_SOURCE_DIR}/apps/nsdq/nsdq_itch_udp_replayer/main.cpp
)

# ---------------------------------------------------------------------------
# Link libraries
# ---------------------------------------------------------------------------
target_link_libraries(nsdq_itch_udp_replayer
    PRIVATE
    pthread
)
