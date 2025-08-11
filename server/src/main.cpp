/**
 * @file main.cpp
 * @brief Server application entry point for AA Video Processing System
 *
 * Initializes and runs the detector gRPC server for real-time object detection.
 * Loads YOLO neural network models, configures polygon filtering, and provides
 * high-performance inference services with graceful shutdown handling.
 *
 * Features:
 * - Command line argument parsing and validation
 * - YOLO model loading and initialization
 * - gRPC server setup and lifecycle management
 * - Signal handling for graceful shutdown
 * - Comprehensive logging and error handling
 *
 * @author AA Video Processing Team
 * @version 1.2.0
 */

#include <iostream>
#include <memory>
#include <signal.h>

#include "options.h"
#include "logging.h"
#include "signal_set.h"

#include "detector_server.h"

using namespace aa::server;
using namespace aa::shared;

int main(int argc, char* argv[]) {  // Parse command line arguments
  Options options(argc, argv, "Detector Server");

  // Check if arguments are valid
  if (!options.IsValid()) {
    options.PrintHelp();
    return 1;
  }

  Logging::Initialize(options.IsVerbose());

  AA_LOG_INFO("Starting detector server...");

  // Initialize the detector server
  DetectorServer server(options);

  // Set up graceful shutdown signal handling
  SignalSet signal_set;

  // Flag to indicate shutdown request
  std::atomic<bool> shutdown_requested{false};

  // Register signal handlers for graceful shutdown
  signal_set.Add(SIGINT, [&](int sig) {
    AA_LOG_INFO("Received SIGINT (" << sig
                                    << "), requesting graceful shutdown...");
    shutdown_requested.store(true);
    server.Shutdown();
  });

  signal_set.Add(SIGTERM, [&](int sig) {
    AA_LOG_INFO("Received SIGTERM (" << sig
                                     << "), requesting graceful shutdown...");
    shutdown_requested.store(true);
    server.Shutdown();
  });

  signal_set.Add(SIGUSR1, [&](int sig) {
    AA_LOG_INFO("Received SIGUSR1 ("
                << sig << "), server status: "
                << (shutdown_requested.load() ? "shutting down" : "running"));
  });

  AA_LOG_INFO(
      "Signal handlers registered. Server will shutdown gracefully on "
      "SIGINT/SIGTERM.");
  AA_LOG_INFO("Send SIGUSR1 to check server status.");

  server.Initialize();

  try {
    server.Start();  // This will block until shutdown
  } catch (const std::exception& e) {
    AA_LOG_ERROR("Server error: " << e.what());
    return 1;
  }

  if (shutdown_requested.load()) {
    AA_LOG_INFO("Server shutdown completed gracefully.");
  }

  return 0;
}
