cmake_minimum_required(VERSION 3.26)
project(md_generator LANGUAGES CXX)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the beacon_exchanges interface library
add_library(beacon_exchanges INTERFACE)
target_include_directories(beacon_exchanges INTERFACE
    ${CMAKE_SOURCE_DIR}/../../../src/libs/exchanges
)
target_compile_features(beacon_exchanges INTERFACE cxx_std_20)

# Define the md_generator executable
add_executable(md_generator
    main.cpp
    message_generator.cpp
    stats_manager.cpp
    config_provider.cpp # Ensure only ConfigProvider is used
)

# Include CLI11 if header-only
target_include_directories(md_generator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../../../src/libs/exchanges
    ${CMAKE_SOURCE_DIR}/../../../include
    ${CMAKE_SOURCE_DIR}/../../../vendor
)

# Link CLI11 if it is a compiled library
# Uncomment the following lines if CLI11 is not header-only
# find_package(CLI11 REQUIRED)
# target_link_libraries(md_generator PRIVATE CLI11::CLI11)

target_link_libraries(md_generator PRIVATE beacon_exchanges)
