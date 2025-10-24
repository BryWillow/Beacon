# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [md_creator]
# @file      CMakeLists.txt
# @brief     CMake configuration for the md_creator application
# @author    Bryan Camp
# ---------------------------------------------------------------------------

# Specify the target executable
add_executable(md_creator md_creator.cpp)

# Specify any required libraries
target_link_libraries(md_creator PRIVATE some_required_library)

# Specify any include directories
target_include_directories(md_creator PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Set any necessary compile options
target_compile_options(md_creator PRIVATE -Wall -Wextra)

# Add any necessary preprocessor definitions
target_compile_definitions(md_creator PRIVATE SOME_DEFINE=1)

# Install rules
install(TARGETS md_creator DESTINATION bin)