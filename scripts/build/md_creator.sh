# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [md_creator]
# @file      CMakeLists.txt
# @brief     Build configuration for the md_creator application
# @author    Bryan Camp
# ---------------------------------------------------------------------------

# This file is part of the Beacon project.
# Please refer to the top-level CMakeLists.txt for project-wide settings.

# ---------------------------------------------------------------------------
# Target: md_creator
# ---------------------------------------------------------------------------
add_executable(md_creator
    # Source files
    ${CMAKE_CURRENT_SOURCE_DIR}/src/md_creator.cpp
)

# ---------------------------------------------------------------------------
# Output directories
# ---------------------------------------------------------------------------
set_target_properties(md_creator
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/md_creator
)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
# target_link_libraries(md_creator ...)

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
install(TARGETS md_creator
    RUNTIME DESTINATION bin/md_creator
)

# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------
# add_test(NAME md_creator_test COMMAND md_creator --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/test_results.xml)