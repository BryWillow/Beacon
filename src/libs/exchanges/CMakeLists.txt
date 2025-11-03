cmake_minimum_required(VERSION 3.26)
project(beacon_exchanges LANGUAGES CXX)

add_library(beacon_exchanges INTERFACE)

target_include_directories(beacon_exchanges INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(beacon_exchanges INTERFACE cxx_std_20)