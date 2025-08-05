# Shared library for common video processing functionality

# Protocol buffer definitions
set(PROTO_FILES
    proto/detector_service.proto
    proto/point.proto
    proto/polygon.proto
)

# Generate protobuf and gRPC files
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    get_filename_component(PROTO_PATH ${PROTO_FILE} PATH)

    set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
    set(GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE})
        add_custom_command(
            OUTPUT "${PROTO_SRCS}" "${PROTO_HDRS}" "${GRPC_SRCS}" "${GRPC_HDRS}"
            COMMAND protobuf::protoc
            ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
                 --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
                 -I "${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_PATH}"
                 --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
                 "${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}"
        )
    endif()
endforeach()

# Source files for shared library
set(SHARED_SOURCES
    src/options.cpp
    src/signal_set.cpp
)

# Add generated protobuf sources if proto files exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/proto/detector_service.proto)
    list(APPEND SHARED_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/detector_service.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/detector_service.grpc.pb.cc
    )
endif()

# Create shared library
add_library(aa_shared STATIC ${SHARED_SOURCES})

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
