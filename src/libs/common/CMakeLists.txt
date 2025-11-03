cmake_minimum_required(VERSION 3.26)

project(beacon_common LANGUAGES CXX)

add_library(beacon_common STATIC
    beacon_common.cpp  # Only include if you have implementation code
)

target_include_directories(beacon_common PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(beacon_common PUBLIC cxx_std_20)
