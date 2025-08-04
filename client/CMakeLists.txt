# Source files
set(CLIENT_SOURCES
    src/main.cpp
    src/detector_client.cpp
)

# Create client executable
add_executable(detector_client ${CLIENT_SOURCES})

# Include directories
target_include_directories(detector_client
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/shared/include
)

# Link libraries
target_link_libraries(detector_client
    PRIVATE
        aa::shared
        ${OpenCV_LIBS}
        gRPC::grpc++
        protobuf::libprotobuf
)

# Set properties
set_target_properties(detector_client PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

# Installation
install(TARGETS detector_client
    RUNTIME DESTINATION bin
)
