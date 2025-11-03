cmake_minimum_required(VERSION 3.26)
project(beacon_utils LANGUAGES CXX)

add_library(beacon_utils STATIC
    hton_utils.cpp
    sp_sc_queue.cpp
    # ...other sources...
)

target_include_directories(beacon_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(beacon_utils PUBLIC cxx_std_20)
