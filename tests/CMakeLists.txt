# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [tests]
# @file      CMakeLists.txt
# @author    Bryan Camp
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.26)
project(BeaconTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Include Beacon headers
# -----------------------------------------------------------------------------
include_directories(${CMAKE_SOURCE_DIR}/../include)

add_executable(beacon_tests test_itch_file_generator/itch_file_generator.cpp)

# Link the test executable to Google Test and your application library (if applicable)
target_link_libraries(beacon_tests gtest_main) # Link to gtest_main for provided main()

# Enable testing with CTest
enable_testing()

# Automatically discover tests if using Google Test's built-in test discovery
include(GoogleTest)
gtest_discover_tests(MyTests)
)