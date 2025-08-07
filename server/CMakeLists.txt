# Server application for video processing bot
cmake_policy(SET CMP0146 OLD)

# Source files
set(SERVER_LIB_SOURCES
    src/detector_server.cpp
)

set(SERVER_MAIN_SOURCES
    src/main.cpp
)

# Create server library for testing
add_library(aa_server ${SERVER_LIB_SOURCES})

# Include directories for library
target_include_directories(aa_server
    PUBLIC
        include
        ${CMAKE_SOURCE_DIR}/shared/include
)

# Link libraries for server library
target_link_libraries(aa_server
    PUBLIC
        aa::shared
        ${OpenCV_LIBS}
        gRPC::grpc++
        gRPC::grpc++_reflection
        protobuf::libprotobuf
)

# Create server executable
add_executable(detector_server ${SERVER_MAIN_SOURCES})

# Include directories for executable
target_include_directories(detector_server
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/shared/include
)

# Link libraries for executable
target_link_libraries(detector_server
    PRIVATE
        aa_server
)

# Set properties for library
set_target_properties(aa_server PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

# Set properties for executable
set_target_properties(detector_server PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

# Create alias for consistent naming
add_library(aa::server ALIAS aa_server)

# Installation
install(TARGETS detector_server aa_server
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
