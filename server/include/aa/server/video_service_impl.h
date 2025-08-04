#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>

#include "detector_service.grpc.pb.h"
// #include "aa/shared/video_processor.h"

namespace aa::server {

/**
 * @brief Implementation of the VideoProcessingService gRPC service
 *
 * Handles incoming video processing requests and delegates the actual
 * processing to the shared VideoProcessor class.
 */
class VideoServiceImpl final : public aa::shared::DetectorService::Service {
 public:
  /**
   * @brief Construct a new Video Service Impl object
   */
  VideoServiceImpl();

  /**
   * @brief Destroy the Video Service Impl object
   */
  ~VideoServiceImpl() override = default;

  // Non-copyable and non-movable
  VideoServiceImpl(const VideoServiceImpl&) = delete;
  VideoServiceImpl& operator=(const VideoServiceImpl&) = delete;
  VideoServiceImpl(VideoServiceImpl&&) = delete;
  VideoServiceImpl& operator=(VideoServiceImpl&&) = delete;

  /**
   * @brief Health check endpoint
   * @param context gRPC server context
   * @param request Health check request
   * @param response Health check response
   * @return gRPC status
   */
  grpc::Status CheckHealth(grpc::ServerContext* context, const aa::shared::CheckHealthRequest* request,
                           aa::shared::CheckHealthResponse* response) override;

 private:
  // Implementation details will be added later
};

}  // namespace aa::server
