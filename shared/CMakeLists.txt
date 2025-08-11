# Shared library for common video processing functionality

# Protocol buffer definitions
set(PROTO_FILES
    point.proto
    frame.proto
    polygon.proto
    detector_service.proto
)

# Generate protobuf and gRPC files
set(PROTO_GENERATED_SOURCES)
set(PROTO_GENERATED_HEADERS)

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
    set(GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/proto/${PROTO_FILE})
        add_custom_command(
            OUTPUT "${PROTO_SRCS}" "${PROTO_HDRS}" "${GRPC_SRCS}" "${GRPC_HDRS}"
            COMMAND protobuf::protoc
            ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
                 --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
                 -I "${CMAKE_CURRENT_SOURCE_DIR}/proto"
                 --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
                 "${CMAKE_CURRENT_SOURCE_DIR}/proto/${PROTO_FILE}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/proto/${PROTO_FILE}"
        )

        list(APPEND PROTO_GENERATED_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc)
        list(APPEND PROTO_GENERATED_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h)

        # Only add gRPC files for service definitions
        if(${PROTO_NAME} STREQUAL "detector_service")
            list(APPEND PROTO_GENERATED_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc)
            list(APPEND PROTO_GENERATED_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h)
        endif()
    endif()
endforeach()

# Create a custom target for protobuf generation
add_custom_target(proto_generation DEPENDS ${PROTO_GENERATED_SOURCES} ${PROTO_GENERATED_HEADERS})

# Source files for shared library
set(SHARED_SOURCES
    src/common.cpp
    src/options.cpp
    src/signal_set.cpp
    src/point.cpp
    src/logging.cpp
    src/frame.cpp
    src/polygon.cpp
    ${PROTO_GENERATED_SOURCES}
)

# Create shared library
add_library(aa_shared STATIC ${SHARED_SOURCES})

# Add dependency on protobuf generation
add_dependencies(aa_shared proto_generation)

# Create namespaced alias for consistent usage
add_library(aa::shared ALIAS aa_shared)

# Include directories
target_include_directories(aa_shared
    PUBLIC
        include
        ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE
        src
        ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(aa_shared
    PUBLIC
        ${OpenCV_LIBS}
        gRPC::grpc++
        protobuf::libprotobuf
    PRIVATE
)

# Set properties
set_target_properties(aa_shared PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
