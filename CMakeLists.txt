cmake_minimum_required(VERSION 3.22)
project(Beacon VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header-only libraries
add_library(common INTERFACE)
target_include_directories(common INTERFACE src/common)

add_library(strategies INTERFACE)
target_include_directories(strategies INTERFACE src/strategies)

# Function to add apps
function(add_exchange_app app_name src_dir)
    file(GLOB APP_SRCS "${src_dir}/*.cpp")
    if(NOT APP_SRCS)
        message(WARNING "No sources found for app ${app_name} in ${src_dir}")
        return()
    endif()
    add_executable(${app_name} ${APP_SRCS})
    target_link_libraries(${app_name} PRIVATE common strategies)
endfunction()

# Market data apps
add_exchange_app(nsdq_market_data_v1_listener src/apps/nsdq/market_data/v1/listener)
add_exchange_app(nsdq_market_data_v1_replayer src/apps/nsdq/market_data/v1/replayer)
add_exchange_app(nsdq_market_data_v1_generator src/apps/nsdq/market_data/v1/generator)

# Execution app
add_exchange_app(nsdq_execution_v1 src/apps/nsdq/execution/v1/execution)
