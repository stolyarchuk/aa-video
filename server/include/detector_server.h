#pragma once

#include <memory>
#include <vector>

#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>

#include "detector_service.h"
#include "options.h"

// Forward declarations
namespace aa::shared {
class Polygon;
}

namespace aa::server {

/**
 * @brief Structure to store detection results
 */
struct Detection {
  cv::Rect bbox;     ///< Bounding box of the detection
  int class_id;      ///< Class identifier of the detected object
  float confidence;  ///< Confidence score of the detection
};

/**
 * @brief High-level server wrapper for detector service using CRTP
 *
 * Provides a convenient interface to manage the lifecycle of the detector
 * service server, including initialization, startup, and shutdown operations.
 * Uses composition with DetectorServiceImpl instead of inheritance.
 */
class DetectorServer {
 public:
  /**
   * @brief Construct a new Detector Server object from options
   *
   * @param options Configuration options containing server settings
   */
  explicit DetectorServer(aa::shared::Options options);

  /**
   * @brief Destroy the Detector Server object
   */
  ~DetectorServer() = default;

  // Disable copy construction and assignment
  DetectorServer(const DetectorServer&) = delete;
  DetectorServer& operator=(const DetectorServer&) = delete;

  // Enable move construction and assignment
  DetectorServer(DetectorServer&&) noexcept = default;
  DetectorServer& operator=(DetectorServer&&) noexcept = default;

  /**
   * @brief Initialize the server components
   */
  void Initialize();

  /**
   * @brief Start the server and begin listening for requests
   */
  void Start();

  /**
   * @brief Shutdown the server gracefully
   */
  void Shutdown();

 private:
  std::unique_ptr<DetectorServiceImpl> service_;
  cv::dnn::Net dnn_network_;
  aa::shared::Options options_;

  /**
   * @brief Initialize the neural network for inference
   */
  void InitializeNetwork();

  /**
   * @brief Load the neural network model for object detection
   *
   * @return true if model loaded successfully
   * @return false if failed to load model
   */
  bool LoadModel();

  /**
   * @brief Preprocess input frame for neural network inference
   *
   * @param frame Input OpenCV Mat frame
   * @return cv::Mat Preprocessed blob ready for inference
   */
  cv::Mat PreprocessFrame(const cv::Mat& frame);

  /**
   * @brief Run inference on the preprocessed frame
   *
   * @param blob Preprocessed input blob
   * @return cv::Mat Network output with detections
   */
  cv::Mat RunInference(const cv::Mat& blob);

  /**
   * @brief Post-process network outputs to extract detections
   *
   * @param network_output Raw network output
   * @param original_frame Original input frame for reference
   * @param polygons Detection zones with inclusion/exclusion rules and
   * priorities
   * @return std::vector<Detection> Detected objects with bounding boxes, class
   * IDs, and confidence scores
   */
  std::vector<Detection> PostprocessDetections(
      const cv::Mat& network_output, const cv::Mat& original_frame,
      const std::vector<aa::shared::Polygon>& polygons);

  /**
   * @brief Parse network output to extract raw detections
   *
   * @param network_output Raw network output tensor
   * @return std::vector<Detection> Raw detections from network
   */
  std::vector<Detection> ParseNetworkOutput(const cv::Mat& network_output);

  /**
   * @brief Apply Non-Maximum Suppression to remove duplicate detections
   *
   * @param detections Input detections to filter
   * @return std::vector<Detection> Filtered detections after NMS
   */
  std::vector<Detection> ApplyNonMaximumSuppression(
      const std::vector<Detection>& detections);

  /**
   * @brief Scale detections from model coordinates to original frame
   * coordinates
   *
   * @param detections Detections in model coordinate space
   * @param original_frame Original frame for scaling reference
   * @return std::vector<Detection> Scaled detections
   */
  std::vector<Detection> ScaleDetectionsToOriginalFrame(
      const std::vector<Detection>& detections,
      [[maybe_unused]] const cv::Mat& original_frame);

  /**
   * @brief Filter detections based on polygon rules
   *
   * @param detections Input detections to filter
   * @param polygons Polygon zones with inclusion/exclusion rules
   * @return std::vector<Detection> Filtered detections
   */
  std::vector<Detection> FilterDetectionsByPolygons(
      const std::vector<Detection>& detections,
      const std::vector<aa::shared::Polygon>& polygons);

  /**
   * @brief Calculate the center point of a detection's bounding box
   *
   * @param detection Detection with bounding box
   * @return std::pair<double, double> Center point (x, y)
   */
  std::pair<double, double> GetDetectionCenter(const Detection& detection);

  /**
   * @brief Find polygons containing a given point
   *
   * @param center_x X coordinate of the point
   * @param center_y Y coordinate of the point
   * @param polygons Available polygons to check
   * @return std::vector<const aa::shared::Polygon*> Containing polygons
   */
  std::vector<const aa::shared::Polygon*> FindContainingPolygons(
      double center_x, double center_y,
      const std::vector<aa::shared::Polygon>& polygons);

  /**
   * @brief Check if detection should be included based on polygon rules
   *
   * @param detection Detection to evaluate
   * @param containing_polygons Polygons containing the detection (sorted by
   * priority)
   * @return bool True if detection should be included
   */
  bool ShouldIncludeDetection(
      const Detection& detection,
      const std::vector<const aa::shared::Polygon*>& containing_polygons);

  /**
   * @brief Check if detection class matches polygon target classes
   *
   * @param detection Detection to check
   * @param polygon Polygon with target classes
   * @return bool True if detection class is in target classes or target classes
   * is empty
   */
  bool IsDetectionClassAllowed(const Detection& detection,
                               const aa::shared::Polygon& polygon);

  /**
   * @brief Log polygon configuration for debugging
   *
   * @param polygons Polygons to log
   */
  void LogPolygonConfiguration(
      const std::vector<aa::shared::Polygon>& polygons);

  /**
   * @brief Log network output dimensions for debugging
   *
   * @param network_output Network output to log
   */
  void LogNetworkOutput(const cv::Mat& network_output);

  /**
   * @brief Check the health of the server
   *
   * @param request Health check request
   * @param response Health check response
   * @return grpc::Status indicating success or failure
   */
  grpc::Status CheckHealth(const aa::proto::CheckHealthRequest*,
                           aa::proto::CheckHealthResponse*) const;
  /**
   * @brief Process a frame for detection
   *
   * @param request Frame processing request
   * @param response Frame processing response
   * @return grpc::Status indicating success or failure
   */
  grpc::Status ProcessFrame(const aa::proto::ProcessFrameRequest*,
                            aa::proto::ProcessFrameResponse*) const;
};

}  // namespace aa::server
