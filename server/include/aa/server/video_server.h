#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

namespace aa::server {

/**
 * @brief Main video processing server class
 *
 * Manages the gRPC server lifecycle and handles video processing requests
 * from clients using OpenCV and the shared video processing functionality.
 */
class VideoServer {
 public:
  /**
   * @brief Construct a new Video Server object
   * @param server_address Server address to bind to (e.g., "0.0.0.0:50051")
   */
  explicit VideoServer(const std::string& server_address);

  /**
   * @brief Destroy the Video Server object
   */
  ~VideoServer() = default;

  // Non-copyable and non-movable
  VideoServer(const VideoServer&) = delete;
  VideoServer& operator=(const VideoServer&) = delete;
  VideoServer(VideoServer&&) = delete;
  VideoServer& operator=(VideoServer&&) = delete;

  /**
   * @brief Start the server and begin listening for requests
   *
   * This method blocks until the server is shut down.
   */
  void Run();

  /**
   * @brief Shutdown the server gracefully
   */
  void Shutdown();

 private:
  std::string server_address_;
  std::unique_ptr<grpc::Server> server_;
  std::unique_ptr<grpc::ServerBuilder> builder_;
};

}  // namespace aa::server
