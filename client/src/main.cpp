#include <chrono>
#include <thread>

#include <opencv2/opencv.hpp>

#include "detector_client.h"
#include "logging.h"
#include "options.h"

using namespace aa::client;
using namespace aa::shared;

int main(int argc, char* argv[]) {
  // Parse command line arguments
  Options options(argc, argv, "Detector Client");

  // Check if arguments are valid
  if (!options.IsValid()) {
    options.PrintHelp();
    return 1;
  }

  Logging::Initialize(options.IsVerbose());

  // Create client
  DetectorClient client(options);

  aa::proto::CheckHealthRequest health_request;
  aa::proto::CheckHealthResponse health_response;

  grpc::Status status = client.CheckHealth(health_request, &health_response);

  if (!status.ok()) {
    AA_LOG_ERROR("Health check failed: " << status.error_message());
    return 1;
  } else {
    AA_LOG_INFO("Health check passed");
  }

  return 0;
}
