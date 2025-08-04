#pragma once

#include <opencv2/opencv.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace aa::shared {

/**
 * @brief Thread-safe frame buffer for video processing
 */
class FrameBuffer {
 public:
  explicit FrameBuffer(size_t max_size = 10);
  ~FrameBuffer() = default;

  /**
   * @brief Add frame to buffer
   * @param frame Frame to add
   * @return True if frame was added, false if buffer is full
   */
  bool PushFrame(const cv::Mat& frame);

  /**
   * @brief Get frame from buffer
   * @param frame Output frame
   * @param timeout_ms Timeout in milliseconds, 0 for no timeout
   * @return True if frame was retrieved, false if timeout or empty
   */
  bool PopFrame(cv::Mat& frame, int timeout_ms = 0);

  /**
   * @brief Check if buffer is empty
   * @return True if empty
   */
  bool Empty() const;

  /**
   * @brief Get current buffer size
   * @return Number of frames in buffer
   */
  size_t Size() const;

  /**
   * @brief Clear all frames from buffer
   */
  void Clear();

 private:
  std::queue<cv::Mat> frames_;
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  size_t max_size_;
};

}  // namespace aa::shared
