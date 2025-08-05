#pragma once

#include "detector_service.grpc.pb.h"
#include "rpc_server.h"

namespace aa::server {

struct DetectorServiceMethods {
  enum { kCheckHealth = 0, kProcessFrame };

  using ObserverTable =
      std::tuple<ServiceMethod<aa::shared::CheckHealthRequest,
                               aa::shared::CheckHealthResponse>>;
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
};

}  // namespace aa::server
