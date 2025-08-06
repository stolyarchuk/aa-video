#pragma once
#include <grpcpp/grpcpp.h>

#include "detector_service.grpc.pb.h"
#include "options.h"
#include "rpc_client.h"

namespace aa::client {

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

  grpc::Status CheckHealth(const aa::proto::CheckHealthRequest& request,
                           aa::proto::CheckHealthResponse* response) {
    return DoRequest(&aa::proto::DetectorService::Stub::CheckHealth, request,
                     response);
  }

  grpc::Status ProcessFrame(const aa::proto::ProcessFrameRequest& request,
                            aa::proto::ProcessFrameResponse* response) {
    return DoRequest(&aa::proto::DetectorService::Stub::ProcessFrame, request,
                     response);
  }

 private:
  aa::shared::Options options_;
};

}  // namespace aa::client
