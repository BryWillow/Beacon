cmake_minimum_required(VERSION 3.26)
project(md_generator LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(md_generator
    main.cpp
    message_generator.cpp
    # ...existing code...
)

# This is brittle, but it's intent is to be a simple way to build md_generator locally.
get_filename_component(VENDOR_PATH "${CMAKE_SOURCE_DIR}/../../../vendor" ABSOLUTE)
message(STATUS "Resolved vendor path: ${VENDOR_PATH}")

target_include_directories(md_generator PUBLIC
    ${CMAKE_SOURCE_DIR}/src/libs/exchanges
    ${CMAKE_SOURCE_DIR}/include
    ${VENDOR_PATH}
)

target_link_libraries(md_generator
    beacon_exchanges
    beacon_utils
)

target_compile_features(md_generator PUBLIC cxx_std_20)


