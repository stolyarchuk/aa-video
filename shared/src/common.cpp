#include "common.h"

#include "types.h"

namespace aa::shared {

void DrawSemiTransparentRect(cv::Mat& frame, int left, int top, int right,
                             int bottom, const cv::Scalar& color, float alpha) {
  cv::Mat overlay;
  frame.copyTo(overlay);
  cv::rectangle(overlay, cv::Point(left, top), cv::Point(right, bottom), color,
                cv::FILLED);
  cv::addWeighted(overlay, alpha, frame, 1.0 - alpha, 0, frame);
}

void DrawColoredRect(cv::Mat& frame, int left, int top, int right, int bottom,
                     const cv::Scalar& color) {
  cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), color, 1,
                cv::LINE_4);
}

void DrawBoundingBox(cv::Mat& frame, int left, int top, int right, int bottom,
                     int class_id, float conf, const cv::Scalar& color,
                     bool filled) {
  if (filled) {
    DrawSemiTransparentRect(frame, left, top, right, bottom, color, 0.3f);
  }

  DrawColoredRect(frame, left, top, right, bottom, color);

  std::string label = cv::format("%.2f", conf);

  if (!kCocoClasses.empty()) {
    CV_Assert(class_id < static_cast<int>(kCocoClasses.size()));
    label = std::string(kCocoClasses[class_id]) + ": " + label;
  } else {
    label = "class_" + std::to_string(class_id) + ": " + label;
  }

  int base_line;
  cv::Size label_size =
      cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &base_line);

  top = std::max(top, label_size.height);
  cv::rectangle(frame, cv::Point(left, top - label_size.height),
                cv::Point(left + label_size.width, top + base_line), color,
                cv::FILLED);
  cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5,
              cv::Scalar::all(255), 1, cv::LINE_AA);
}

}  // namespace aa::shared
