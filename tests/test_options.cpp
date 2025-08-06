/**
 * @file test_options.cpp
 * @brief Comprehensive unit tests for the aa::shared::Options class
 *
 * This test suite validates the Options class functionality including:
 * - Command line argument parsing with various parameter types
 * - Template method Get<T>() with different types (string, int, double, bool)
 * - Validation of argument ranges and constraints
 * - Help message handling and invalid argument detection
 * - Default value handling and optional parameters
 * - Error handling for missing or invalid arguments
 * - Edge cases and boundary value testing
 *
 * The tests simulate various command line scenarios to ensure robust
 * argument parsing and validation behavior.
 */

#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <sstream>

#include "options.h"

using namespace aa::shared;

namespace {

// Helper function to create argv from string arguments
std::vector<char*> CreateArgv(const std::vector<std::string>& args) {
  std::vector<char*> argv;
  for (const auto& arg : args) {
    argv.push_back(const_cast<char*>(arg.c_str()));
  }
  return argv;
}

// Test fixture for Options tests
class OptionsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Initialize logging for test output
  }

  void TearDown() override {
    // Clean up after each test
  }

  // Helper method to create Options with string arguments
  std::unique_ptr<Options> CreateOptions(
      const std::vector<std::string>& args,
      const std::string& instance_name = "Test Server") {
    auto modified_args = args;

    // Check if model parameter is already provided
    bool has_model = false;
    for (const auto& arg : args) {
      if (arg.find("--model") == 0 || arg.find("-m") == 0) {
        has_model = true;
        break;
      }
    }

    // Add default model parameter if not provided and instance is server
    if (!has_model && instance_name.find("Server") != std::string::npos) {
      modified_args.push_back("--model=/test/model.onnx");
    }

    auto argv = CreateArgv(modified_args);
    return std::make_unique<Options>(static_cast<int>(argv.size()), argv.data(),
                                     instance_name);
  }

  // Helper method to create Options without automatic model parameter addition
  std::unique_ptr<Options> CreateOptionsRaw(
      const std::vector<std::string>& args,
      const std::string& instance_name = "Test Server") {
    auto argv = CreateArgv(args);
    return std::make_unique<Options>(static_cast<int>(argv.size()), argv.data(),
                                     instance_name);
  }
};

// Test basic construction with minimal arguments
TEST_F(OptionsTest, BasicConstruction) {
  auto options = CreateOptions({"test_program"});

  EXPECT_TRUE(options->IsValid());

  // Test default values
  EXPECT_EQ(options->Get<std::string>("input"), "<NONE>");
  EXPECT_EQ(options->Get<int>("width"), 640);
  EXPECT_EQ(options->Get<int>("height"), 640);
  EXPECT_EQ(options->Get<double>("confidence"), 0.5);
  EXPECT_EQ(options->Get<std::string>("address"), "localhost:50051");
  EXPECT_FALSE(options->IsVerbose());
}

// Test construction with all parameters specified
TEST_F(OptionsTest, FullParameterConstruction) {
  auto options = CreateOptions(
      {"test_program", "--input=test_video.mp4", "--output=output.avi",
       "--width=1920", "--height=1080", "--confidence=0.8",
       "--model=/path/to/model.onnx", "--address=remote:8080", "--verbose"});

  EXPECT_TRUE(options->IsValid());

  EXPECT_EQ(options->Get<std::string>("input"), "test_video.mp4");
  EXPECT_EQ(options->Get<std::string>("output"), "output.avi");
  EXPECT_EQ(options->Get<int>("width"), 1920);
  EXPECT_EQ(options->Get<int>("height"), 1080);
  EXPECT_EQ(options->Get<double>("confidence"), 0.8);
  EXPECT_EQ(options->Get<std::string>("model"), "/path/to/model.onnx");
  EXPECT_EQ(options->Get<std::string>("address"), "remote:8080");
  EXPECT_TRUE(options->IsVerbose());
}

