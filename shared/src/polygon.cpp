#include "polygon.h"

#include <utility>

namespace aa::shared {

Polygon::Polygon(std::vector<Point> vertices, PolygonType type,
                 int32_t priority, std::vector<int32_t> target_classes)
    : vertices_{std::move(vertices)},
      type_{type},
      priority_{priority},
      target_classes_{std::move(target_classes)} {}

Polygon Polygon::FromProto(const ::aa::proto::Polygon& proto_polygon) {
  std::vector<Point> vertices;
  vertices.reserve(proto_polygon.vertices_size());
  for (const auto& proto_point : proto_polygon.vertices()) {
    vertices.push_back(Point::FromProto(proto_point));
  }

  PolygonType type = static_cast<PolygonType>(proto_polygon.type());

  std::vector<int32_t> target_classes;
  target_classes.reserve(proto_polygon.target_classes_size());
  for (int32_t target_class : proto_polygon.target_classes()) {
    target_classes.push_back(target_class);
  }

  return Polygon{std::move(vertices), type, proto_polygon.priority(),
                 std::move(target_classes)};
}

::aa::proto::Polygon Polygon::ToProto() const {
  ::aa::proto::Polygon proto_polygon;

  // Set vertices
  for (const auto& vertex : vertices_) {
    *proto_polygon.add_vertices() = vertex.ToProto();
  }

  // Set type
  proto_polygon.set_type(
      static_cast<::aa::proto::PolygonType>(static_cast<int>(type_)));

  // Set priority
  proto_polygon.set_priority(priority_);

  // Set target classes
  for (int32_t target_class : target_classes_) {
    proto_polygon.add_target_classes(target_class);
  }

  return proto_polygon;
}

}  // namespace aa::shared
