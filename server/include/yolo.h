#pragma once

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "options.h"
#include "types.h"

namespace aa::server {

class Yolo {
 public:
  Yolo(aa::shared::Options options);
  ~Yolo() = default;

  void Inference(cv::Mat& input,
                 std::vector<aa::shared::Detection>& detections);
  void DrawBoundingBoxes(
      cv::Mat& img, const std::vector<aa::shared::Detection>& detections) const;

 private:
  cv::dnn::Net net_;
  aa::shared::Options options_;

  cv::Scalar mean_;
  cv::Scalar scale_;

  int input_width_;
  int input_height_;

  float thr_;
  float nms_;
  float padding_value_;
  bool swap_rb_;

  cv::Size input_size_;

  void Initialize();
  auto PreProcess();
  auto PostProcess(std::vector<cv::Mat>& outs);
};

}  // namespace aa::server
