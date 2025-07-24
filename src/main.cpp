#include <iostream>
#include "calculator.h"

int main() {
  std::cout << "Hello, World! This is a C++ CMake project.\n";

  // Demonstrate calculator functionality
  std::cout << "\nCalculator Demo:\n";
  std::cout << "2 + 3 = " << aa::Calculator::add(2.0, 3.0) << "\n";
  std::cout << "5 - 2 = " << aa::Calculator::subtract(5.0, 2.0) << "\n";
  std::cout << "4 * 6 = " << aa::Calculator::multiply(4.0, 6.0) << "\n";
  std::cout << "10 / 2 = " << aa::Calculator::divide(10.0, 2.0) << "\n";

  return 0;
}
