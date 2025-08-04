#include "detector_client.h"

using grpc::Status;

namespace aa::client {

grpc::Status CheckHealth(const aa::shared::CheckHealthRequest& request, aa::shared::CheckHealthResponse* response);

// Status AudioIOClient::SetInputState(const SetInputStateReq& request, SetInputStateRes* response) {
//   return DoRequest(&AudioIO::Stub::SetInputState, request, response);
// }

// Status AudioIOClient::GetInputState(const Empty& request, GetInputStateRes* response) {
//   return DoRequest(&AudioIO::Stub::GetInputState, request, response);
// }

// Status AudioIOClient::SetOutputState(const SetOutputStateReq& request, SetOutputStateRes* response) {
//   return DoRequest(&AudioIO::Stub::SetOutputState, request, response);
// }

// Status AudioIOClient::GetOutputState(const Empty& request, GetOutputStateRes* response) {
//   return DoRequest(&AudioIO::Stub::GetOutputState, request, response);
// }

// Status AudioIOClient::SetInputVolume(const SetInputVolumeReq& request, SetInputVolumeRes* response) {
//   return DoRequest(&AudioIO::Stub::SetInputVolume, request, response);
// }

// Status AudioIOClient::GetInputVolume(const GetInputVolumeReq& request, GetInputVolumeRes* response) {
//   return DoRequest(&AudioIO::Stub::GetInputVolume, request, response);
// }

// Status AudioIOClient::SetOutputVolume(const SetOutputVolumeReq& request, SetOutputVolumeRes* response) {
//   return DoRequest(&AudioIO::Stub::SetOutputVolume, request, response);
// }

// Status AudioIOClient::GetOutputVolume(const GetOutputVolumeReq& request, GetOutputVolumeRes* response) {
//   return DoRequest(&AudioIO::Stub::GetOutputVolume, request, response);
// }

}  // namespace aa::client
