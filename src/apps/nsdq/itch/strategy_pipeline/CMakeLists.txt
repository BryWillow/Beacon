cmake_minimum_required(VERSION 3.26)
project(nsdq_itch_strategy_pipeline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Absolute include paths
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(VENDOR_DIR  ${CMAKE_SOURCE_DIR}/vendor)

include_directories(${INCLUDE_DIR})
include_directories(${VENDOR_DIR}/json)

# Executable
add_executable(nsdq_itch_strategy_pipeline
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

# Link pthread if needed
target_link_libraries(nsdq_itch_strategy_pipeline PRIVATE pthread)
