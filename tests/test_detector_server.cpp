#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <stdexcept>
#include <fstream>
#include <opencv2/opencv.hpp>

#include "detector_server.h"
#include "options.h"

namespace aa::server {

class DetectorServerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create test arguments for Options with valid parameters
    const char* test_argv[] = {
        "test_program", "--address=localhost:50051",
        "--model=/nonexistent/path/model.onnx",  // Non-existent path for
                                                 // testing
        "--input=test_input.png", "--confidence=0.5"};
    int test_argc = sizeof(test_argv) / sizeof(test_argv[0]);

    test_options_ = std::make_unique<aa::shared::Options>(
        test_argc, test_argv, "Test Detector Server");

    // Create valid options for successful tests
    const char* valid_argv[] = {"test_program", "--address=localhost:50052",
                                "--input=input.png",
                                "--model=/test/model.onnx"};
    int valid_argc = sizeof(valid_argv) / sizeof(valid_argv[0]);

    valid_options_ = std::make_unique<aa::shared::Options>(
        valid_argc, valid_argv, "Test Detector Server");
  }

  void TearDown() override {
    test_options_.reset();
    valid_options_.reset();
  }

  std::unique_ptr<aa::shared::Options> test_options_;
  std::unique_ptr<aa::shared::Options> valid_options_;
};

// Test: Constructor with valid model creates server successfully
TEST_F(DetectorServerTest, ConstructorWithValidModel) {
  ASSERT_TRUE(valid_options_->IsValid());

  // Constructor should work with existing model file
  EXPECT_NO_THROW({ DetectorServer server(std::move(*valid_options_)); });
}

// Test: Constructor with invalid model throws exception
TEST_F(DetectorServerTest, ConstructorWithInvalidModel) {
  const char* invalid_argv[] = {"test_program", "--address=localhost:50052",
                                "--model=/nonexistent/model.onnx"};
  int invalid_argc = sizeof(invalid_argv) / sizeof(invalid_argv[0]);

  aa::shared::Options invalid_options(invalid_argc, invalid_argv,
                                      "Test Server");
  ASSERT_TRUE(invalid_options.IsValid());

  // Constructor should throw with non-existent model
  EXPECT_THROW({ DetectorServer server(std::move(invalid_options)); },
               cv::Exception);
}

// Test: Server lifecycle methods
TEST_F(DetectorServerTest, ServerLifecycle) {
  DetectorServer server(std::move(*valid_options_));

  // Initialize should register service methods
  EXPECT_NO_THROW({ server.Initialize(); });

  // Shutdown should be safe to call
  EXPECT_NO_THROW({ server.Shutdown(); });
}

// Test: Multiple shutdown calls are safe
TEST_F(DetectorServerTest, MultipleShutdownCalls) {
  DetectorServer server(std::move(*valid_options_));

  // Multiple shutdown calls should be safe (idempotent)
  EXPECT_NO_THROW({
    server.Shutdown();
    server.Shutdown();
    server.Shutdown();
  });
}

}  // namespace aa::server
