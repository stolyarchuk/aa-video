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

  // Use all COCO classes (0-79)
  std::vector<int32_t> class_options;
  for (int32_t i = 0; i < 80; ++i) {
    class_options.push_back(i);
  }

  // First polygon: starts from left boundary (col = 0)
  {
    double polygon_width = input_image.cols * 0.89;  // 78% of image width
    double polygon_height = input_image.rows;        // Full image height

    std::vector<aa::shared::Point> vertices;
    double x_offset = 0.0;  // Left boundary
    double y_offset = 0.0;  // Top boundary

    // Create bounding box (4 vertices)
    vertices.emplace_back(x_offset, y_offset);                  // Top-left
    vertices.emplace_back(x_offset + polygon_width, y_offset);  // Top-right
    vertices.emplace_back(x_offset + polygon_width,
                          y_offset + polygon_height);            // Bottom-right
    vertices.emplace_back(x_offset, y_offset + polygon_height);  // Bottom-left

    aa::shared::PolygonType type = aa::shared::PolygonType::INCLUSION;
    int32_t priority = 3;
    std::vector<int32_t> target_classes = class_options;

    aa::shared::Polygon polygon(std::move(vertices), type, priority,
                                std::move(target_classes));
    *frame_request.add_polygons() = polygon.ToProto();

    AA_LOG_INFO("Added polygon 1: left boundary ("
                << static_cast<int>(polygon_width) << "x"
                << static_cast<int>(polygon_height) << "), INCLUSION"
                << ", priority=" << priority << ", position=(0,0)"
                << ", classes=" << class_options.size());
  }

  // Second polygon: ends at right boundary (col = max_col)
  {
    double polygon_width = input_image.cols * 0.55;  // 55% of image width
    double polygon_height = input_image.rows;        // Full image height

    std::vector<aa::shared::Point> vertices;
    double x_offset =
        input_image.cols - polygon_width;  // Right boundary minus width
    double y_offset = 0.0;                 // Top boundary

    // Create bounding box (4 vertices)
    vertices.emplace_back(x_offset, y_offset);                  // Top-left
    vertices.emplace_back(x_offset + polygon_width, y_offset);  // Top-right
    vertices.emplace_back(x_offset + polygon_width,
                          y_offset + polygon_height);            // Bottom-right
    vertices.emplace_back(x_offset, y_offset + polygon_height);  // Bottom-left

    aa::shared::PolygonType type = aa::shared::PolygonType::EXCLUSION;
    int32_t priority = 5;
    std::vector<int32_t> target_classes = class_options;

    aa::shared::Polygon polygon(std::move(vertices), type, priority,
                                std::move(target_classes));
    *frame_request.add_polygons() = polygon.ToProto();

    AA_LOG_INFO("Added polygon 2: right boundary ("
                << static_cast<int>(polygon_width) << "x"
                << static_cast<int>(polygon_height) << "), INCLUSION"
                << ", priority=" << priority << ", position=("
                << static_cast<int>(x_offset) << ",0)"
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

  if (!result_image.empty()) {
    cv::imwrite(output_path, result_image);

    const std::string kWinName = "Yolo Object Detector";
    cv::namedWindow(kWinName, cv::WINDOW_NORMAL);
    cv::imshow(kWinName, result_image);

    if (int k = cv::waitKey(30000); k == 27) {  // ESC key to exit
      AA_LOG_INFO("Exiting on user request");
    }
    AA_LOG_INFO("Processed frame saved to: " << output_path);
  }
  return 0;
}
