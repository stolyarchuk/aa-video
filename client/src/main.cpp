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
      0.0, static_cast<double>(image.cols / 10));  // X within image width/10
  std::uniform_real_distribution<> y_coord_dist(
      0.0, static_cast<double>(image.rows / 10));  // Y within image height/10
  std::uniform_int_distribution<> priority_dist(1, 10);  // Random priority
  std::uniform_int_distribution<> type_dist(1, 2);  // 1=INCLUSION, 2=EXCLUSION
  std::vector<int32_t> class_options = {17, 75, 76, 78, 79};
  std::uniform_int_distribution<> class_idx_dist(
      0, static_cast<int>(class_options.size() - 1));
  std::uniform_int_distribution<> num_classes_dist(
      1, 3);  // 1-3 target classes per polygon

  for (int i = 0; i < 4; ++i) {
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

    // Random target classes
    std::vector<int32_t> target_classes;
    int num_classes = num_classes_dist(gen);
    std::set<int> selected_indices;

    for (int k = 0; k < num_classes; ++k) {
      int idx;
      do {
        idx = class_idx_dist(gen);
      } while (selected_indices.count(idx));
      selected_indices.insert(idx);
      target_classes.push_back(class_options[idx]);
    }

    // Create polygon and add to request
    aa::shared::Polygon polygon(std::move(vertices), type, priority,
                                std::move(target_classes));
    *frame_request.add_polygons() = polygon.ToProto();

    AA_LOG_INFO("Added polygon "
                << (i + 1) << ": " << vertex_count << " vertices, "
                << (type == aa::shared::PolygonType::INCLUSION ? "INCLUSION"
                                                               : "EXCLUSION")
                << ", priority=" << priority
                << ", classes=" << target_classes.size());
  }

  status = client.ProcessFrame(frame_request, &frame_response);

  if (!status.ok()) {
    AA_LOG_ERROR("Process frame failed: " << status.error_message());
    return 1;
  }

  return 0;
}
