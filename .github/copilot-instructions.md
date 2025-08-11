<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# C++ CMake Project Instructions

This is a modern C++ project using CMake as the build system. Please follow these guidelines when working with this codebase:

## Code Style and Standards

- **C++ Standard**: Use C++23 features and modern C++ best practices
- **Style Guide**: Follow Google C++ Style Guide with 120 character line limit
- **Formatting**: Use `.clang-format` configuration for automatic code formatting
- **Documentation Style**: Be concise and technically professional. No verbose explanations. Use precise, direct language for comments, README files, and documentation.
- **Naming Conventions** (Google Style):
  - Classes/Types: PascalCase (e.g., `Calculator`, `MyClass`)
  - Functions/Methods: PascalCase (e.g., `AddNumbers`, `GetValue`, `YoloPostProcessing`)
  - Variables: snake_case (e.g., `result_value`, `user_count`)
  - Constants: kConstantName (e.g., `kMaxValue`, `kDefaultTimeout`)
  - Member variables: snake_case with trailing underscore (e.g., `value_`, `count_`)
  - Namespaces: snake_case (e.g., `aa`)
  - Macros: UPPER_SNAKE_CASE (e.g., `MY_MACRO`)
- **Project Namespace**: Use `aa` as the main project namespace for all code.
- **Nested Namespaces**: Use concatenated nested namespace syntax (C++17+) for future code. Write `namespace aa::detection::yolo {` instead of separate nested declarations like `namespace aa { namespace detection { namespace yolo {`
- **Constructor Initialization**: Always use curly brackets `{}` for member initialization lists instead of parentheses `()`
- **Constructor Parameters**: Use "pass by value and then move" idiom for constructor parameters where acceptable (e.g., for expensive-to-copy types like strings, containers, complex objects)

## Header Ordering

- **Always follow this include order** with blank lines between groups:
  1. **System includes**: All standard library headers (e.g., `<chrono>`, `<mutex>`, `<unistd.h>`, `<iostream>`, `<string>`, `<vector>`)
  2. **Third-party includes**: External library headers (e.g., `<opencv2/opencv.hpp>`, `<grpcpp/grpcpp.h>`, `<gtest/gtest.h>`)
  3. **Local project includes**: Project-specific headers (e.g., `"detector_client.h"`, `"options.h"`, `"logging.h"`)
- **Sort alphabetically** within each group
- **Add blank lines** between groups for better readability
- **Test files exception**: In test files, always place `<gtest/gtest.h>` first among all includes, before system includes
- **Example**:

  ```cpp
  #include <chrono>
  #include <iostream>
  #include <string>
  #include <thread>

  #include <opencv2/opencv.hpp>
  #include <grpcpp/grpcpp.h>

  #include "detector_client.h"
  #include "logging.h"
  #include "options.h"
  ```

- **Test file example**:

  ```cpp
  #include <gtest/gtest.h>

  #include <chrono>
  #include <string>
  #include <vector>

  #include <opencv2/opencv.hpp>

  #include "detector_server.h"
  #include "options.h"
  ```

## Header Guards

- **Always use `#pragma once`** for header guards in all `.h` files
- Place `#pragma once` as the very first line in header files
- Do not use traditional `#ifndef`/`#define`/`#endif` header guards
- If a header file is missing header guards, add `#pragma once` immediately
- If a header file is missing header guards, add `#pragma once` immediately

## Project Structure

- **Headers**: Place in `include/` directory with `.h` extension
- **Source files**: Place in `src/` directory with `.cpp` extension
- **Tests**: Place in `tests/` directory with `test_` prefix
- **Build files**: Generated in `build/` directory (not tracked in git)

## CMake Guidelines

- Always update `CMakeLists.txt` when adding new source files
- Add new sources to the `SOURCES` variable
- For new executables, create separate `add_executable()` calls
- Use `target_include_directories()` for include paths
- Use `target_link_libraries()` for linking

## Testing

- Use Google Test framework for unit testing
- Create test files with `test_` prefix (e.g., `test_calculator.cpp`)
- Use descriptive test names that explain what is being tested
- Test both positive and negative cases
- Include edge cases and error conditions

## Error Handling

- Use exceptions for error conditions (e.g., `std::invalid_argument`)
- Prefer RAII for resource management
- Use smart pointers when dynamic allocation is needed

## Logging

- **Always use AA_LOG_ macros** from `logging.h` instead of `std::cout`, `std::cerr`, or `printf`
- Include `logging.h` header when using logging functionality
- **Available log levels**:
  - `AA_LOG_ERROR("message")` - For error conditions and failures
  - `AA_LOG_WARNING("message")` - For warnings and potential issues
  - `AA_LOG_INFO("message")` - For general information and status updates
  - `AA_LOG_DEBUG("message")` - For detailed debugging information
