#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

namespace aa::shared {

/**
 * @brief OpenCV logging utilities for the video processing application
 *
 * Provides convenient wrappers around OpenCV's built-in logging system.
 */
class Logging {
 public:
  /**
   * @brief Set the global log level based on verbose flag
   *
   * @param verbose If true, enables debug logging; otherwise info level
   */
  static void Initialize(bool verbose = false) {
    if (verbose) {
      cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_DEBUG);
    } else {
      cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_INFO);
    }
  }

  /**
   * @brief Set specific log level
   *
   * @param level OpenCV log level to set
   */
  static void SetLogLevel(cv::utils::logging::LogLevel level) {
    cv::utils::logging::setLogLevel(level);
  }

  /**
   * @brief Get current log level
   *
   * @return Current OpenCV log level
   */
  static cv::utils::logging::LogLevel GetLogLevel() {
    return cv::utils::logging::getLogLevel();
  }
};

}  // namespace aa::shared

// Convenience macros with project tag
#define AA_LOG_FATAL(msg) CV_LOG_FATAL(nullptr, msg)
#define AA_LOG_ERROR(msg) CV_LOG_ERROR(nullptr, msg)
#define AA_LOG_WARNING(msg) CV_LOG_WARNING(nullptr, msg)
#define AA_LOG_INFO(msg) CV_LOG_INFO(nullptr, msg)
#define AA_LOG_DEBUG(msg) CV_LOG_DEBUG(nullptr, msg)
#define AA_LOG_VERBOSE(msg) CV_LOG_VERBOSE(nullptr, msg)
