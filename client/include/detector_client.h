#pragma once
#include <grpcpp/grpcpp.h>

#include "detector_service.grpc.pb.h"
#include "options.h"
#include "rpc_client.h"

namespace aa::client {

/**
 * @brief High-level client wrapper for detector gRPC service
 *
 * Provides a convenient interface for communicating with the video processing
 * detector server. Implements the DetectorService gRPC interface with proper
 * error handling, timeout management, and type-safe method calls.
 *
 * @grpc Implements aa::proto::DetectorService client interface
 * @threadsafe Thread-safe for concurrent requests through gRPC framework
 *
 * Features:
 * - Health check monitoring of server status
 * - Frame processing requests with polygon filtering
 * - Automatic timeout and error handling
 * - Configuration through Options class
 *
 * Usage:
 * @code
 * aa::shared::Options options(argc, argv, "Detector Client");
 * if (!options.IsValid()) {
 *     options.PrintHelp();
 *     return -1;
 * }
 *
 * aa::client::DetectorClient client(std::move(options));
 *
 * // Check server health
 * aa::proto::CheckHealthRequest health_req;
 * aa::proto::CheckHealthResponse health_res;
 * auto status = client.CheckHealth(health_req, &health_res);
 *
 * // Process frame
 * aa::proto::ProcessFrameRequest frame_req;
 * aa::proto::ProcessFrameResponse frame_res;
 * status = client.ProcessFrame(frame_req, &frame_res);
 * @endcode
 */
class DetectorClient final : public RpcClient<aa::proto::DetectorService> {
 public:
  /**
   * @brief Construct a new Detector Client object from options
   *
   * @param options Configuration options containing server address
   */
  explicit DetectorClient(aa::shared::Options options)
      : RpcClient{options.Get<std::string>("address")},
        options_{std::move(options)} {}

  /**
   * @brief Check server health status
   *
   * Sends a health check request to the detector server to verify connectivity
   * and service availability.
   *
   * @param request Health check request (typically empty)
   * @param response Health check response with server status
   * @return grpc::Status Result of the gRPC call
   *
   * @grpc Calls DetectorService::CheckHealth
   */
  grpc::Status CheckHealth(const aa::proto::CheckHealthRequest& request,
                           aa::proto::CheckHealthResponse* response) {
    return DoRequest(&aa::proto::DetectorService::Stub::CheckHealth, request,
                     response);
  }

  /**
   * @brief Process a video frame for object detection
   *
   * Sends a frame to the detector server for object detection processing
   * with polygon-based filtering and returns the detection results.
   *
   * @param request Frame processing request containing image data and polygons
   * @param response Frame processing response with detection results
   * @return grpc::Status Result of the gRPC call
   *
   * @grpc Calls DetectorService::ProcessFrame
   * @yolo Uses YOLOv7 neural network for object detection
   * @polygon Supports polygon-based detection zone filtering
   */
  grpc::Status ProcessFrame(const aa::proto::ProcessFrameRequest& request,
                            aa::proto::ProcessFrameResponse* response) {
    return DoRequest(&aa::proto::DetectorService::Stub::ProcessFrame, request,
                     response);
  }

 private:
  aa::shared::Options options_;
};

}  // namespace aa::client
