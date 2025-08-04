#include <iostream>
#include <memory>
#include <signal.h>

#include "aa/server/video_server.h"
#include "aa/shared/config.h"

namespace {
std::unique_ptr<aa::server::VideoServer> g_server;

void SignalHandler(int signal) {
  std::cout << "\nReceived signal " << signal << ". Shutting down server...\n";
  if (g_server) {
    g_server->Shutdown();
  }
}
}  // namespace

void PrintUsage(const char* program_name) {
  std::cout << "Usage: " << program_name << " [options]\n";
  std::cout << "Options:\n";
  std::cout << "  --port <port>         Server port (default: 50051)\n";
  std::cout << "  --address <address>   Server address (default: 0.0.0.0)\n";
  std::cout << "  --help                Show this help\n";
}

int main(int argc, char* argv[]) {
  std::string server_address = aa::shared::Config::kDefaultServerAddress;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--port" && i + 1 < argc) {
      std::string port = argv[++i];
      // Extract address part if present, otherwise use default
      size_t colon_pos = server_address.find(':');
      if (colon_pos != std::string::npos) {
        server_address = server_address.substr(0, colon_pos + 1) + port;
      } else {
        server_address = "0.0.0.0:" + port;
      }
    } else if (arg == "--address" && i + 1 < argc) {
      std::string address = argv[++i];
      // Extract port part if present, otherwise use default
      size_t colon_pos = server_address.find(':');
      if (colon_pos != std::string::npos) {
        server_address = address + server_address.substr(colon_pos);
      } else {
        server_address = address + ":50051";
      }
    } else if (arg == "--help") {
      PrintUsage(argv[0]);
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      PrintUsage(argv[0]);
      return 1;
    }
  }

  std::cout << "Video Processing Server\n";
  std::cout << "Server address: " << server_address << "\n\n";

  // Set up signal handling for graceful shutdown
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  try {
    // Create and start server
    g_server = std::make_unique<aa::server::VideoServer>(server_address);

    std::cout << "Starting server on " << server_address << "...\n";
    g_server->Run();  // This blocks until shutdown

    std::cout << "Server stopped.\n";
  } catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
