#pragma once

#include <opencv2/opencv.hpp>
#include <memory>
#include <string>

namespace aa::shared {

/**
 * @brief Core video processing functionality
 */
class VideoProcessor {
 public:
  VideoProcessor();
  ~VideoProcessor() = default;

  /**
   * @brief Apply blur filter to frame
   * @param frame Input frame
   * @param kernel_size Blur kernel size
   * @return Processed frame
   */
  cv::Mat ApplyBlur(const cv::Mat& frame, int kernel_size = 15);

  /**
   * @brief Apply edge detection to frame
   * @param frame Input frame
   * @param low_threshold Low threshold for Canny edge detection
   * @param high_threshold High threshold for Canny edge detection
   * @return Processed frame
   */
  cv::Mat ApplyEdgeDetection(const cv::Mat& frame, double low_threshold = 50.0, double high_threshold = 150.0);

  /**
   * @brief Resize frame
   * @param frame Input frame
   * @param width Target width
   * @param height Target height
   * @return Resized frame
   */
  cv::Mat ResizeFrame(const cv::Mat& frame, int width, int height);

  /**
   * @brief Convert frame format
   * @param frame Input frame
   * @param target_format Target format (e.g., "BGR", "RGB", "GRAY")
   * @return Converted frame
   */
  cv::Mat ConvertFormat(const cv::Mat& frame, const std::string& target_format);

 private:
  // Internal state if needed
};

}  // namespace aa::shared
