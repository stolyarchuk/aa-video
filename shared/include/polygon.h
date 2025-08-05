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

 private:
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
