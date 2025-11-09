cmake_minimum_required(VERSION 3.26)
project(client_algorithm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include paths
include_directories(${CMAKE_SOURCE_DIR}/../../../vendor)
include_directories(${CMAKE_SOURCE_DIR}/../../../include)

# Test receiver (TCP + UDP)
add_executable(client_algorithm main.cpp)

# Algorithm with order entry (OUCH protocol) - now with config support
add_executable(client_algo 
    main_algo.cpp
    config_provider.cpp
)

# HFT threaded algorithm with production architecture
add_executable(client_algo_hft
    main_algo_threaded.cpp
)

# TWAP execution algorithm (industry-standard reference implementation)
add_executable(algo_twap
    algo_twap.cpp
)

# Minimal template for developers to create their own algorithms
add_executable(algo_template
    algo_template.cpp
)

# Latency tracking example
add_executable(example_latency_tracking
    example_latency_tracking.cpp
)

# Protocol test clients
add_executable(test_pillar test_pillar.cpp)
add_executable(test_cme test_cme.cpp)

# Link pthread for threading support
find_package(Threads REQUIRED)
target_link_libraries(client_algo Threads::Threads)
target_link_libraries(client_algo_hft Threads::Threads)
target_link_libraries(algo_twap Threads::Threads)
target_link_libraries(algo_template Threads::Threads)
