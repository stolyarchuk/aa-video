#pragma once

#include <vector>

#include "polygon.h"
#include "types.h"

namespace aa::server {

class PolygonFilter {
 public:
  PolygonFilter();
  ~PolygonFilter();

  /**
   * @brief Draw filled bounding boxes for polygons
   *
   * @param frame Image frame to draw on
   * @param polygons Vector of polygons to visualize
   */
  void DrawPolygonBoundingBoxes(cv::Mat& frame) const;

  /**
   * @brief Filter detections based on polygon rules
   *
   * @param detections Input detections to filter
   * @param polygons Polygon zones with inclusion/exclusion rules
   * @return std::vector<Detection> Filtered detections
   */
  std::vector<aa::shared::Detection> FilterDetectionsByPolygons(
      const std::vector<aa::shared::Detection>& detections);

  void SetPolygons(std::vector<aa::shared::Polygon>&& polygons);

 private:
  std::vector<aa::shared::Polygon> polygons_;

  std::pair<double, double> GetDetectionCenter(
      const aa::shared::Detection& detection);

  std::vector<const aa::shared::Polygon*> FindContainingPolygons(
      double center_x, double center_y);

  bool ShouldIncludeDetection(
      const aa::shared::Detection& detection,
      const std::vector<const aa::shared::Polygon*>& containing_polygons);

  bool IsDetectionClassAllowed(const aa::shared::Detection& detection,
                               const aa::shared::Polygon& polygon);
};

}  // namespace aa::server
