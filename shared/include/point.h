#pragma once

#include "point.pb.h"

namespace aa::shared {

/**
 * @brief C++ representation of Point protobuf message
 *
 * Represents a point in 2D space with x and y coordinates.
 */
class Point {
 public:
  /**
   * @brief Default constructor
   */
  Point() = default;

  /**
   * @brief Constructor with coordinates
   * @param x X coordinate
   * @param y Y coordinate
   */
  Point(double x, double y);

  /**
   * @brief Create Point from protobuf message
   * @param proto_point Protobuf Point message
   * @return Point instance
   */
  static Point FromProto(const ::aa::proto::Point& proto_point);

  /**
   * @brief Convert Point to protobuf message
   * @return Protobuf Point message
   */
  ::aa::proto::Point ToProto() const;

  // Getters
  double GetX() const { return x_; }
  double GetY() const { return y_; }

  // Setters
  void SetX(double x) { x_ = x; }
  void SetY(double y) { y_ = y; }

 private:
  double x_{0.0};  ///< X coordinate
  double y_{0.0};  ///< Y coordinate
};

}  // namespace aa::shared
