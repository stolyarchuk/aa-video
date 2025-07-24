#include "calculator.h"
#include <stdexcept>

namespace aa {
double Calculator::add(double a, double b) { return a + b; }

double Calculator::subtract(double a, double b) { return a - b; }

double Calculator::multiply(double a, double b) { return a * b; }

double Calculator::divide(double a, double b) {
  if (b == 0.0) {
    throw std::invalid_argument("Division by zero is not allowed");
  }
  return a / b;
}
}  // namespace aa