- **Usage examples**:

  ```cpp
  #include "logging.h"

  // Error logging
  AA_LOG_ERROR("Failed to load model: " << error_message);

  // Info logging with variables
  AA_LOG_INFO("Processing completed with " << count << " results");

  // Debug logging (can be compiled out in release builds)
  AA_LOG_DEBUG("Network output dims: " << dims << ", size: " << size);
  ```

- **Benefits**: Thread-safe, configurable log levels, consistent formatting, and can be optimized for production builds
- **Never use** `std::cout`, `std::cerr`, `std::clog`, or `printf` for logging - use AA*LOG*\* macros instead

## gRPC Error Handling

- **Always return `grpc::Status::OK`** in gRPC service method implementations for successful program logic execution
- gRPC Status is for **transport-level errors only** (network issues, serialization failures, etc.)
- **Program-level errors** (business logic errors, validation failures) should be handled through protobuf message fields
- Use error fields in response messages for application errors:

  ```cpp
  // Good: Program error in protobuf response
  response->set_success(false);
  response->set_error_message("Invalid input parameters");
  return grpc::Status::OK;  // Transport succeeded

  // Bad: Using gRPC status for program errors
  return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid input");
  ```

- Reserve gRPC error statuses for actual transport/infrastructure failures

## Build and Development

- Use the provided CMake configuration
- Build in separate `build/` directory
- Run tests after making changes
- Use compiler warnings as errors (already configured)
- **Cross-compilation**: Not currently supported, but planned for ARM64 Rockchip RK3588 SoC
- **Preferred Development Environment**: Use the provided devcontainer and Visual Studio Code for all development tasks. This ensures a consistent, reproducible environment and access to all required tools.
- **Commits**: Use [Conventional Commits](https://www.conventionalcommits.org/) for all commit messages to ensure clarity and consistency in version history.

## Documentation

- Use Doxygen-style comments for public APIs
- Include `@brief`, `@param`, `@return`, and `@throws` tags
- Document all public classes and methods
- Keep comments concise but informative
- Generate docs with: `cmake -B build -DBUILD_DOCS=ON && cmake --build build --target docs`
- Generate docs with: `cmake -B build -DBUILD_DOCS=ON && cmake --build build --target docs`

## Code Formatting

- Use the provided `.clang-format` configuration (Google style with 120 char limit)
- Format code automatically: `clang-format -i <file>`
- VS Code: Use Shift+Alt+F to format current file
- All code should be formatted before committing

## Dependencies

- Minimize external dependencies
- Use standard library when possible
- Document any new dependencies in README.md
- Prefer header-only libraries when external deps are needed

## Example Code Patterns

```cpp
// Header file structure (Google Style)
#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include "local_headers.h"

namespace aa {

class ExampleClass {
 public:
  // Use "pass by value and then move" for expensive-to-copy parameters
  explicit ExampleClass(std::string name, std::vector<int> data);
  explicit ExampleClass(int value);

  // Getter methods
  int GetValue() const;
  const std::string& GetName() const;

  // Setter methods
  void SetValue(int value);
  void SetName(std::string name);  // Pass by value for move semantics

 private:
  int value_;
  std::string name_;
  std::vector<int> data_;
};

}  // namespace aa

// Implementation file structure (Google Style)
#include "example_class.h"

#include <stdexcept>
#include <utility>

namespace aa {

// Pass by value and move for expensive-to-copy types
ExampleClass::ExampleClass(std::string name, std::vector<int> data)
    : name_{std::move(name)}, data_{std::move(data)} {
  if (name_.empty()) {
    throw std::invalid_argument("Name cannot be empty");
  }
}

ExampleClass::ExampleClass(int value) : value_{value} {
  if (value < 0) {
    throw std::invalid_argument("Value must be non-negative");
  }
}

int ExampleClass::GetValue() const {
  return value_;
}

void ExampleClass::SetValue(int value) {
  if (value < 0) {
    throw std::invalid_argument("Value must be non-negative");
  }
  value_ = value;
}

void ExampleClass::SetName(std::string name) {
  if (name.empty()) {
    throw std::invalid_argument("Name cannot be empty");
  }
  name_ = std::move(name);
}

}  // namespace aa
```

When generating code, ensure it follows these patterns, uses the `aa` namespace, and integrates well with the existing project structure. Prefer using the devcontainer and VS Code for all development tasks.
