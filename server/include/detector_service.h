#pragma once

#include "detector_service.grpc.pb.h"
#include "rpc_server.h"

namespace aa::server {

struct DetectorServiceMethods {
  enum { kCheckHealth = 0, kProcessFrame };

  using ObserverTable =
      std::tuple<ServiceMethod<aa::shared::CheckHealthRequest,
                               aa::shared::CheckHealthResponse>,
                 ServiceMethod<aa::shared::ProcessFrameRequest,
                               aa::shared::ProcessFrameResponse>>;
};

class DetectorServiceImpl final : public aa::shared::DetectorService::Service,
                                  public RpcServerFromThis<DetectorServiceImpl>,
                                  public Observable<DetectorServiceMethods> {
 public:
  using RpcServerFromThis::RpcServerFromThis;

  grpc::Status CheckHealth(grpc::ServerContext* context,
                           const aa::shared::CheckHealthRequest* request,
                           aa::shared::CheckHealthResponse* response) override {
    return Invoke<DetectorServiceMethods::kCheckHealth>(context, request,
                                                        response);
  }

  grpc::Status ProcessFrame(
      grpc::ServerContext* context,
      const aa::shared::ProcessFrameRequest* request,
      aa::shared::ProcessFrameResponse* response) override {
    return Invoke<DetectorServiceMethods::kProcessFrame>(context, request,
                                                         response);
  }
};

}  // namespace aa::server
