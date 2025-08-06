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
  cv::Mat image = cv::imread(input_path);

  if (image.empty()) {
    AA_LOG_ERROR("Failed to load image from: " << input_path);
    return 1;
  }

  AA_LOG_INFO("Loaded image: " << input_path << " (" << image.rows << "x"
                               << image.cols << ")");

  // Create Frame from cv::Mat and set in request
  aa::shared::Frame frame(image);
  *frame_request.mutable_frame() = frame.ToProto();

  // Create 4 random polygons
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> vertex_count_dist(
      3, 6);  // 4-6 vertices (more than 3, less than 7)
  std::uniform_real_distribution<> x_coord_dist(
      0.0, static_cast<double>(image.cols * 0.4));  // X within image width/10
  std::uniform_real_distribution<> y_coord_dist(
      0.0, static_cast<double>(image.rows * 0.4));  // Y within image height/10
  std::uniform_int_distribution<> priority_dist(1, 10);  // Random priority
  std::uniform_int_distribution<> type_dist(1, 2);  // 1=INCLUSION, 2=EXCLUSION
  std::vector<int32_t> class_options = {15, 75, 57, 65, 17};

  for (int i = 0; i < 1; ++i) {
    std::vector<aa::shared::Point> vertices;
    int vertex_count = vertex_count_dist(gen) + 1;  // +1 to get 4-7 vertices

    // Generate random vertices within constrained bounds (image dims/10)
    for (int j = 0; j < vertex_count; ++j) {
      double x = x_coord_dist(gen);  // X coordinate within [0, image.cols/10]
      double y = y_coord_dist(gen);  // Y coordinate within [0, image.rows/10]
      vertices.emplace_back(x, y);
    }

    // Random polygon type
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
                << (i + 1) << ": " << vertex_count << " vertices, "
                << (type == aa::shared::PolygonType::INCLUSION ? "INCLUSION"
                                                               : "EXCLUSION")
                << ", priority=" << priority
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
