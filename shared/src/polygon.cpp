#include "polygon.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace aa::shared {

Polygon::Polygon(std::vector<Point> vertices, PolygonType type,
                 int32_t priority, std::vector<int32_t> target_classes)
    : vertices_{std::move(vertices)},
      type_{type},
      priority_{priority},
      target_classes_{std::move(target_classes)} {}

Polygon::Polygon(const Polygon& other)
    : vertices_{other.vertices_},  // Deep copy of vector of Points
      type_{other.type_},
      priority_{other.priority_},
      target_classes_{other.target_classes_} {}  // Deep copy of vector

Polygon::Polygon(Polygon&& other) noexcept
    : vertices_{std::move(other.vertices_)},
      type_{other.type_},
      priority_{other.priority_},
      target_classes_{std::move(other.target_classes_)} {
  // Reset moved-from object to valid state
  other.type_ = PolygonType::UNSPECIFIED;
  other.priority_ = 0;
}

Polygon& Polygon::operator=(const Polygon& other) {
  if (this != &other) {           // Self-assignment check
    vertices_ = other.vertices_;  // Deep copy of vector of Points
    type_ = other.type_;
    priority_ = other.priority_;
    target_classes_ = other.target_classes_;  // Deep copy of vector
  }
  return *this;
}

Polygon& Polygon::operator=(Polygon&& other) noexcept {
  if (this != &other) {  // Self-assignment check
    vertices_ = std::move(other.vertices_);
    type_ = other.type_;
    priority_ = other.priority_;
    target_classes_ = std::move(other.target_classes_);

    // Reset moved-from object to valid state
    other.type_ = PolygonType::UNSPECIFIED;
    other.priority_ = 0;
  }
  return *this;
}

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

void Polygon::Scale(double scale_x, double scale_y) {
  for (auto& vertex : vertices_) {
    vertex.SetX(vertex.GetX() * scale_x);
    vertex.SetY(vertex.GetY() * scale_y);
  }
}

bool Polygon::Contains(double x, double y) const {
  int num_vertices = static_cast<int>(vertices_.size());
  if (num_vertices < 3) {
    return false;  // A polygon must have at least 3 vertices
  }

  const double EPSILON = 1e-10;

  // First check if point is exactly on a vertex or edge
  for (int i = 0; i < num_vertices; i++) {
    double xi = vertices_[i].GetX();
    double yi = vertices_[i].GetY();

    // Check if point is exactly on a vertex
    if (std::abs(x - xi) < EPSILON && std::abs(y - yi) < EPSILON) {
      return false;  // Points on vertices are considered outside
    }

    // Check if point is on an edge
    int j = (i + 1) % num_vertices;
    double xj = vertices_[j].GetX();
    double yj = vertices_[j].GetY();

    // Check if point lies on the line segment between vertices[i] and
    // vertices[j]
    if (IsPointOnLineSegment(x, y, xi, yi, xj, yj)) {
      return false;  // Points on edges are considered outside
    }
  }

  // Ray casting algorithm for interior points
  bool inside = false;
  int j = num_vertices - 1;

  for (int i = 0; i < num_vertices; i++) {
    double xi = vertices_[i].GetX();
    double yi = vertices_[i].GetY();
    double xj = vertices_[j].GetX();
    double yj = vertices_[j].GetY();

    // Check if ray crosses the edge from vertices[j] to vertices[i]
    if (((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
      inside = !inside;
    }
    j = i;
  }

  return inside;
}

bool Polygon::Contains(const Point& point) const {
  return Contains(point.GetX(), point.GetY());
}

bool Polygon::IsPointOnLineSegment(double px, double py, double x1, double y1,
                                   double x2, double y2) const {
  const double EPSILON = 1e-10;

  // Check if point is within the bounding box of the line segment
  double min_x = std::min(x1, x2);
  double max_x = std::max(x1, x2);
  double min_y = std::min(y1, y2);
  double max_y = std::max(y1, y2);

  if (px < min_x - EPSILON || px > max_x + EPSILON || py < min_y - EPSILON ||
      py > max_y + EPSILON) {
    return false;
  }

  // Check if point lies on the line using cross product
  // Vector from (x1,y1) to (x2,y2): (x2-x1, y2-y1)
  // Vector from (x1,y1) to (px,py): (px-x1, py-y1)
  // Cross product: (x2-x1)*(py-y1) - (y2-y1)*(px-x1)
  double cross_product = (x2 - x1) * (py - y1) - (y2 - y1) * (px - x1);

  return std::abs(cross_product) < EPSILON;
}

}  // namespace aa::shared
