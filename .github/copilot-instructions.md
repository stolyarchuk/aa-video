<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# C++ CMake Project Instructions

This is a modern C++ project using CMake as the build system. Please follow these guidelines when working with this codebase:

## Code Style and Standards

- **C++ Standard**: Use C++23 features and modern C++ best practices
- **Style Guide**: Follow Google C++ Style Guide with 120 character line limit
- **Formatting**: Use `.clang-format` configuration for automatic code formatting
- **Naming Conventions** (Google Style):
  - Classes/Types: PascalCase (e.g., `Calculator`, `MyClass`)
  - Functions/Methods: PascalCase (e.g., `AddNumbers`, `GetValue`)
  - Variables: snake_case (e.g., `result_value`, `user_count`)
  - Constants: kConstantName (e.g., `kMaxValue`, `kDefaultTimeout`)
  - Member variables: snake_case with trailing underscore (e.g., `value_`, `count_`)
  - Namespaces: snake_case (e.g., `aa`)
  - Macros: UPPER_SNAKE_CASE (e.g., `MY_MACRO`)
- **Project Namespace**: Use `aa` as the main project namespace for all code.

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
- Generate docs with: `cmake -B build -DBUILD_DOCUMENTATION=ON && cmake --build build --target docs`

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
  explicit ExampleClass(int value);

  // Getter methods
  int GetValue() const;
  const std::string& GetName() const;

  // Setter methods
  void SetValue(int value);
  void SetName(const std::string& name);

 private:
  int value_;
  std::string name_;
};

}  // namespace aa

// Implementation file structure (Google Style)
#include "example_class.h"

#include <stdexcept>

namespace aa {

ExampleClass::ExampleClass(int value) : value_(value) {
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

}  // namespace aa
```

When generating code, ensure it follows these patterns, uses the `aa` namespace, and integrates well with the existing project structure. Prefer using the devcontainer and VS Code for all development tasks.
