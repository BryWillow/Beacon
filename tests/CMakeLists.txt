# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [tests]
# @file      CMakeLists.txt
# @author    Bryan Camp
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.26)
project(BeaconTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Include Beacon headers
# -----------------------------------------------------------------------------
include_directories(${CMAKE_SOURCE_DIR}/../include)

# -----------------------------------------------------------------------------
# Enable testing
# -----------------------------------------------------------------------------
enable_testing()
