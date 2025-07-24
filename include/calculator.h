#pragma once

namespace aa {
/**
 * @brief A simple calculator class for demonstration
 */
class Calculator {
 public:
  /**
   * @brief Add two numbers
   * @param a First number
   * @param b Second number
   * @return Sum of a and b
   */
  static double add(double a, double b);

  /**
   * @brief Subtract two numbers
   * @param a First number
   * @param b Second number
   * @return Difference of a and b
   */
  static double subtract(double a, double b);

  /**
   * @brief Multiply two numbers
   * @param a First number
   * @param b Second number
   * @return Product of a and b
   */
  static double multiply(double a, double b);

  /**
   * @brief Divide two numbers
   * @param a First number
   * @param b Second number
   * @return Quotient of a and b
   * @throws std::invalid_argument if b is zero
   */
  static double divide(double a, double b);
};
}  // namespace aa
