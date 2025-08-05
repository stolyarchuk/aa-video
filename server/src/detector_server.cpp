#include "detector_server.h"

namespace aa::server {

DetectorServer::DetectorServer(std::string_view address) {
  service_ = std::make_unique<DetectorServiceImpl>(address);
}

DetectorServer::DetectorServer(const aa::shared::Options &options) {
  service_ = std::make_unique<DetectorServiceImpl>(options.GetAddress());
}

void DetectorServer::Initialize() {
  service_->Register<DetectorServiceMethods::kCheckHealth>(
      [this](auto request, auto response) {
        return CheckHealth(request, response);
      });
  service_->Register<DetectorServiceMethods::kProcessFrame>(
      [this](auto request, auto response) {
        return ProcessFrame(request, response);
      });
}

void DetectorServer::Start() {
  service_->Build();
  service_->Wait();
}

void DetectorServer::Shutdown() { service_->Stop(); }

grpc::Status DetectorServer::CheckHealth(
    const aa::shared::CheckHealthRequest *,
    aa::shared::CheckHealthResponse *) const {
  return grpc::Status();
}

grpc::Status DetectorServer::ProcessFrame(
    const aa::shared::ProcessFrameRequest *,
    aa::shared::ProcessFrameResponse *) const {
  return grpc::Status();
}

}  // namespace aa::server