// Test parameter aliases
TEST_F(OptionsTest, ParameterAliases) {
  auto options =
      CreateOptions({"test_program", "-i=input.jpg", "-o=output.jpg", "-w=800",
                     "--ht=600",  // height alias
                     "-c=0.7", "-m=model.onnx", "-a=server:9090", "-v"});

  EXPECT_TRUE(options->IsValid());

  EXPECT_EQ(options->Get<std::string>("input"), "input.jpg");
  EXPECT_EQ(options->Get<std::string>("output"), "output.jpg");
  EXPECT_EQ(options->Get<int>("width"), 800);
  EXPECT_EQ(options->Get<int>("height"), 600);
  EXPECT_EQ(options->Get<double>("confidence"), 0.7);
  EXPECT_EQ(options->Get<std::string>("model"), "model.onnx");
  EXPECT_EQ(options->Get<std::string>("address"), "server:9090");
  EXPECT_TRUE(options->IsVerbose());
}

// Test help request handling
TEST_F(OptionsTest, HelpRequest) {
  auto options = CreateOptions({"test_program", "--help"});

  EXPECT_FALSE(options->IsValid());  // Help request should make options invalid
}

TEST_F(OptionsTest, HelpRequestWithAlias) {
  auto options = CreateOptions({"test_program", "-h"});

  EXPECT_FALSE(options->IsValid());  // Help request should make options invalid
}

TEST_F(OptionsTest, UsageRequest) {
  auto options = CreateOptions({"test_program", "--usage"});

  EXPECT_FALSE(
      options->IsValid());  // Usage request should make options invalid
}

// Test confidence threshold validation
TEST_F(OptionsTest, ValidConfidenceThreshold) {
  auto options1 = CreateOptions({"test_program", "--confidence=0.0"});
  EXPECT_TRUE(options1->IsValid());
  EXPECT_EQ(options1->Get<double>("confidence"), 0.0);

  auto options2 = CreateOptions({"test_program", "--confidence=1.0"});
  EXPECT_TRUE(options2->IsValid());
  EXPECT_EQ(options2->Get<double>("confidence"), 1.0);

  auto options3 = CreateOptions({"test_program", "--confidence=0.5"});
  EXPECT_TRUE(options3->IsValid());
  EXPECT_EQ(options3->Get<double>("confidence"), 0.5);
}

TEST_F(OptionsTest, InvalidConfidenceThresholdTooLow) {
  auto options = CreateOptions({"test_program", "--confidence=-0.1"});

  EXPECT_FALSE(options->IsValid());
}

TEST_F(OptionsTest, InvalidConfidenceThresholdTooHigh) {
  auto options = CreateOptions({"test_program", "--confidence=1.1"});

  EXPECT_FALSE(options->IsValid());
}

// Test width and height validation
TEST_F(OptionsTest, ValidDimensions) {
  auto options = CreateOptions({"test_program", "--width=1", "--height=1"});

  EXPECT_TRUE(options->IsValid());
  EXPECT_EQ(options->Get<int>("width"), 1);
  EXPECT_EQ(options->Get<int>("height"), 1);
}

TEST_F(OptionsTest, InvalidWidthZero) {
  auto options = CreateOptions({"test_program", "--width=0"});

  EXPECT_FALSE(options->IsValid());
}

TEST_F(OptionsTest, InvalidWidthNegative) {
  auto options = CreateOptions({"test_program", "--width=-100"});

  EXPECT_FALSE(options->IsValid());
}

TEST_F(OptionsTest, InvalidHeightZero) {
  auto options = CreateOptions({"test_program", "--height=0"});

  EXPECT_FALSE(options->IsValid());
}

TEST_F(OptionsTest, InvalidHeightNegative) {
  auto options = CreateOptions({"test_program", "--height=-50"});

  EXPECT_FALSE(options->IsValid());
}

TEST_F(OptionsTest, BothDimensionsInvalid) {
  auto options = CreateOptions({"test_program", "--width=0", "--height=-1"});

  EXPECT_FALSE(options->IsValid());
}

// Test template method Get<T>() with different types
TEST_F(OptionsTest, TemplateMethodStringType) {
  auto options = CreateOptions(
      {"test_program", "--input=test_string_value", "--model=path/to/model"});

  EXPECT_TRUE(options->IsValid());

  std::string input = options->Get<std::string>("input");
  std::string model = options->Get<std::string>("model");

  EXPECT_EQ(input, "test_string_value");
  EXPECT_EQ(model, "path/to/model");
}

