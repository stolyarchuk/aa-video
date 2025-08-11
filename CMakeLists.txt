cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0146 OLD)

# Project definition
project(aa_video_processing VERSION 1.0.0 LANGUAGES CXX)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add compile flags for better development experience
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Werror -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -Wno-error -Werror=empty-body")

# Build options
option(BUILD_CLIENT "Build client application" ON)
option(BUILD_SERVER "Build server application" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_DOCS "Create and install the HTML based API documentation (requires Doxygen)" OFF)

# Find required packages
if(BUILD_TESTS OR BUILD_CLIENT OR BUILD_SERVER)
    find_package(PkgConfig REQUIRED)
    find_package(Protobuf REQUIRED)
    find_package(gRPC REQUIRED)
    find_package(OpenCV REQUIRED)
endif()

if(BUILD_TESTS OR BUILD_CLIENT OR BUILD_SERVER)
    add_subdirectory(shared)
endif()

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(BUILD_SERVER)
    add_subdirectory(server)
endif()

if(BUILD_TESTS AND BUILD_CLIENT AND BUILD_SERVER)
    enable_testing()
    find_package(GTest)
    add_subdirectory(tests)
endif()

if(BUILD_DOCS)
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
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/output/output.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/docs/html/output)
    else()
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif()
endif()
