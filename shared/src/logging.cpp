#include "logging.h"

namespace aa::shared {

void Logging::Initialize(bool verbose) {
  if (verbose) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_DEBUG);
  } else {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_INFO);
  }
}

void Logging::SetLogLevel(cv::utils::logging::LogLevel level) {
  cv::utils::logging::setLogLevel(level);
}

cv::utils::logging::LogLevel Logging::GetLogLevel() {
  return cv::utils::logging::getLogLevel();
}

}  // namespace aa::shared
