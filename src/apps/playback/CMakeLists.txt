# =============================================================================
# Project:      Beacon
# Application:  playback
# Purpose:      Build configuration for the market data playback application.
# Author:       Bryan Camp
# =============================================================================

cmake_minimum_required(VERSION 3.26)
project(playback LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Note: This app can be built two ways:
# (1) As a standalone executable. 
# (2) With a larger Beacon build of other executables.
# 
# Regardless, we need to know the root of the repo for:
# vendor, config, etc.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # This is a standalone build. Adjust the REPO root.
    set(BEACON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
else()
    # This is part of a larger Beacon build.
    set(BEACON_ROOT "${CMAKE_SOURCE_DIR}")
endif()

# UGLY, FIXCollect sources using glob
file(GLOB PLAYBACK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_executable(playback ${PLAYBACK_SOURCES})

target_include_directories(playback PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces
    ${BEACON_ROOT}/include
    ${BEACON_ROOT}/vendor
)

# Add exchanges as an INTERFACE library for header-only usage
add_library(exchanges INTERFACE)
target_include_directories(exchanges INTERFACE
  ${CMAKE_SOURCE_DIR}/libs/exchanges
)

# Link exchanges to playback
target_link_libraries(playback PRIVATE exchanges)

# Enable warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(playback PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optionally add tests here
# add_subdirectory(tests)