# C++ CMake Project

A modern C++ project using CMake build system with testing support.

## Project Structure

```text
test_task/
├── CMakeLists.txt          # Main CMake configuration
├── include/                # Header files (empty)
├── src/                    # Source files
│   └── main.cpp           # Main application entry point
├── tests/                  # Test files (empty)
├── build/                  # Build directory (generated)
└── README.md              # This file
```

## Features

- **Modern C++23** standard
- **CMake 3.20+** build system
- **Google Test** integration ready for unit testing
- **Strict compiler warnings** enabled
- **Cross-platform** support
- **Cross-compilation**: Not currently supported, but planned for ARM64 Rockchip RK3588 SoC in the future

## Prerequisites

- CMake 3.20 or higher
- C++23 compatible compiler (GCC 11+, Clang 12+, MSVC 2022+)
- Google Test (optional, for running tests)

## Building the Project

### Using CMake directly

```bash
# Create build directory
mkdir build
cd build

# Configure the project
cmake ..

# Build the project
cmake --build .

# Run the executable
./test_task
```

### Using CMake with specific build type

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Running Tests

Currently no tests are available. To add tests:

1. Create test files in the `tests/` directory
2. Update `CMakeLists.txt` to include test files
3. Build and run tests as shown below:

```bash
# From build directory
ctest

# Or run tests directly
./test_task_tests
```

## Documentation

To generate API documentation using Doxygen:

```bash
# Configure with documentation enabled
cmake -B build -DBUILD_DOCUMENTATION=ON

# Build documentation
cmake --build build --target docs

# Open documentation
xdg-open build/docs/html/index.html
```

The documentation is automatically generated and published to GitHub Pages on every push to the main branch.

## VS Code Integration

This project includes VS Code configuration for:

- Building with CMake Tools extension
- Debugging with C/C++ extension
- Running tests with Test Explorer

### Recommended Extensions

- C/C++ Extension Pack
- CMake Tools
- Test Explorer UI

## Development

### Adding New Source Files

1. Add `.cpp` files to the `src/` directory
2. Add `.h` files to the `include/` directory
3. Update `CMakeLists.txt` to include new source files in the `SOURCES` variable

### Adding New Tests

1. Create test files in the `tests/` directory
2. Follow the existing test naming convention: `test_<module>.cpp`
3. Include the test source files in the test executable configuration

## Code Style

- Use modern C++23 features when appropriate
- Follow consistent naming conventions
- Include comprehensive documentation
- Write unit tests for new functionality

## Cross-Compilation

Currently, this project does not support cross-compilation. However, we plan to add support for cross-compilation targeting ARM64 Rockchip RK3588 SoC chips in future releases. This will enable building the project for embedded systems and ARM-based development boards.

**Note**: Cross-compilation support is planned but not yet implemented.

## License

This project is provided as a template for educational and development purposes.
