#include "polygon_filter.h"

#include "common.h"

namespace aa::server {

PolygonFilter::PolygonFilter() = default;
PolygonFilter::~PolygonFilter() = default;

std::pair<double, double> PolygonFilter::GetDetectionCenter(
    const aa::shared::Detection& detection) {
  const cv::Rect& bbox = detection.bbox;
  double center_x = bbox.x + bbox.width / 2.0;
  double center_y = bbox.y + bbox.height / 2.0;
  return {center_x, center_y};
}

std::vector<const aa::shared::Polygon*> PolygonFilter::FindContainingPolygons(
    double center_x, double center_y) {
  std::vector<const aa::shared::Polygon*> containing_polygons;

  for (const auto& polygon : polygons_) {
    if (polygon.Contains(center_x, center_y)) {
      containing_polygons.push_back(&polygon);
    }
  }

  return containing_polygons;
}

bool PolygonFilter::ShouldIncludeDetection(
    const aa::shared::Detection& detection,
    const std::vector<const aa::shared::Polygon*>& containing_polygons) {
  if (containing_polygons.empty()) {
    return false;
  }

  const aa::shared::Polygon* highest_priority_polygon = containing_polygons[0];

  if (highest_priority_polygon->GetType() ==
      aa::shared::PolygonType::EXCLUSION) {
    return false;
  } else if (highest_priority_polygon->GetType() ==
             aa::shared::PolygonType::INCLUSION) {
    return IsDetectionClassAllowed(detection, *highest_priority_polygon);
  }

  return false;
}

bool PolygonFilter::IsDetectionClassAllowed(
    const aa::shared::Detection& detection,
    const aa::shared::Polygon& polygon) {
  const auto& target_classes = polygon.GetTargetClasses();

  if (target_classes.empty()) {
    return true;
  }

  auto it = std::find(target_classes.begin(), target_classes.end(),
                      detection.class_id);
  return it != target_classes.end();
}

std::vector<aa::shared::Detection> PolygonFilter::FilterDetectionsByPolygons(
    const std::vector<aa::shared::Detection>& detections) {
  std::vector<aa::shared::Detection> filtered_detections;

  for (const auto& detection : detections) {
    auto [center_x, center_y] = GetDetectionCenter(detection);

    auto containing_polygons = FindContainingPolygons(center_x, center_y);

    if (containing_polygons.empty()) {
      continue;
    }

    std::sort(containing_polygons.begin(), containing_polygons.end(),
              [](const aa::shared::Polygon* a, const aa::shared::Polygon* b) {
                return a->GetPriority() > b->GetPriority();
              });

    if (ShouldIncludeDetection(detection, containing_polygons)) {
      filtered_detections.push_back(detection);
    }
  }

  return filtered_detections;
}

void PolygonFilter::SetPolygons(std::vector<aa::shared::Polygon>&& polygons) {
  polygons_ = std::move(polygons);
}

void PolygonFilter::DrawPolygonBoundingBoxes(cv::Mat& frame) const {
  for (size_t i = 0; i < polygons_.size(); ++i) {
    const auto& polygon = polygons_[i];
    const auto& vertices = polygon.GetVertices();

    if (vertices.size() < 3) {
      continue;  // Skip invalid polygons
    }

    // Calculate bounding box of the polygon
    double min_x = vertices[0].GetX();
    double max_x = vertices[0].GetX();
    double min_y = vertices[0].GetY();
    double max_y = vertices[0].GetY();

    for (const auto& vertex : vertices) {
      min_x = std::min(min_x, vertex.GetX());
      max_x = std::max(max_x, vertex.GetX());
      min_y = std::min(min_y, vertex.GetY());
      max_y = std::max(max_y, vertex.GetY());
    }

    // Convert to integer coordinates
    int left = static_cast<int>(std::max(0.0, min_x));
    int top = static_cast<int>(std::max(0.0, min_y));
    int right =
        static_cast<int>(std::min(static_cast<double>(frame.cols), max_x));
    int bottom =
        static_cast<int>(std::min(static_cast<double>(frame.rows), max_y));

    cv::Scalar color = aa::shared::Color::kGrey;
    if (polygon.GetType() == aa::shared::PolygonType::INCLUSION) {
      color = aa::shared::Color::kGreen;
    } else if (polygon.GetType() == aa::shared::PolygonType::EXCLUSION) {
      color = aa::shared::Color::kGrey;
    }

    aa::shared::DrawColoredRect(frame, left, top, right, bottom, color);
    aa::shared::DrawSemiTransparentRect(frame, left, top, right, bottom, color,
                                        0.3f);

    std::string type_text;
    if (polygon.GetType() == aa::shared::PolygonType::INCLUSION) {
      type_text = "INCLUSION";
    } else if (polygon.GetType() == aa::shared::PolygonType::EXCLUSION) {
      type_text = "EXCLUSION";
    } else {
      type_text = "UNSPECIFIED";  // Something wrong if happened
    }

    std::string label = "P" + std::to_string(i + 1) + " " + type_text +
                        " (Pri:" + std::to_string(polygon.GetPriority()) + ")";

    // Draw label background and text
    int baseline = 0;
    cv::Size text_size =
        cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);

    cv::Point text_origin(left, top + text_size.height + 5);
    cv::Rect text_rect(text_origin.x, text_origin.y - text_size.height,
                       text_size.width, text_size.height + baseline);

    cv::rectangle(frame, text_rect, color, cv::FILLED);
    cv::putText(frame, label, text_origin, cv::FONT_HERSHEY_SIMPLEX, 0.5,
                cv::Scalar(0, 0, 0), 1);  // Black text
  }
}

}  // namespace aa::server
