#pragma once
#include <grpcpp/grpcpp.h>

#include "detector_service.grpc.pb.h"
#include "rpc_client.h"

namespace aa::client {

class DetectorClient final : public RpcClient<aa::shared::DetectorService> {
 public:
  using RpcClient::RpcClient;

  grpc::Status CheckHealth(const aa::shared::CheckHealthRequest& request, aa::shared::CheckHealthResponse* response);
  // grpc::Status GetInputState(const Empty& request, GetInputStateRes* response);

  // grpc::Status SetOutputState(const SetOutputStateReq& request, SetOutputStateRes* response);
  // grpc::Status GetOutputState(const Empty& request, GetOutputStateRes* response);

  // grpc::Status SetInputVolume(const SetInputVolumeReq& request, SetInputVolumeRes* response);
  // grpc::Status GetInputVolume(const GetInputVolumeReq& request, GetInputVolumeRes* response);

  // grpc::Status SetOutputVolume(const SetOutputVolumeReq& request, SetOutputVolumeRes* response);
  // grpc::Status GetOutputVolume(const GetOutputVolumeReq& request, GetOutputVolumeRes* response);
};

}  // namespace aa::client
