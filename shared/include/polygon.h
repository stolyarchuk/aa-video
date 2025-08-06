#pragma once

#include <vector>
#include <cstdint>

#include "point.h"
#include "polygon.pb.h"

namespace aa::shared {

/**
 * @brief Polygon zone type enumeration
 */
enum class PolygonType {
  UNSPECIFIED = 0,  ///< Unspecified polygon type
  INCLUSION = 1,    ///< Objects inside this zone will be processed
  EXCLUSION = 2     ///< Objects inside this zone will be ignored
};

/**
 * @brief C++ representation of Polygon protobuf message
 *
 * Represents polygon definition for detection zones.
 */
class Polygon {
 public:
  /**
   * @brief Default constructor
   */
  Polygon() = default;

  /**
   * @brief Copy constructor - performs deep copy
   * @param other Polygon to copy from
   */
  Polygon(const Polygon& other);

  /**
   * @brief Move constructor
   * @param other Polygon to move from
   */
  Polygon(Polygon&& other) noexcept;

  /**
   * @brief Copy assignment operator - performs deep copy
   * @param other Polygon to copy from
   * @return Reference to this Polygon
   */
  Polygon& operator=(const Polygon& other);

  /**
   * @brief Move assignment operator
   * @param other Polygon to move from
   * @return Reference to this Polygon
   */
  Polygon& operator=(Polygon&& other) noexcept;

  /**
   * @brief Constructor with polygon parameters
   * @param vertices List of points defining the polygon boundary
   * @param type Type of polygon (inclusion or exclusion)
   * @param priority Processing priority for objects within this polygon
   * @param target_classes List of target object classes to detect in this
   * polygon
   */
  Polygon(std::vector<Point> vertices, PolygonType type, int32_t priority,
          std::vector<int32_t> target_classes);

  /**
   * @brief Create Polygon from protobuf message
   * @param proto_polygon Protobuf Polygon message
   * @return Polygon instance
   */
  static Polygon FromProto(const ::aa::proto::Polygon& proto_polygon);

  /**
   * @brief Convert Polygon to protobuf message
   * @return Protobuf Polygon message
   */
  ::aa::proto::Polygon ToProto() const;

  // Getters
  const std::vector<Point>& GetVertices() const { return vertices_; }
  PolygonType GetType() const { return type_; }
  int32_t GetPriority() const { return priority_; }
  const std::vector<int32_t>& GetTargetClasses() const {
    return target_classes_;
  }

  // Setters
  void SetVertices(std::vector<Point> vertices) {
    vertices_ = std::move(vertices);
  }
  void SetType(PolygonType type) { type_ = type; }
  void SetPriority(int32_t priority) { priority_ = priority; }
  void SetTargetClasses(std::vector<int32_t> target_classes) {
    target_classes_ = std::move(target_classes);
  }

  /**
   * @brief Scale polygon vertices by given factors
   * @param scale_x Scaling factor for X coordinates
   * @param scale_y Scaling factor for Y coordinates
   */
  void Scale(double scale_x, double scale_y);

  /**
   * @brief Check if a point is inside the polygon using ray casting algorithm
   * @param x X coordinate of the point to test
   * @param y Y coordinate of the point to test
   * @return true if the point is inside the polygon, false otherwise
   */
  bool Contains(double x, double y) const;

  /**
   * @brief Check if a point is inside the polygon using ray casting algorithm
   * @param point Point to test for containment
   * @return true if the point is inside the polygon, false otherwise
   */
  bool Contains(const Point& point) const;

 private:
  /**
   * @brief Helper method to check if a point lies on a line segment
   * @param px X coordinate of point to test
   * @param py Y coordinate of point to test
   * @param x1 X coordinate of line segment start
   * @param y1 Y coordinate of line segment start
   * @param x2 X coordinate of line segment end
   * @param y2 Y coordinate of line segment end
   * @return true if point lies on the line segment, false otherwise
   */
  bool IsPointOnLineSegment(double px, double py, double x1, double y1,
                            double x2, double y2) const;

  std::vector<Point>
      vertices_;  ///< List of points defining the polygon boundary
  PolygonType type_{
      PolygonType::UNSPECIFIED};  ///< Type of polygon (inclusion or exclusion)
  int32_t priority_{
      0};  ///< Processing priority for objects within this polygon
  std::vector<int32_t> target_classes_;  ///< List of target object classes to
                                         ///< detect in this polygon
};

}  // namespace aa::shared
