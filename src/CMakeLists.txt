cmake_minimum_required(VERSION 3.20)
project(BeaconSrc)

#------------------------------#
# Common headers (header-only library)
#------------------------------#
add_library(common INTERFACE)
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/common)

#------------------------------#
# NSDQ Execution v1 executable
#------------------------------#
add_executable(nsdq_execution_v1 apps/nsdq/execution/v1/main.cpp)
target_link_libraries(nsdq_execution_v1 PRIVATE common)

#------------------------------#
# NSDQ Market Data v1 executables
#------------------------------#
set(MD_DIR apps/nsdq/market_data/v1)
add_executable(nsdq_market_data_v1_generator ${MD_DIR}/generator_main.cpp)
add_executable(nsdq_market_data_v1_listener ${MD_DIR}/listener_main.cpp)
add_executable(nsdq_market_data_v1_replayer ${MD_DIR}/replayer_main.cpp)

target_link_libraries(nsdq_market_data_v1_generator PRIVATE common)
target_link_libraries(nsdq_market_data_v1_listener PRIVATE common)
target_link_libraries(nsdq_market_data_v1_replayer PRIVATE common)

