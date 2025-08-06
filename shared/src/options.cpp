#include "options.h"

#include "logging.h"

namespace {
// Define command line parameters using OpenCV's format
// Format: "{parameter_name|alias|default_value|description}"

const cv::String keys =
    "{help h usage ? |      | Print this help message}"
    "{input i        |<NONE>| Input file path (optional)}"
    "{output o       |output.png| Output file path (optional)}"
    "{width w        | 640  | Frame width for processing}"
    "{height ht      | 640  | Frame height for processing}"
    "{confidence c   | 0.5  | Confidence threshold for detection (0.0-1.0)}"
    "{model m        |<NONE>| Path to detection model file (REQUIRED)}"
    "{cfg            |<NONE>| Path to YOLOv7 configuration file (optional)}"
    "{address a      | localhost:50051 | Server address for gRPC "
    "communication}"
    "{verbose v      | false| Enable verbose output}";
}  // namespace

namespace aa::shared {

Options::Options(int argc, const char* const argv[], std::string_view name)
    : parser_{argc, argv, keys}, is_valid_{false}, instance_name_{name} {
  InitializeParser(argc, argv, name);
  is_valid_ = ValidateArguments();
}

bool Options::IsValid() const { return is_valid_; }

void Options::PrintHelp() const { parser_.printMessage(); }

bool Options::IsVerbose() const { return parser_.get<bool>("verbose"); }

void Options::InitializeParser(int argc, const char* const argv[],
                               std::string_view name) {
  // Initialize parser with the keys
  parser_ = cv::CommandLineParser(argc, argv, keys);
  parser_.about(name.data());
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

  // Context-aware validation based on instance type
  bool is_server = instance_name_.find("Server") != std::string::npos;
  bool is_client = instance_name_.find("Client") != std::string::npos;

  // Validate model parameter - REQUIRED for server, optional for client
  if (is_server) {
    try {
      cv::String model_path = parser_.get<cv::String>("model");
      if (model_path.empty() || model_path == "true" || model_path == "false" ||
          model_path == "<NONE>") {
        AA_LOG_ERROR(
            "Model parameter is required for DetectorServer. "
            "Use: --model=path/to/model.onnx or -m path/to/model.onnx");
        return false;
      }
      // TODO: Add file existence check when needed
      // if (!std::filesystem::exists(model_path)) {
      //   AA_LOG_ERROR("Model file does not exist: " << model_path);
      //   return false;
      // }
    } catch (const std::exception& e) {
      AA_LOG_ERROR("Error parsing model parameter: " << e.what());
      return false;
    }
  }

  // Validate input parameter - REQUIRED for client, optional for server
  if (is_client) {
    try {
      cv::String input_path = parser_.get<cv::String>("input");
      if (input_path.empty() || input_path == "true" || input_path == "false" ||
          input_path == "<NONE>") {
        AA_LOG_ERROR(
            "Input parameter is required for DetectorClient. "
            "Use: --input=path/to/input.jpg or -i path/to/input.jpg");
        return false;
      }
      // TODO: Add file existence check when needed
      // if (!std::filesystem::exists(input_path)) {
      //   AA_LOG_ERROR("Input file does not exist: " << input_path);
      //   return false;
      // }
    } catch (const std::exception& e) {
      AA_LOG_ERROR("Error parsing input parameter: " << e.what());
      return false;
    }
  }

  // Validate ranges (common for both client and server)
  double confidence = parser_.get<double>("confidence");
  if (confidence < 0.0 || confidence > 1.0) {
    AA_LOG_ERROR("Confidence threshold must be between 0.0 and 1.0");
    return false;
  }

  int width = parser_.get<int>("width");
  int height = parser_.get<int>("height");
  if (width <= 0 || height <= 0) {
    AA_LOG_ERROR("Width and height must be positive values");
    return false;
  }

  return true;
}

}  // namespace aa::shared
