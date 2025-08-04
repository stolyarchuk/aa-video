#include "aa/server/video_server.h"

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>

#include "aa/server/video_service_impl.h"

namespace aa::server {

VideoServer::VideoServer(const std::string& server_address) : server_address_(server_address) {
  // TODO: Initialize server components
}

void VideoServer::Run() {
  // TODO: Implement server startup and main loop
  // 1. Create ServerBuilder
  // 2. Add listening port
  // 3. Register VideoServiceImpl service
  // 4. Build and start the server
  // 5. Wait for server shutdown

  std::cout << "VideoServer::Run() - Implementation needed\n";
}

void VideoServer::Shutdown() {
  // TODO: Implement graceful server shutdown
  // 1. Stop accepting new requests
  // 2. Wait for ongoing requests to complete
  // 3. Shutdown the server

  std::cout << "VideoServer::Shutdown() - Implementation needed\n";
}

}  // namespace aa::server
