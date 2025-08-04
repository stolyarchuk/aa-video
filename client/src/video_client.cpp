#include "aa/client/video_client.h"
#include <iostream>

namespace aa::client {

VideoClient::VideoClient(const std::string& server_address) : server_address_(server_address) {
  auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  stub_ = aa::shared::VideoProcessingService::NewStub(channel);
}

bool VideoClient::IsConnected() {
  grpc::ClientContext context;
  aa::shared::HealthRequest request;
  aa::shared::HealthResponse response;

  auto status = stub_->HealthCheck(&context, request, &response);
  return status.ok() && response.healthy();
}

bool VideoClient::ProcessFrame(const cv::Mat& frame, const std::string& operation,
                               const std::map<std::string, std::string>& parameters, cv::Mat& result) {
  grpc::ClientContext context;
  aa::shared::ProcessRequest request;
  aa::shared::ProcessResponse response;

  // Convert Mat to VideoFrame
  MatToVideoFrame(frame, request.mutable_frame());
  request.set_operation(operation);

  // Set parameters
  for (const auto& param : parameters) {
    (*request.mutable_parameters())[param.first] = param.second;
  }

  auto status = stub_->ProcessFrame(&context, request, &response);

  if (status.ok() && response.success()) {
    VideoFrameToMat(response.result(), result);
    return true;
  }

  if (!response.error_message().empty()) {
    std::cerr << "Processing error: " << response.error_message() << "\n";
  }

  return false;
}

bool VideoClient::StartStreamProcessing(const std::string& operation,
                                        const std::map<std::string, std::string>& parameters) {
  stream_context_ = std::make_unique<grpc::ClientContext>();
  stream_ = stub_->StreamProcess(stream_context_.get());

  if (!stream_) {
    return false;
  }

  // Send initial configuration (empty frame with operation)
  aa::shared::ProcessRequest config_request;
  config_request.set_operation(operation);
  for (const auto& param : parameters) {
    (*config_request.mutable_parameters())[param.first] = param.second;
  }

  return stream_->Write(config_request);
}

bool VideoClient::SendFrame(const cv::Mat& frame) {
  if (!stream_) {
    return false;
  }

  aa::shared::ProcessRequest request;
  MatToVideoFrame(frame, request.mutable_frame());

  return stream_->Write(request);
}

bool VideoClient::ReceiveFrame(cv::Mat& result) {
  if (!stream_) {
    return false;
  }

  aa::shared::ProcessResponse response;
  if (stream_->Read(&response) && response.success()) {
    VideoFrameToMat(response.result(), result);
    return true;
  }

  return false;
}

void VideoClient::StopStreamProcessing() {
  if (stream_) {
    stream_->WritesDone();
    auto status = stream_->Finish();
    stream_.reset();
    stream_context_.reset();
  }
}

void VideoClient::MatToVideoFrame(const cv::Mat& mat, aa::shared::VideoFrame* frame) {
  frame->set_width(mat.cols);
  frame->set_height(mat.rows);
  frame->set_channels(mat.channels());

  // Set format based on channels
  if (mat.channels() == 1) {
    frame->set_format("GRAY");
  } else if (mat.channels() == 3) {
    frame->set_format("BGR");
  } else {
    frame->set_format("UNKNOWN");
  }

  // Copy image data
  if (mat.isContinuous()) {
    frame->set_data(mat.data, mat.total() * mat.elemSize());
  } else {
    // Handle non-continuous matrices
    cv::Mat continuous = mat.clone();
    frame->set_data(continuous.data, continuous.total() * continuous.elemSize());
  }

  // Set timestamp
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                 .count();
  frame->set_timestamp(now);
}

void VideoClient::VideoFrameToMat(const aa::shared::VideoFrame& frame, cv::Mat& mat) {
  int cv_type;
  if (frame.channels() == 1) {
    cv_type = CV_8UC1;
  } else if (frame.channels() == 3) {
    cv_type = CV_8UC3;
  } else {
    // Default to single channel
    cv_type = CV_8UC1;
  }

  mat = cv::Mat(frame.height(), frame.width(), cv_type);

  if (mat.total() * mat.elemSize() == frame.data().size()) {
    std::memcpy(mat.data, frame.data().data(), frame.data().size());
  }
}

}  // namespace aa::client
