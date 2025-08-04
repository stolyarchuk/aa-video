#include "aa/shared/video_processor.h"
#include <opencv2/imgproc.hpp>

namespace aa::shared {

VideoProcessor::VideoProcessor() {
  // Initialize any required state
}

cv::Mat VideoProcessor::ApplyBlur(const cv::Mat& frame, int kernel_size) {
  cv::Mat result;
  cv::GaussianBlur(frame, result, cv::Size(kernel_size, kernel_size), 0);
  return result;
}

cv::Mat VideoProcessor::ApplyEdgeDetection(const cv::Mat& frame, double low_threshold, double high_threshold) {
  cv::Mat gray, result;

  // Convert to grayscale if needed
  if (frame.channels() == 3) {
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = frame.clone();
  }

  // Apply Canny edge detection
  cv::Canny(gray, result, low_threshold, high_threshold);

  return result;
}

cv::Mat VideoProcessor::ResizeFrame(const cv::Mat& frame, int width, int height) {
  cv::Mat result;
  cv::resize(frame, result, cv::Size(width, height));
  return result;
}

cv::Mat VideoProcessor::ConvertFormat(const cv::Mat& frame, const std::string& target_format) {
  cv::Mat result;

  if (target_format == "RGB" && frame.channels() == 3) {
    cv::cvtColor(frame, result, cv::COLOR_BGR2RGB);
  } else if (target_format == "BGR" && frame.channels() == 3) {
    result = frame.clone();
  } else if (target_format == "GRAY") {
    if (frame.channels() == 3) {
      cv::cvtColor(frame, result, cv::COLOR_BGR2GRAY);
    } else {
      result = frame.clone();
    }
  } else {
    // Default: return original frame
    result = frame.clone();
  }

  return result;
}

}  // namespace aa::shared
