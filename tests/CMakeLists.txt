# Tests CMakeLists.txt

# Ensure GTest is available
if(NOT GTest_FOUND)
    message(FATAL_ERROR "Google Test is required for building tests")
endif()

# Include shared library headers
include_directories(${CMAKE_SOURCE_DIR}/shared/include)

# Create the test executable
add_executable(test_signal_set
    test_signal_set.cpp
)

# Link against required libraries
target_link_libraries(test_signal_set
    aa_shared
    GTest::GTest
    GTest::Main
    pthread
)

# Add the test to CTest
add_test(NAME SignalSetTests COMMAND test_signal_set)

# Set test properties
set_tests_properties(SignalSetTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;signal_handling"
)

# Ensure the shared library is built before tests
add_dependencies(test_signal_set aa_shared)
