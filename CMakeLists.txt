cmake_minimum_required(VERSION 3.26)
project(Beacon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Header-only common library
# -----------------------------------------------------------------------------
add_library(common INTERFACE)
target_include_directories(common INTERFACE ${PROJECT_SOURCE_DIR}/include)

# -----------------------------------------------------------------------------
# Threads (POSIX only)
# -----------------------------------------------------------------------------
find_package(Threads REQUIRED)

# -----------------------------------------------------------------------------
# NSDQ Applications
# -----------------------------------------------------------------------------
set(NSDQ_APPS
    nsdq_generator
    nsdq_listener
    nsdq_replayer
    nsdq_execution
)

foreach(app IN LISTS NSDQ_APPS)
    if(app STREQUAL "nsdq_generator")
        add_executable(${app} apps/nsdq/nsdq_generator/md_generator.cpp)
    elseif(app STREQUAL "nsdq_listener")
        add_executable(${app} apps/nsdq/nsdq_market_data/listener.cpp)
    elseif(app STREQUAL "nsdq_replayer")
        add_executable(${app} apps/nsdq/nsdq_market_data/replayer.cpp)
    elseif(app STREQUAL "nsdq_execution")
        add_executable(${app} apps/nsdq/nsdq_execution/main.cpp)
    endif()

    target_link_libraries(${app} PRIVATE common Threads::Threads)
endforeach()

# -----------------------------------------------------------------------------
# Enable testing
# -----------------------------------------------------------------------------
enable_testing()

# -----------------------------------------------------------------------------
# GoogleTest (local copy)
# -----------------------------------------------------------------------------
add_subdirectory(vendor/googletest)

# -----------------------------------------------------------------------------
# Unit Tests
# -----------------------------------------------------------------------------
add_executable(test_itch_file_generator
  tests/itch_file_generator/itch_file_generator.cpp
)
target_link_libraries(test_itch_file_generator
  PRIVATE common Threads::Threads gtest gtest_main
)

add_test(NAME test_itch_file_generator
         COMMAND test_itch_file_generator)
