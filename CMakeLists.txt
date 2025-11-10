# =============================================================================
# Project:      Beacon
# Purpose:      Root build configuration for the Beacon project
# Author:       Bryan Camp
# =============================================================================

cmake_minimum_required(VERSION 3.26)
project(Beacon LANGUAGES CXX)

# Set C++ standard for the entire project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add vendor directory to include paths for the entire project
include_directories(${CMAKE_SOURCE_DIR}/vendor)
include_directories(${CMAKE_SOURCE_DIR}/vendor/nlohmann)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add shared include directory for all targets
include_directories(${PROJECT_SOURCE_DIR}/include)

# Create a static library for shared utilities
add_library(beacon_utils STATIC
    include/utils/string_utils.cpp
)

target_include_directories(beacon_utils PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Link beacon_utils to your apps
# Example for exchange_matching_engine:
add_executable(exchange_matching_engine
    src/apps/exchange_matching_engine/main.cpp
    # ...other sources...
)
target_link_libraries(exchange_matching_engine beacon_utils)

# Repeat target_link_libraries for other apps:
add_executable(exchange_market_data_generator
    src/apps/exchange_market_data_generator/main.cpp
    # ...other sources...
)
target_link_libraries(exchange_market_data_generator beacon_utils)

add_executable(exchange_market_data_playback
    src/apps/exchange_market_data_playback/main.cpp
    # ...other sources...
)
target_link_libraries(exchange_market_data_playback beacon_utils)

add_executable(client_algorithm
    src/apps/client_algorithm/main_algo.cpp
    # ...other sources...
)
target_link_libraries(client_algorithm beacon_utils)

# Add subdirectories for libraries
add_subdirectory(src/libs/exchange_protocols)

# Add subdirectories for applications
add_subdirectory(src/apps/exchange_market_data_generator)
add_subdirectory(src/apps/exchange_market_data_playback)
add_subdirectory(src/apps/exchange_matching_engine)
add_subdirectory(src/apps/client_algorithm)

# Enable testing
enable_testing()

# Optional: Add GoogleTest if tests directory exists and has CMakeLists.txt
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(vendor/googletest)
    add_subdirectory(tests)
endif()
