#include "aa/shared/frame_buffer.h"
#include <chrono>

namespace aa::shared {

FrameBuffer::FrameBuffer(size_t max_size) : max_size_(max_size) {}

bool FrameBuffer::PushFrame(const cv::Mat& frame) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (frames_.size() >= max_size_) {
    return false;  // Buffer is full
  }

  frames_.push(frame.clone());
  condition_.notify_one();
  return true;
}

bool FrameBuffer::PopFrame(cv::Mat& frame, int timeout_ms) {
  std::unique_lock<std::mutex> lock(mutex_);

  if (timeout_ms == 0) {
    // No timeout - wait indefinitely
    condition_.wait(lock, [this] { return !frames_.empty(); });
  } else {
    // Wait with timeout
    if (!condition_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] { return !frames_.empty(); })) {
      return false;  // Timeout
    }
  }

  if (frames_.empty()) {
    return false;
  }

  frame = frames_.front();
  frames_.pop();
  return true;
}

bool FrameBuffer::Empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return frames_.empty();
}

size_t FrameBuffer::Size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return frames_.size();
}

void FrameBuffer::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::queue<cv::Mat> empty;
  frames_.swap(empty);
}

}  // namespace aa::shared
