cmake_minimum_required(VERSION 3.20)
project(BeaconSrc)

add_library(common INTERFACE)
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/common)

add_executable(nsdq_execution apps/nsdq/execution/main.cpp)
target_link_libraries(nsdq_execution PRIVATE common)

add_executable(nsdq_generator apps/nsdq/market_data/generator_main.cpp)
add_executable(nsdq_listener apps/nsdq/market_data/listener_main.cpp)
add_executable(nsdq_replayer apps/nsdq/market_data/replayer_main.cpp)

target_link_libraries(nsdq_generator PRIVATE common)
target_link_libraries(nsdq_listener PRIVATE common)
target_link_libraries(nsdq_replayer PRIVATE common)