TEST_F(OptionsTest, TemplateMethodIntType) {
  auto options =
      CreateOptions({"test_program", "--width=1920", "--height=1080"});

  EXPECT_TRUE(options->IsValid());

  int width = options->Get<int>("width");
  int height = options->Get<int>("height");

  EXPECT_EQ(width, 1920);
  EXPECT_EQ(height, 1080);
}

TEST_F(OptionsTest, TemplateMethodDoubleType) {
  auto options = CreateOptions({"test_program", "--confidence=0.85"});

  EXPECT_TRUE(options->IsValid());

  double confidence = options->Get<double>("confidence");

  EXPECT_DOUBLE_EQ(confidence, 0.85);
}

TEST_F(OptionsTest, TemplateMethodBoolType) {
  auto options1 = CreateOptions({"test_program", "--verbose"});
  EXPECT_TRUE(options1->IsValid());

  bool verbose1 = options1->Get<bool>("verbose");
  EXPECT_TRUE(verbose1);

  auto options2 = CreateOptions({"test_program"});
  EXPECT_TRUE(options2->IsValid());

  bool verbose2 = options2->Get<bool>("verbose");
  EXPECT_FALSE(verbose2);
}

// Test empty output parameter (optional)
TEST_F(OptionsTest, EmptyOutputParameter) {
  auto options = CreateOptions({"test_program"});

  EXPECT_TRUE(options->IsValid());

  std::string output = options->Get<std::string>("output");
  EXPECT_TRUE(output.empty());  // Default empty value for output
}

// Test verbose convenience method
TEST_F(OptionsTest, VerboseConvenienceMethod) {
  auto options1 = CreateOptions({"test_program", "--verbose"});
  EXPECT_TRUE(options1->IsValid());
  EXPECT_TRUE(options1->IsVerbose());

  auto options2 = CreateOptions({"test_program", "-v"});
  EXPECT_TRUE(options2->IsValid());
  EXPECT_TRUE(options2->IsVerbose());

  auto options3 = CreateOptions({"test_program"});
  EXPECT_TRUE(options3->IsValid());
  EXPECT_FALSE(options3->IsVerbose());
}

// Test invalid parameter names (should handle gracefully)
TEST_F(OptionsTest, InvalidParameterName) {
  auto options = CreateOptions({"test_program"});

  EXPECT_TRUE(options->IsValid());

  // OpenCV's parser throws for unknown parameters, so we test that it throws
  EXPECT_THROW(options->Get<std::string>("nonexistent_param"), cv::Exception);
}

// Test edge case values
TEST_F(OptionsTest, EdgeCaseValues) {
  auto options = CreateOptions({
      "test_program", "--width=1", "--height=1", "--confidence=0.0", "--input=",
      "--address=::1:8080"  // IPv6 address
  });

  EXPECT_TRUE(options->IsValid());

  EXPECT_EQ(options->Get<int>("width"), 1);
  EXPECT_EQ(options->Get<int>("height"), 1);
  EXPECT_EQ(options->Get<double>("confidence"), 0.0);
  EXPECT_EQ(options->Get<std::string>("input"), "");
  EXPECT_EQ(options->Get<std::string>("address"), "::1:8080");
}

// Test boundary confidence values
TEST_F(OptionsTest, BoundaryConfidenceValues) {
  auto options1 = CreateOptions({"test_program", "--confidence=0.0"});
  EXPECT_TRUE(options1->IsValid());
  EXPECT_EQ(options1->Get<double>("confidence"), 0.0);

  auto options2 = CreateOptions({"test_program", "--confidence=1.0"});
  EXPECT_TRUE(options2->IsValid());
  EXPECT_EQ(options2->Get<double>("confidence"), 1.0);

  // Just outside boundaries
  auto options3 = CreateOptions({"test_program", "--confidence=-0.000001"});
  EXPECT_FALSE(options3->IsValid());

  auto options4 = CreateOptions({"test_program", "--confidence=1.000001"});
  EXPECT_FALSE(options4->IsValid());
}

