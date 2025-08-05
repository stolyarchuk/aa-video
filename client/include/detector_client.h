#pragma once
#include <grpcpp/grpcpp.h>

#include "detector_service.grpc.pb.h"
#include "options.h"
#include "rpc_client.h"

namespace aa::client {

class DetectorClient final : public RpcClient<aa::shared::DetectorService> {
 public:
  /**
   * @brief Construct a new Detector Client object from options
   *
   * @param options Configuration options containing server address
   */
  explicit DetectorClient(aa::shared::Options options)
      : RpcClient{options.Get<std::string>("address")},
        options_{std::move(options)} {}

  grpc::Status CheckHealth(const aa::shared::CheckHealthRequest& request,
                           aa::shared::CheckHealthResponse* response) {
    return DoRequest(&aa::shared::DetectorService::Stub::CheckHealth, request,
                     response);
  }

 private:
  aa::shared::Options options_;
};

}  // namespace aa::client
