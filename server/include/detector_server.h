#pragma once

#include <memory>

#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>

#include "detector_service.h"
#include "options.h"

namespace aa::server {

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
   * @brief Load the ResNet model for object detection
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
   * @return std::vector<cv::Rect> Detected bounding boxes
   */
  std::vector<cv::Rect> PostprocessDetections(const cv::Mat& network_output,
                                              const cv::Mat& original_frame);

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
