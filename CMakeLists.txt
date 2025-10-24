# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [Top-Level] [CICD Pipeline]
# @file      CMakeLists.txt
# @brief     Options: debug(default)|release|all|clean|pipeline
#            This scripts builds all apps and can start the CICD pipeline.
#            Note that each app also has its own out-of-source build script.
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

# Global verbosity control (default: quiet)
option(BEACON_VERBOSE "Enable verbose CMake output (STATUS messages, rules)" OFF)

if(NOT BEACON_VERBOSE)
    # Hide STATUS messages and rule chatter
    set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
    set(CMAKE_RULE_MESSAGES OFF)
    set(CMAKE_VERBOSE_MAKEFILE OFF)
endif()

# Make project headers available to all targets
include_directories(${PROJECT_SOURCE_DIR}/include)

# On macOS with Apple Clang, enforce libc++ for compile/link consistency
if(APPLE)
    message(STATUS "Applying macOS libc++ flags for consistency with vendor libs")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif()

# ---------------------------------------------------------------------------
# Include apps
# ---------------------------------------------------------------------------
add_subdirectory(src/apps/md_creator)
add_subdirectory(src/apps/md_server)
add_subdirectory(src/apps/ex_match)
add_subdirectory(src/apps/md_client)

# Add more apps here:
# add_subdirectory(src/apps/nsdq/itch/strategy_pipeline)

# ---------------------------------------------------------------------------
# Optionally: set global output directories
# ---------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../../bin)

# Tests: make tests optional and robust if vendor/googletest is missing
option(BEACON_BUILD_TESTS "Enable building tests" ON)

if(BEACON_BUILD_TESTS)
    # Prefer vendored googletest if present
    if(EXISTS "${CMAKE_SOURCE_DIR}/vendor/googletest/CMakeLists.txt")
        message(STATUS "Using vendored googletest (vendor/googletest)")
        # Add vendored googletest so tests can link to GTest::gtest_main.
        # Assumes vendor/googletest is present and contains a CMake project.
        add_subdirectory(vendor/googletest EXCLUDE_FROM_ALL)
        set(HAVE_GTEST TRUE)
    else()
        # Try to find a system-installed GTest
        find_package(GTest QUIET)
        if(GTest_FOUND)
            message(STATUS "Found system GTest")
            set(HAVE_GTEST TRUE)
        else()
            message(WARNING "googletest not found in vendor/ and no system GTest found. Tests will be disabled.")
            message(WARNING "To enable tests: (1) add vendor/googletest (git submodule or copy) OR (2) install system GTest and re-run CMake.")
            set(BEACON_BUILD_TESTS OFF)
        endif()
    endif()
endif()

if(BEACON_BUILD_TESTS AND HAVE_GTEST)
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Tests disabled (BEACON_BUILD_TESTS=${BEACON_BUILD_TESTS})")
endif()
