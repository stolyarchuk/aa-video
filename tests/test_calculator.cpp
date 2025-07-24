#include <gtest/gtest.h>
#include "calculator.h"
#include <stdexcept>

class CalculatorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Setup code here if needed
  }

  void TearDown() override {
    // Cleanup code here if needed
  }
};

TEST_F(CalculatorTest, AdditionWorks) {
  EXPECT_DOUBLE_EQ(aa::Calculator::add(2.0, 3.0), 5.0);
  EXPECT_DOUBLE_EQ(aa::Calculator::add(-1.0, 1.0), 0.0);
  EXPECT_DOUBLE_EQ(aa::Calculator::add(0.0, 0.0), 0.0);
}

TEST_F(CalculatorTest, SubtractionWorks) {
  EXPECT_DOUBLE_EQ(aa::Calculator::subtract(5.0, 3.0), 2.0);
  EXPECT_DOUBLE_EQ(aa::Calculator::subtract(1.0, 1.0), 0.0);
  EXPECT_DOUBLE_EQ(aa::Calculator::subtract(-1.0, -1.0), 0.0);
}

TEST_F(CalculatorTest, MultiplicationWorks) {
  EXPECT_DOUBLE_EQ(aa::Calculator::multiply(4.0, 5.0), 20.0);
  EXPECT_DOUBLE_EQ(aa::Calculator::multiply(-2.0, 3.0), -6.0);
  EXPECT_DOUBLE_EQ(aa::Calculator::multiply(0.0, 10.0), 0.0);
}

TEST_F(CalculatorTest, DivisionWorks) {
  EXPECT_DOUBLE_EQ(aa::Calculator::divide(10.0, 2.0), 5.0);
  EXPECT_DOUBLE_EQ(aa::Calculator::divide(7.0, 2.0), 3.5);
  EXPECT_DOUBLE_EQ(aa::Calculator::divide(-6.0, 2.0), -3.0);
}

TEST_F(CalculatorTest, DivisionByZeroThrows) {
  EXPECT_THROW(aa::Calculator::divide(5.0, 0.0), std::invalid_argument);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
