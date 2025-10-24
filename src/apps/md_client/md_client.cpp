// ---------------------------------------------------------------------------
// @project   Beacon
// @component Build Script [App] [md_client]
// @file      CMakeLists.txt
// @brief     CMake configuration for the md_client application
// @author    Bryan Camp
// ---------------------------------------------------------------------------

# Specify the target name
set(TARGET_NAME md_client)

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/${TARGET_NAME}.cpp"
)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/apps/md_client/include
)

# ---------------------------------------------------------------------------
# Build rules
# ---------------------------------------------------------------------------
add_executable(${TARGET_NAME} ${SOURCES})

# ---------------------------------------------------------------------------
# Target properties
# ---------------------------------------------------------------------------
set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../../bin/${CMAKE_BUILD_TYPE}"
)

# ---------------------------------------------------------------------------
# Link libraries
# ---------------------------------------------------------------------------
target_link_libraries(${TARGET_NAME}
    PRIVATE
        # Add private dependencies here
    PUBLIC
        # Add public dependencies here
)

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)

# ---------------------------------------------------------------------------
# Testing rules
# ---------------------------------------------------------------------------
if(BEACON_BUILD_TESTS AND HAVE_GTEST)
    # Add tests for this application here
    # Example:
    # add_executable(${TARGET_NAME}_test test/${TARGET_NAME}_test.cpp)
    # target_link_libraries(${TARGET_NAME}_test PRIVATE GTest::gtest GTest::gtest_main)
endif()