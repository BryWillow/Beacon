cmake_minimum_required(VERSION 3.26)
project(BeaconTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# GoogleTest
# -----------------------------------------------------------------------------
# Assumes googletest is in vendor/googletest
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/googletest googletest_build)

# -----------------------------------------------------------------------------
# Test: Itch File Generator
# -----------------------------------------------------------------------------
set(TEST_NAME test_itch_file_generator)
add_executable(${TEST_NAME} tests/itch_file_generator/itch_file_generator.cpp)

# Include your project headers
target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link with GoogleTest
target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main Threads::Threads)

# Enable CTest
enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
