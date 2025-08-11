#pragma once

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "options.h"
#include "types.h"

namespace aa::server {

/**
 * @brief YOLO object detection neural network inference engine
 *
 * Provides real-time object detection using YOLO neural networks with
 * support for multiple model formats (YOLOv7, YOLOv10, YOLOv11, YOLOX).
 * Handles preprocessing, inference, and postprocessing with OpenCV DNN.
 *
 * Features:
 * - Multi-YOLO model support (.onnx, .weights+.cfg)
 * - Letterboxing preprocessing for aspect ratio preservation
 * - Non-Maximum Suppression (NMS) for duplicate filtering
 * - COCO dataset class support (80 classes)
 * - Real-time performance optimization
 *
 * @yolo Compatible with YOLO model architectures
 * @coco Outputs COCO dataset class IDs (0-79)
 * @opencv Uses OpenCV DNN module for inference
 * @performance Optimized for CPU inference (~100-200ms per frame)
 * @memorysafe Input validation and bounds checking
 */
class Yolo {
 public:
  /**
   * @brief Construct YOLO inference engine with configuration options
   *
   * Initializes neural network, preprocessing parameters, and model-specific
   * settings from command line options. Automatically detects model format
   * and configures appropriate preprocessing pipeline.
   *
   * @param options Configuration options containing model path, dimensions,
   *                thresholds, and preprocessing parameters
   * @yolo Supports YOLOv7, YOLOv10, YOLOv11, and YOLOX models
   * @validation Validates model file existence and format compatibility
   * @throws std::runtime_error if model loading fails
   */
  Yolo(aa::shared::Options options);

  /**
   * @brief Default destructor with automatic cleanup
   */
  ~Yolo() = default;

  /**
   * @brief Perform object detection inference on input image
   *
   * Executes complete YOLO pipeline: preprocessing, neural network inference,
   * postprocessing with NMS, and coordinate transformation. Input image is
   * processed with letterboxing to maintain aspect ratio.
   *
   * @param input Input image in OpenCV Mat format (any size, BGR)
   * @param detections Output vector of detected objects with bounding boxes,
   *                   class IDs, and confidence scores
   * @yolo Compatible with all supported YOLO model formats
   * @coco Returns COCO dataset class IDs (0-79)
   * @performance Optimized for real-time CPU inference
   * @memorysafe Input validation and bounds checking
   */
  void Inference(cv::Mat& input,
                 std::vector<aa::shared::Detection>& detections);

  /**
   * @brief Draw detection bounding boxes on image for visualization
   *
   * Renders semi-transparent filled rectangles with class labels and
   * confidence scores. Uses predefined color palette for consistent
   * visualization across different object classes.
   *
   * @param img Input/output image to draw on (modified in-place)
   * @param detections Vector of detections to visualize
   * @opencv Uses OpenCV drawing functions for rendering
   * @coco Displays COCO class names with confidence scores
   */
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
