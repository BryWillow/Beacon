cmake_minimum_required(VERSION 3.26)
project(matching_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main executable
add_executable(matching_engine main.cpp)

# Link pthread for threading support
find_package(Threads REQUIRED)
target_link_libraries(matching_engine Threads::Threads)