// Test large dimension values
TEST_F(OptionsTest, LargeDimensionValues) {
  auto options = CreateOptions({
      "test_program",
      "--width=7680",  // 8K width
      "--height=4320"  // 8K height
  });

  EXPECT_TRUE(options->IsValid());
  EXPECT_EQ(options->Get<int>("width"), 7680);
  EXPECT_EQ(options->Get<int>("height"), 4320);
}

// Test special characters in string parameters
TEST_F(OptionsTest, SpecialCharactersInStrings) {
  auto options = CreateOptions({"test_program",
                                "--input=/path/with spaces/file-name_123.mp4",
                                "--model=./models/model@v2.1.onnx",
                                "--address=user:pass@host.domain.com:8080"});

  EXPECT_TRUE(options->IsValid());

  EXPECT_EQ(options->Get<std::string>("input"),
            "/path/with spaces/file-name_123.mp4");
  EXPECT_EQ(options->Get<std::string>("model"), "./models/model@v2.1.onnx");
  EXPECT_EQ(options->Get<std::string>("address"),
            "user:pass@host.domain.com:8080");
}

// Test invalid model parameter with "true" string (OpenCV bug case)
TEST_F(OptionsTest, InvalidModelParameterTrue) {
  // Create options with model parameter set to "true" (the bug case)
  auto options = CreateOptions({"./test", "--model=true"});
  EXPECT_FALSE(options->IsValid());
}

// Test invalid model parameter with "false" string
TEST_F(OptionsTest, InvalidModelParameterFalse) {
  // Create options with model parameter set to "false"
  auto options = CreateOptions({"./test", "--model=false"});
  EXPECT_FALSE(options->IsValid());
}

// Test invalid model parameter with empty string
TEST_F(OptionsTest, InvalidModelParameterEmpty) {
  // Create options with model parameter set to empty string
  auto options = CreateOptions({"./test", "--model="});
  EXPECT_FALSE(options->IsValid());
}

// Test valid model parameter is required when not provided
TEST_F(OptionsTest, ModelParameterRequired) {
  // Create options without model parameter (should fail with <none> default)
  auto options = CreateOptionsRaw({"./test", "--verbose"});
  EXPECT_FALSE(options->IsValid());
}

// Test context-aware validation: Server requires model, Client does not
TEST_F(OptionsTest, ServerRequiresModel) {
  // Server without model should fail
  auto server_options =
      CreateOptionsRaw({"./test", "--verbose"}, "Detector Server");
  EXPECT_FALSE(server_options->IsValid());

  // Server with model should pass
  auto server_with_model = CreateOptionsRaw(
      {"./test", "--model=/test/model.onnx"}, "Detector Server");
  EXPECT_TRUE(server_with_model->IsValid());
}

TEST_F(OptionsTest, ClientRequiresInput) {
  // Client without input should fail
  auto client_options =
      CreateOptionsRaw({"./test", "--verbose"}, "Detector Client");
  EXPECT_FALSE(client_options->IsValid());

  // Client with input should pass
  auto client_with_input = CreateOptionsRaw(
      {"./test", "--input=/test/input.jpg"}, "Detector Client");
  EXPECT_TRUE(client_with_input->IsValid());
}

TEST_F(OptionsTest, ClientModelOptional) {
  // Client without model but with input should pass (model not required for
  // client)
  auto client_options = CreateOptionsRaw({"./test", "--input=/test/input.jpg"},
                                         "Detector Client");
  EXPECT_TRUE(client_options->IsValid());

  // Client with both input and model should also pass
  auto client_with_both = CreateOptionsRaw(
      {"./test", "--input=/test/input.jpg", "--model=/test/model.onnx"},
      "Detector Client");
  EXPECT_TRUE(client_with_both->IsValid());
}

TEST_F(OptionsTest, ServerInputOptional) {
  // Server without input but with model should pass (input not required for
  // server)
  auto server_options = CreateOptionsRaw({"./test", "--model=/test/model.onnx"},
                                         "Detector Server");
  EXPECT_TRUE(server_options->IsValid());

  // Server with both model and input should also pass
  auto server_with_both = CreateOptionsRaw(
      {"./test", "--model=/test/model.onnx", "--input=/test/input.jpg"},
      "Detector Server");
  EXPECT_TRUE(server_with_both->IsValid());
}

}  // namespace
