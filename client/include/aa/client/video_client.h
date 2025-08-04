#pragma once

#include <grpcpp/grpcpp.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <string>

#include "aa/shared/config.h"
#include "video_service.grpc.pb.h"

namespace aa::client {

/**
 * @brief Video processing client for communicating with server
 */
class VideoClient {
 public:
  /**
   * @brief Constructor
   * @param server_address Server address (e.g., "localhost:50051")
   */
  explicit VideoClient(const std::string& server_address);

  /**
   * @brief Destructor
   */
  ~VideoClient() = default;

  /**
   * @brief Check if client is connected to server
   * @return True if connected
   */
  bool IsConnected();

  /**
   * @brief Process a single frame
   * @param frame Input frame
   * @param operation Operation to perform (e.g., "blur", "edge_detect")
   * @param parameters Operation parameters
   * @param result Output frame
   * @return True if processing succeeded
   */
  bool ProcessFrame(const cv::Mat& frame, const std::string& operation,
                    const std::map<std::string, std::string>& parameters, cv::Mat& result);

  /**
   * @brief Start streaming processing session
   * @param operation Operation to perform
   * @param parameters Operation parameters
   * @return True if stream started successfully
   */
  bool StartStreamProcessing(const std::string& operation, const std::map<std::string, std::string>& parameters);

  /**
   * @brief Send frame for streaming processing
   * @param frame Input frame
   * @return True if frame sent successfully
   */
  bool SendFrame(const cv::Mat& frame);

  /**
   * @brief Receive processed frame from streaming
   * @param result Output frame
   * @return True if frame received successfully
   */
  bool ReceiveFrame(cv::Mat& result);

  /**
   * @brief Stop streaming processing session
   */
  void StopStreamProcessing();

 private:
  std::unique_ptr<aa::shared::VideoProcessingService::Stub> stub_;
  std::unique_ptr<grpc::ClientContext> stream_context_;
  std::unique_ptr<grpc::ClientReaderWriter<aa::shared::ProcessRequest, aa::shared::ProcessResponse>> stream_;
  std::string server_address_;

  /**
   * @brief Convert OpenCV Mat to protobuf VideoFrame
   * @param mat Input OpenCV Mat
   * @param frame Output protobuf VideoFrame
   */
  void MatToVideoFrame(const cv::Mat& mat, aa::shared::VideoFrame* frame);

  /**
   * @brief Convert protobuf VideoFrame to OpenCV Mat
   * @param frame Input protobuf VideoFrame
   * @param mat Output OpenCV Mat
   */
  void VideoFrameToMat(const aa::shared::VideoFrame& frame, cv::Mat& mat);
};

}  // namespace aa::client
