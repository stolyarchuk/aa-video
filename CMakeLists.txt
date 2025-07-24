cmake_minimum_required(VERSION 3.20)

# Project definition
project(test_task VERSION 1.0.0 LANGUAGES CXX)

# NOTE: Cross-compilation is not currently supported but planned for ARM64 Rockchip RK3588 SoC
# TODO: Add cross-compilation toolchain support for embedded targets

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compile flags for better development experience
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/calculator.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Target include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest)

if(GTest_FOUND)
    # Test executable
    add_executable(${PROJECT_NAME}_tests
        tests/test_calculator.cpp
        src/calculator.cpp
    )

    target_include_directories(${PROJECT_NAME}_tests PRIVATE include)
    target_link_libraries(${PROJECT_NAME}_tests GTest::gtest_main)

    # Add test
    add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
else()
    message(WARNING "Google Test not found. Tests will not be built.")
endif()

# Documentation
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)

if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        # Set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # Request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # Note the option ALL which allows to build the docs together with the application
        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
    else(DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif(DOXYGEN_FOUND)
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
