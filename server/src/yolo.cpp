#include "yolo.h"

#include "common.h"
#include "logging.h"

namespace {
const int kNumClasses = 80;
const auto kPaddingMode = cv::dnn::ImagePaddingMode::DNN_PMODE_LETTERBOX;
const cv::Scalar kDefaultMean = cv::Scalar::all(0.0);
const cv::Scalar kDefaultScale = cv::Scalar::all(1.0 / 255);

}  // namespace

namespace aa::server {

Yolo::Yolo(aa::shared::Options options) : options_{std::move(options)} {
  input_width_ = options_.Get<int>("width");
  input_height_ = options_.Get<int>("height");
  input_size_ = cv::Size(input_width_, input_height_);

  mean_ = options_.Has("mean") ? cv::Scalar::all(options_.Get<int>("mean"))
                               : kDefaultMean;
  scale_ = options_.Has("scale") ? cv::Scalar::all(options_.Get<int>("scale"))
                                 : kDefaultScale;

  thr_ = options_.Get<float>("thr");
  nms_ = options_.Get<float>("nms");
  padding_value_ = options_.Get<float>("padvalue");
  swap_rb_ = options_.Get<bool>("rgb");

  Initialize();
}

auto Yolo::PreProcess() {
  cv::dnn::Image2BlobParams img_params(scale_, input_size_, mean_, swap_rb_,
                                       CV_32F, cv::dnn::DNN_LAYOUT_NCHW,
                                       kPaddingMode, padding_value_);

  cv::dnn::Image2BlobParams net_params{};
  net_params.scalefactor = scale_;
  net_params.size = input_size_;
  net_params.mean = mean_;
  net_params.swapRB = swap_rb_;
  net_params.paddingmode = kPaddingMode;

  return std::make_pair(img_params, net_params);
}

auto Yolo::PostProcess(std::vector<cv::Mat>& outs) {
  std::vector<int> class_ids;
  std::vector<float> confidences;
  std::vector<cv::Rect2d> boxes;
  std::vector<aa::shared::Detection> detections;

  CV_CheckEQ(
      outs[0].dims, 3,
      "Invalid output shape. The shape should be [1, #anchors, nc+5 or nc+4]");
  CV_CheckEQ((outs[0].size[2] == kNumClasses + 5 ||
              outs[0].size[2] == kNumClasses + 4),
             true, "Invalid output shape: ");

  for (auto preds : outs) {
    preds = preds.reshape(1, preds.size[1]);

    for (int i = 0; i < preds.rows; ++i) {
      float obj_conf = preds.at<float>(i, 4);
      if (obj_conf < thr_) continue;

      cv::Mat scores = preds.row(i).colRange(5, preds.cols);
      double conf;
      cv::Point max_loc;
      cv::minMaxLoc(scores, 0, &conf, 0, &max_loc);
      conf = conf * obj_conf;
      if (conf < thr_) continue;

      float* det = preds.ptr<float>(i);
      double cx = det[0];
      double cy = det[1];
      double w = det[2];
      double h = det[3];

      boxes.push_back(
          cv::Rect2d(cx - 0.5 * w, cy - 0.5 * h, cx + 0.5 * w, cy + 0.5 * h));
      class_ids.push_back(max_loc.x);
      confidences.push_back(static_cast<float>(conf));
    }
  }

  std::vector<int> keep_idx;
  cv::dnn::NMSBoxes(boxes, confidences, thr_, nms_, keep_idx);

  detections.clear();
  for (auto i : keep_idx) {
    aa::shared::Detection detection;
    detection.class_id = class_ids[i];
    detection.confidence = confidences[i];
    cv::Rect2d rect2d = boxes[i];
    detection.bbox = cv::Rect(cvFloor(rect2d.x), cvFloor(rect2d.y),
                              cvFloor(rect2d.width - rect2d.x),
                              cvFloor(rect2d.height - rect2d.y));
    detections.push_back(detection);
  }

  return detections;
}

void Yolo::Inference(cv::Mat& img,
                     std::vector<aa::shared::Detection>& detections) {
  auto&& [img_params, net_params] = PreProcess();
  auto input = cv::dnn::blobFromImageWithParams(img, img_params);

  net_.setInput(input);

  std::vector<cv::Mat> outs;
  net_.forward(outs, net_.getUnconnectedOutLayersNames());

  detections = PostProcess(outs);

  std::vector<cv::Rect> boxes;
  for (const auto& detection : detections) {
    boxes.push_back(detection.bbox);
  }

  net_params.blobRectsToImageRects(boxes, boxes, img.size());

  for (std::size_t i = 0; i < detections.size(); ++i) {
    detections[i].bbox = boxes[i];
  }
}

void Yolo::DrawBoundingBoxes(
    cv::Mat& img, const std::vector<aa::shared::Detection>& detections) const {
  for (const auto& detection : detections) {
    cv::Rect box = detection.bbox;
    aa::shared::DrawBoundingBox(img, box.x, box.y, box.width + box.x,
                                box.height + box.y, detection.class_id,
                                detection.confidence, aa::shared::Color::kRed,
                                true);
  }
}

void Yolo::Initialize() {
  net_ = cv::dnn::readNet(options_.Get<std::string>("model"));
  net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
  net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

}  // namespace aa::server
