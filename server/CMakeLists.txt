# Server application for video processing bot

# Source files
set(SERVER_SOURCES
    src/main.cpp
    src/video_server.cpp
    src/video_service_impl.cpp
)

# Create server executable
add_executable(aa_video_server ${SERVER_SOURCES})

# Include directories
target_include_directories(aa_video_server
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/shared/include
)

# Link libraries
target_link_libraries(aa_video_server
    PRIVATE
        aa_shared
        ${OpenCV_LIBS}
        gRPC::grpc++
        protobuf::libprotobuf
)

# Set properties
set_target_properties(aa_video_server PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

# Installation
install(TARGETS aa_video_server
    RUNTIME DESTINATION bin
)
