#include "config.h"

namespace {
const std::string keys =
    "{help h usage ? |      | Print this help message}"
    "{input i        |      | Input file path, camera index, or video stream "
    "(required)}"
    "{output o       |      | Output file path (optional)}"
    "{width w        | 640  | Frame width for processing}"
    "{height ht      | 480  | Frame height for processing}"
    "{confidence c   | 0.5  | Confidence threshold for detection (0.0-1.0)}"
    "{model m        |      | Path to detection model file (required)}"
    "{config cfg     |      | Path to configuration file (optional)}"
    "{verbose v      | false| Enable verbose output}"
    "{gpu g          | false| Use GPU acceleration if available}";

}

namespace aa::shared {

Options::Options(int argc, const char* const argv[])
    : parser_{argc, argv, keys}, is_valid_{false} {
  InitializeParser(argc, argv);
  is_valid_ = ValidateArguments();
}

bool Options::IsValid() const { return is_valid_; }

void Options::PrintHelp() const { parser_.printMessage(); }

std::string Options::GetInput() const {
  return parser_.get<std::string>("input");
}

std::string Options::GetOutput() const {
  return parser_.get<std::string>("output");
}

int Options::GetWidth() const { return parser_.get<int>("width"); }

int Options::GetHeight() const { return parser_.get<int>("height"); }

double Options::GetConfidenceThreshold() const {
  return parser_.get<double>("confidence");
}

std::string Options::GetModelPath() const {
  return parser_.get<std::string>("model");
}

std::string Options::GetConfigPath() const {
  return parser_.get<std::string>("config");
}

bool Options::IsVerbose() const { return parser_.get<bool>("verbose"); }

bool Options::UseGpu() const { return parser_.get<bool>("gpu"); }

void Options::InitializeParser(int argc, const char* const argv[]) {
  // Define command line parameters using OpenCV's format
  // Format: "{parameter_name|alias|default_value|description}"
  const std::string keys =
      "{help h usage ? |      | Print this help message}"
      "{input i        |      | Input file path, camera index, or video stream "
      "(required)}"
      "{output o       |      | Output file path (optional)}"
      "{width w        | 640  | Frame width for processing}"
      "{height ht      | 480  | Frame height for processing}"
      "{confidence c   | 0.5  | Confidence threshold for detection (0.0-1.0)}"
      "{model m        |      | Path to detection model file (required)}"
      "{config cfg     |      | Path to configuration file (optional)}"
      "{verbose v      | false| Enable verbose output}"
      "{gpu g          | false| Use GPU acceleration if available}";

  // Initialize parser with the keys
  parser_ = cv::CommandLineParser(argc, argv, keys);
}

bool Options::ValidateArguments() {
  // Check for help request
  if (parser_.has("help")) {
    return false;  // Will trigger help display
  }

  // Check if parser encountered any errors
  if (!parser_.check()) {
    parser_.printErrors();
    return false;
  }

  // Check required parameters
  if (!parser_.has("input") || parser_.get<std::string>("input").empty()) {
    std::cerr << "Error: Input parameter is required" << std::endl;
    return false;
  }

  if (!parser_.has("model") || parser_.get<std::string>("model").empty()) {
    std::cerr << "Error: Model parameter is required" << std::endl;
    return false;
  }

  // Validate ranges
  double confidence = parser_.get<double>("confidence");
  if (confidence < 0.0 || confidence > 1.0) {
    std::cerr << "Error: Confidence threshold must be between 0.0 and 1.0"
              << std::endl;
    return false;
  }

  int width = parser_.get<int>("width");
  int height = parser_.get<int>("height");
  if (width <= 0 || height <= 0) {
    std::cerr << "Error: Width and height must be positive values" << std::endl;
    return false;
  }

  return true;
}

}  // namespace aa::shared
