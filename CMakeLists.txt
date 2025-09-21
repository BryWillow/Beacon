cmake_minimum_required(VERSION 3.20)

project(MarketDataPipeline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#==============================#
# Include directories
#==============================#
include_directories(
    ${PROJECT_SOURCE_DIR}/src/common
)

#==============================#
# Common header-only library
#==============================#
add_library(common INTERFACE)
target_include_directories(common INTERFACE ${PROJECT_SOURCE_DIR}/src/common)

#==============================#
# NSDQ Execution binary
#==============================#
set(NSDQ_EXEC_SRC
    src/apps/nsdq/execution/main.cpp
)

add_executable(nsdq_execution ${NSDQ_EXEC_SRC})
target_link_libraries(nsdq_execution PRIVATE common)

#==============================#
# NSDQ Market Data binaries
#==============================#
set(NSDQ_MD_SRC
    src/apps/nsdq/market_data/generator_main.cpp
)

add_executable(nsdq_generator src/apps/nsdq/market_data/generator_main.cpp)
target_link_libraries(nsdq_generator PRIVATE common)

add_executable(nsdq_listener src/apps/nsdq/market_data/listener_main.cpp)
target_link_libraries(nsdq_listener PRIVATE common)

add_executable(nsdq_replayer src/apps/nsdq/market_data/replayer_main.cpp)
target_link_libraries(nsdq_replayer PRIVATE common)

