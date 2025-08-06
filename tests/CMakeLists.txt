# Tests CMakeLists.txt

# Ensure GTest is available
if(NOT GTest_FOUND)
    message(FATAL_ERROR "Google Test is required for building tests")
endif()

# Include shared library headers
include_directories(${CMAKE_SOURCE_DIR}/shared/include)
include_directories(${CMAKE_SOURCE_DIR}/server/include)

# Find required dependencies for server tests
# find_package(OpenCV REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Create the test executables
add_executable(test_signal_set
    test_signal_set.cpp
)

add_executable(test_detector_server
    test_detector_server.cpp
)

add_executable(test_options
    test_options.cpp
)

add_executable(test_copy_move_semantics
    test_copy_move_semantics.cpp
)

# Link against required libraries for signal set tests
target_link_libraries(test_signal_set
    aa_shared
    GTest::GTest
    GTest::Main
    pthread
)

# Link against required libraries for detector server tests
target_link_libraries(test_detector_server
    aa_server
    aa_shared
    ${OpenCV_LIBS}
    gRPC::grpc++
    protobuf::libprotobuf
    GTest::GTest
    GTest::Main
    GTest::gmock
    pthread
)

# Link against required libraries for options tests
target_link_libraries(test_options
    aa_shared
    ${OpenCV_LIBS}
    GTest::GTest
    GTest::Main
    pthread
)

# Link against required libraries for copy/move semantics tests
target_link_libraries(test_copy_move_semantics
    aa_shared
    ${OpenCV_LIBS}
    GTest::GTest
    GTest::Main
    pthread
)

# Add the tests to CTest
add_test(NAME SignalSetTests COMMAND test_signal_set)
add_test(NAME DetectorServerTests COMMAND test_detector_server)
add_test(NAME OptionsTests COMMAND test_options)
add_test(NAME CopyMoveSemantics COMMAND test_copy_move_semantics)

# Set test properties
set_tests_properties(SignalSetTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;signal_handling"
)

set_tests_properties(DetectorServerTests PROPERTIES
    TIMEOUT 60
    LABELS "unit;server;detector"
)

# Ensure the shared library is built before tests
add_dependencies(test_signal_set aa_shared)
add_dependencies(test_detector_server aa_server aa_shared)
add_dependencies(test_copy_move_semantics aa_shared)
