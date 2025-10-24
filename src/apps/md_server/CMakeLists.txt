# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [md_server]
# @file      CMakeLists.txt
# @brief     Build configuration for the md_server application
# @author    Bryan Camp
# ---------------------------------------------------------------------------

# Find required packages
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    md_server.cpp
)

# Executable
add_executable(md_server md_server.cpp)

# Link libraries
target_link_libraries(md_server
    PRIVATE
        Boost::system
        Boost::filesystem
        Threads::Threads
)

# Install rules
install(TARGETS md_server
    RUNTIME DESTINATION bin
)