#include "point.h"

namespace aa::shared {

Point::Point(double x, double y) : x_{x}, y_{y} {}

Point Point::FromProto(const ::aa::proto::Point& proto_point) {
  return Point{proto_point.x(), proto_point.y()};
}

::aa::proto::Point Point::ToProto() const {
  ::aa::proto::Point proto_point;
  proto_point.set_x(x_);
  proto_point.set_y(y_);
  return proto_point;
}

}  // namespace aa::shared
