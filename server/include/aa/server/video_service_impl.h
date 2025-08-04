#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>

#include "video_service.grpc.pb.h"
#include "aa/shared/video_processor.h"

namespace aa::server {

/**
 * @brief Implementation of the VideoProcessingService gRPC service
 *
 * Handles incoming video processing requests and delegates the actual
 * processing to the shared VideoProcessor class.
 */
class VideoServiceImpl final : public aa::shared::VideoProcessingService::Service {
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
   * @brief Process a single video frame
   * @param context gRPC server context
   * @param request Processing request containing frame and operation
   * @param response Processing response with result
   * @return gRPC status
   */
  grpc::Status ProcessFrame(grpc::ServerContext* context, const aa::shared::ProcessRequest* request,
                            aa::shared::ProcessResponse* response) override;

  /**
   * @brief Health check endpoint
   * @param context gRPC server context
   * @param request Health check request
   * @param response Health check response
   * @return gRPC status
   */
  grpc::Status HealthCheck(grpc::ServerContext* context, const aa::shared::HealthRequest* request,
                           aa::shared::HealthResponse* response) override;

  /**
   * @brief Stream processing endpoint for real-time video processing
   * @param context gRPC server context
   * @param stream Bidirectional stream for request/response pairs
   * @return gRPC status
   */
  grpc::Status StreamProcess(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<aa::shared::ProcessResponse, aa::shared::ProcessRequest>* stream) override;

 private:
  std::unique_ptr<aa::shared::VideoProcessor> video_processor_;
};

}  // namespace aa::server
