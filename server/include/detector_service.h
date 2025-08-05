#pragma once

#include "detector_service.grpc.pb.h"
#include "rpc_server.h"

namespace aa::server {

struct DetectorServiceMethods {
  enum { kCheckHealth = 0, kProcessFrame };

  using ObserverTable =
      std::tuple<ServiceMethod<aa::proto::CheckHealthRequest,
                               aa::proto::CheckHealthResponse>,
                 ServiceMethod<aa::proto::ProcessFrameRequest,
                               aa::proto::ProcessFrameResponse>>;
};

class DetectorServiceImpl final : public aa::proto::DetectorService::Service,
                                  public RpcServerFromThis<DetectorServiceImpl>,
                                  public Observable<DetectorServiceMethods> {
 public:
  using RpcServerFromThis::RpcServerFromThis;

  grpc::Status CheckHealth(grpc::ServerContext* context,
                           const aa::proto::CheckHealthRequest* request,
                           aa::proto::CheckHealthResponse* response) override {
    return Invoke<DetectorServiceMethods::kCheckHealth>(context, request,
                                                        response);
  }

  grpc::Status ProcessFrame(
      grpc::ServerContext* context,
      const aa::proto::ProcessFrameRequest* request,
      aa::proto::ProcessFrameResponse* response) override {
    return Invoke<DetectorServiceMethods::kProcessFrame>(context, request,
                                                         response);
  }
};

}  // namespace aa::server
