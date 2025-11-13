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
    replayers/replayer_factory/replayer_factory.cpp
)

# Include directories
target_include_directories(playback PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/advisors
    ${CMAKE_CURRENT_SOURCE_DIR}/authorities
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/replayers
    ${CMAKE_CURRENT_SOURCE_DIR}/replayers/replayer_factory
    ${CMAKE_CURRENT_SOURCE_DIR}/replayers/replayer_types
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

# Enable warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(playback PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optionally add tests here
# add_subdirectory(tests)