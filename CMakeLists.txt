cmake_minimum_required(VERSION 3.20)
project(Beacon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#------------------------------#
# Enable testing (if any)
#------------------------------#
enable_testing()

#------------------------------#
# Include src subdirectory
#   - Contains all libraries and executables
#------------------------------#
add_subdirectory(src)

#------------------------------#
# Include tests subdirectory (optional)
#------------------------------#
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory(tests)
endif()

