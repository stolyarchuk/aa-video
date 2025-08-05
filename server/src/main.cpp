#include <iostream>
#include <memory>
#include <signal.h>

#include "options.h"
#include "logging.h"

#include "detector_server.h"

using namespace aa::server;
using namespace aa::shared;

int main(int argc, char* argv[]) {  // Parse command line arguments
  Options options(argc, argv);

  // Check if arguments are valid
  if (!options.IsValid()) {
    options.PrintHelp();
    return 1;
  }

  Logging::Initialize(options.IsVerbose());

  // Initialize the detector server
  DetectorServer server(options);

  server.Initialize();
  server.Start();

  return 0;
}
