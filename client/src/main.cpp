#include <chrono>
#include <random>
#include <set>
#include <thread>

#include <opencv2/opencv.hpp>

#include "detector_client.h"
#include "frame.h"
#include "logging.h"
#include "options.h"
#include "point.h"
#include "polygon.h"

using namespace aa::client;
using namespace aa::shared;

int main(int argc, char* argv[]) {
  // Parse command line arguments
  Options options(argc, argv, "Detector Client");

  // Check if arguments are valid
  if (!options.IsValid()) {
    options.PrintHelp();
    return 1;
  }

  Logging::Initialize(options.IsVerbose());

  // Create client
  DetectorClient client(options);

  aa::proto::CheckHealthRequest health_request;
  aa::proto::CheckHealthResponse health_response;

  grpc::Status status = client.CheckHealth(health_request, &health_response);

  if (!status.ok()) {
    AA_LOG_ERROR("Health check failed: " << status.error_message());
    return 1;
  } else {
    AA_LOG_INFO("Health check passed");
  }

  aa::proto::ProcessFrameRequest frame_request;
  aa::proto::ProcessFrameResponse frame_response;

  // Load image using OpenCV
  std::string input_path = options.Get<std::string>("input");
  cv::Mat input_image = cv::imread(input_path);

  if (input_image.empty()) {
    AA_LOG_ERROR("Failed to load image from: " << input_path);
    return 1;
  }

  AA_LOG_INFO("Loaded image: " << input_path << " (" << input_image.rows << "x"
                               << input_image.cols << ")");

  // Create Frame from cv::Mat and set in request
  aa::shared::Frame frame(input_image);
  *frame_request.mutable_frame() = frame.ToProto();

  // Create bounding box-like square polygons
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> priority_dist(0, 10);  // Random priority 0-10
  std::uniform_int_distribution<> type_dist(
      1, 2);  // 1=INCLUSION, 2=EXCLUSION (not 0)

  // Calculate dimension constraints based on input_image size
  double min_width = input_image.cols * 0.2;   // Minimum 20% of image width
  double max_width = input_image.cols * 0.8;   // Maximum 80% of image width
  double min_height = input_image.rows * 0.2;  // Minimum 20% of image height
  double max_height = input_image.rows * 0.8;  // Maximum 80% of image height

  std::uniform_real_distribution<> width_dist(min_width, max_width);
  std::uniform_real_distribution<> height_dist(min_height, max_height);

  // Use all COCO classes (0-79)
  std::vector<int32_t> class_options;
  for (int32_t i = 0; i < 80; ++i) {
    class_options.push_back(i);
  }

  for (int i = 0; i < 6; ++i) {
    std::vector<aa::shared::Point> vertices;

    // Generate random dimensions within constraints
    double width = width_dist(gen);
    double height = height_dist(gen);

    // Calculate random position ensuring the polygon fits within image bounds
    std::uniform_real_distribution<> x_dist(0.0, input_image.cols - width);
    std::uniform_real_distribution<> y_dist(0.0, input_image.rows - height);

    double x_offset = x_dist(gen);
    double y_offset = y_dist(gen);

    // Create bounding box-like square (4 vertices)
    vertices.emplace_back(x_offset, y_offset);                   // Top-left
    vertices.emplace_back(x_offset + width, y_offset);           // Top-right
    vertices.emplace_back(x_offset + width, y_offset + height);  // Bottom-right
    vertices.emplace_back(x_offset, y_offset + height);          // Bottom-left

    // Random polygon type (not 0)
    aa::shared::PolygonType type = (type_dist(gen) == 1)
                                       ? aa::shared::PolygonType::INCLUSION
                                       : aa::shared::PolygonType::EXCLUSION;

    // Random priority
    int32_t priority = priority_dist(gen);

    // Use all target classes
    std::vector<int32_t> target_classes = class_options;

    // Create polygon and add to request
    aa::shared::Polygon polygon(std::move(vertices), type, priority,
                                std::move(target_classes));
    *frame_request.add_polygons() = polygon.ToProto();

    AA_LOG_INFO("Added polygon "
                << (i + 1) << ": bounding box (" << static_cast<int>(width)
                << "x" << static_cast<int>(height) << "), "
                << (type == aa::shared::PolygonType::INCLUSION ? "INCLUSION"
                                                               : "EXCLUSION")
                << ", priority=" << priority << ", position=("
                << static_cast<int>(x_offset) << ","
                << static_cast<int>(y_offset) << ")"
                << ", classes=" << class_options.size());
  }

  status = client.ProcessFrame(frame_request, &frame_response);

  if (!status.ok()) {
    AA_LOG_ERROR("Process frame failed: " << status.error_message());
    return 1;
  }

  auto result_image =
      aa::shared::Frame::FromProto(frame_response.result()).ToMat();
  auto output_path = options.Get<std::string>("output");

  cv::imwrite(output_path, result_image);

  return 0;
}
