# Client application for video processing bot

# Source files
set(CLIENT_SOURCES
    src/main.cpp
    src/video_client.cpp
)

# Create client executable
add_executable(aa_video_client ${CLIENT_SOURCES})

# Include directories
target_include_directories(aa_video_client
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/shared/include
)

# Link libraries
target_link_libraries(aa_video_client
    PRIVATE
        aa_shared
        ${OpenCV_LIBS}
        gRPC::grpc++
        protobuf::libprotobuf
)

# Set properties
set_target_properties(aa_video_client PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

# Installation
install(TARGETS aa_video_client
    RUNTIME DESTINATION bin
)
