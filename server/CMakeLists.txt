# Server application for video processing bot

# Source files
set(SERVER_SOURCES
    src/main.cpp
    src/detector_server.cpp
)

# Create server executable
add_executable(detector_server ${SERVER_SOURCES})

# Include directories
target_include_directories(detector_server
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/shared/include
)

# Link libraries
target_link_libraries(detector_server
    PRIVATE
        aa::shared
        ${OpenCV_LIBS}
        gRPC::grpc++
        gRPC::grpc++_reflection
        protobuf::libprotobuf
)

# Set properties
set_target_properties(detector_server PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

# Installation
install(TARGETS detector_server
    RUNTIME DESTINATION bin
)
