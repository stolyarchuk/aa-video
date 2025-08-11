#include "detector_server.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "common.h"
#include "frame.h"
#include "logging.h"
#include "polygon.h"

namespace {
// hardcoded stuff
const cv::Scalar kScale = 0.003921568627;
const cv::Scalar kMean = 0;
const float kPadValue = 144.0f;
const auto kPaddingMode = cv::dnn::ImagePaddingMode::DNN_PMODE_LETTERBOX;

}  // namespace

namespace aa::server {

DetectorServer::DetectorServer(aa::shared::Options options)
    : options_{std::move(options)}, yolo_{options_} {
  service_ = std::make_unique<DetectorServiceImpl>(
      options_.Get<std::string>("address"));
}

void DetectorServer::Initialize() {
  service_->Register<DetectorServiceMethods::kCheckHealth>(
      [this](auto request, auto response) {
        return CheckHealth(request, response);
      });
  service_->Register<DetectorServiceMethods::kProcessFrame>(
      [this](auto request, auto response) {
        return ProcessFrame(request, response);
      });
}

void DetectorServer::Start() {
  service_->Build();
  service_->Wait();
}

void DetectorServer::Shutdown() { service_->Stop(); }

grpc::Status DetectorServer::CheckHealth(
    const aa::proto::CheckHealthRequest*,
    aa::proto::CheckHealthResponse*) const {
  // For now, we assume the server is always healthy
  AA_LOG_INFO("Health check passed");

  return grpc::Status::OK;
}

grpc::Status DetectorServer::ProcessFrame(
    const aa::proto::ProcessFrameRequest* request,
    aa::proto::ProcessFrameResponse* response) const {
  try {
    if (request->polygons_size() == 0) {
      AA_LOG_ERROR("No polygons provided in request");
      response->set_success(false);
      return grpc::Status::OK;
    }

    std::vector<aa::shared::Polygon> polygons;
    polygons.reserve(request->polygons_size());

    for (int i = 0; i < request->polygons_size(); ++i) {
      auto polygon = aa::shared::Polygon::FromProto(request->polygons(i));

      if (polygon.GetType() == aa::shared::PolygonType::UNSPECIFIED) {
        AA_LOG_WARNING("Skipping polygon at index "
                       << i << " with UNSPECIFIED type");
        continue;
      }

      polygons.push_back(std::move(polygon));
    }

    if (polygons.empty()) {
      AA_LOG_ERROR(
          "No valid polygons found after filtering out UNSPECIFIED types");
      response->set_success(false);
      return grpc::Status::OK;
    }

    std::sort(polygons.begin(), polygons.end(),
              [](const aa::shared::Polygon& a, const aa::shared::Polygon& b) {
                return a.GetPriority() > b.GetPriority();
              });

    auto img = aa::shared::Frame::FromProto(request->frame()).ToMat();

    std::vector<aa::shared::Detection> outs;
    const_cast<Yolo&>(yolo_).Inference(img, outs);

    const_cast<PolygonFilter&>(polygon_filter_)
        .SetPolygons(std::move(polygons));
    auto filtered = const_cast<PolygonFilter&>(polygon_filter_)
                        .FilterDetectionsByPolygons(outs);

    polygon_filter_.DrawPolygonBoundingBoxes(img);
    yolo_.DrawBoundingBoxes(img, filtered);

    auto result_frame = aa::shared::Frame(img);
    auto proto_result_frame = result_frame.ToProto();

    response->mutable_result()->CopyFrom(proto_result_frame);
    response->set_success(true);

    AA_LOG_INFO("Processed frame successfully. Found " << outs.size()
                                                       << " detections.");
    return grpc::Status::OK;
  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error processing frame: " << e.what());
    return grpc::Status(grpc::StatusCode::INTERNAL, "Frame processing failed");
  }
}

/*
void DetectorServer::DrawBoundingBoxes(
    cv::Mat& frame, const std::vector<Detection>& detections) const {
  for (const auto& detection : detections) {
    // Choose color based on class ID
    cv::Scalar color = aa::shared::TupleToScalar(
        aa::shared::kClassColors[detection.class_id %
aa::shared::kClassColors.size()]);

    // Draw bounding box rectangle
    cv::rectangle(frame, detection.bbox, color, 1);

    // Prepare label text
    std::string class_name =
        (detection.class_id < static_cast<int>(aa::shared::kCocoClasses.size()))
            ? std::string{aa::shared::kCocoClasses[detection.class_id]}
            : "class_" + std::to_string(detection.class_id);

    std::string label =
        class_name + " " +
        std::to_string(static_cast<int>(detection.confidence * 100)) + "%";

    // Calculate text size for background rectangle
    int baseline = 0;
    cv::Size text_size =
        cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);

    // Draw background rectangle for text
    cv::Point text_origin(detection.bbox.x, detection.bbox.y - 5);
    cv::Rect text_rect(text_origin.x, text_origin.y - text_size.height,
                       text_size.width, text_size.height + baseline);
    cv::rectangle(frame, text_rect, color, cv::FILLED);

    // Draw text label
    cv::putText(frame, label, text_origin, cv::FONT_HERSHEY_SIMPLEX, 0.5,
                cv::Scalar(255, 255, 255), 1);
  }
} */

}  // namespace aa::server
