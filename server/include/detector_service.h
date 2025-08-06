#pragma once

#include "detector_service.grpc.pb.h"
#include "rpc_server.h"

namespace aa::server {

/**
 * @brief Method identifier structure for detector service
 *
 * Defines the available service methods and their observer table structure
 * for the detector gRPC service. Used with the Observable pattern to
 * register and invoke method handlers.
 */
struct DetectorServiceMethods {
  /// @brief Enumeration of available service methods
  enum { kCheckHealth = 0, kProcessFrame };

  /// @brief Observer table type mapping method IDs to their signatures
  using ObserverTable =
      std::tuple<ServiceMethod<aa::proto::CheckHealthRequest,
                               aa::proto::CheckHealthResponse>,
                 ServiceMethod<aa::proto::ProcessFrameRequest,
                               aa::proto::ProcessFrameResponse>>;
};

/**
 * @brief Implementation of the detector gRPC service
 *
 * Provides the actual implementation of the DetectorService using CRTP
 * pattern with RpcServerFromThis and Observable pattern for method
 * registration. Handles health checks and frame processing requests.
 *
 * @grpc Implements aa::proto::DetectorService interface
 * @threadsafe Methods are thread-safe through gRPC framework
 *
 * Usage:
 * @code
 * DetectorServiceImpl service("localhost:50051");
 * service.Register<DetectorServiceMethods::kCheckHealth>(handler);
 * service.Build();
 * service.Wait();
 * @endcode
 */
class DetectorServiceImpl final : public aa::proto::DetectorService::Service,
                                  public RpcServerFromThis<DetectorServiceImpl>,
                                  public Observable<DetectorServiceMethods> {
 public:
  /// @brief Inherit constructors from RpcServerFromThis
  using RpcServerFromThis::RpcServerFromThis;

  /**
   * @brief Handle health check requests
   *
   * @param context gRPC server context for the request
   * @param request Health check request parameters
   * @param response Health check response to populate
   * @return grpc::Status indicating success or failure
   *
   * Invokes the registered health check handler through the Observable pattern.
   */
  grpc::Status CheckHealth(grpc::ServerContext* context,
                           const aa::proto::CheckHealthRequest* request,
                           aa::proto::CheckHealthResponse* response) override {
    return Invoke<DetectorServiceMethods::kCheckHealth>(context, request,
                                                        response);
  }

  /**
   * @brief Handle frame processing requests
   *
   * @param context gRPC server context for the request
   * @param request Frame processing request with image data
   * @param response Frame processing response to populate with detections
   * @return grpc::Status indicating success or failure
   *
   * Invokes the registered frame processing handler through the Observable
   * pattern.
   */
  grpc::Status ProcessFrame(
      grpc::ServerContext* context,
      const aa::proto::ProcessFrameRequest* request,
      aa::proto::ProcessFrameResponse* response) override {
    return Invoke<DetectorServiceMethods::kProcessFrame>(context, request,
                                                         response);
  }
};

}  // namespace aa::server
