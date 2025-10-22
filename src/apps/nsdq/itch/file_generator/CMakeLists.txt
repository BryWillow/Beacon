# ---------------------------------------------------------------------------
# @file      CMakeLists.txt
# @project   Beacon
# @component Beacon Build System [file_generator]
# @brief     Builds the file_generator component in debug|release configurations.
#            Debug builds include sanitizers, cppcheck, and debug symbols.
#            Release builds are fully optimized.
#            Output binaries go to bin/<config>/file_generator.
# @author    Bryan Camp
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.26)
project(file_generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compute repo root relative to this CMakeLists.txt
set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../")

# Only print diagnostics when verbose mode is on
if(BEACON_VERBOSE)
	message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
	message(STATUS "REPO_ROOT: ${REPO_ROOT}")
endif()

# Build executable
add_executable(file_generator main.cpp)

# Include directories for headers
target_include_directories(file_generator PRIVATE
    "${REPO_ROOT}/include"
    "${REPO_ROOT}/vendor"
)

# Verify include dirs
get_target_property(INCLUDE_DIRS file_generator INCLUDE_DIRECTORIES)
if(BEACON_VERBOSE)
	message(STATUS "file_generator include dirs: ${INCLUDE_DIRS}")
endif()

# Reduce build verbosity
set(CMAKE_RULE_MESSAGES OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Ensure bin layout uses lowercase config names
# bin/file_generator/{debug,release}/file_generator
set_target_properties(file_generator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${REPO_ROOT}/bin/file_generator/debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${REPO_ROOT}/bin/file_generator/release"
)
