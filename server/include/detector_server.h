#pragma once

#include <memory>
#include <vector>

#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>

#include "detector_service.h"
#include "options.h"
#include "polygon_filter.h"
#include "types.h"
#include "yolo.h"

// Forward declarations
namespace aa::shared {
class Polygon;
}

/**
 * @brief Server-side components for the AA Video Processing System
 *
 * Contains gRPC server implementations, YOLO inference engines, polygon
 * filtering systems, and server-specific functionality for real-time
 * object detection processing. Provides high-performance neural network
 * inference with advanced detection zone management.
 */
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
  aa::shared::Options options_;
  std::unique_ptr<DetectorServiceImpl> service_;
  Yolo yolo_;
  PolygonFilter polygon_filter_;

  /**
   * @brief Check the health of the server
   *
   * Provides a health check endpoint to verify server status and availability.
   * Always returns success for a running server instance.
   *
   * @param request Health check request (pointer, typically empty)
   * @param response Health check response (pointer to populate)
   * @return grpc::Status indicating success or failure
   */
  grpc::Status CheckHealth(const aa::proto::CheckHealthRequest* request,
                           aa::proto::CheckHealthResponse* response) const;

  /**
   * @brief Process a frame for detection
   *
   * Performs object detection on the provided frame using the loaded YOLO model
   * and applies polygon-based filtering to the detection results.
   *
   * @param request Frame processing request (pointer containing frame and
   * polygons)
   * @param response Frame processing response (pointer to populate with
   * results)
   * @return grpc::Status indicating success or failure
   */
  grpc::Status ProcessFrame(const aa::proto::ProcessFrameRequest* request,
                            aa::proto::ProcessFrameResponse* response) const;
};

}  // namespace aa::server
