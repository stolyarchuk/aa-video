#include <gtest/gtest.h>

#include <vector>

#include "point.h"
#include "polygon.h"

namespace aa::shared {

// Test Polygon copy/move semantics
TEST(PolygonCopyMoveTest, CopyConstructor) {
  std::vector<Point> vertices = {Point{1.0f, 2.0f}, Point{3.0f, 4.0f}};
  std::vector<int32_t> target_classes = {1, 2, 3};
  Polygon original{vertices, PolygonType::INCLUSION, 5, target_classes};

  Polygon copied{original};  // Copy constructor

  // Verify deep copy
  EXPECT_EQ(copied.GetVertices().size(), original.GetVertices().size());
  EXPECT_EQ(copied.GetType(), original.GetType());
  EXPECT_EQ(copied.GetPriority(), original.GetPriority());
  EXPECT_EQ(copied.GetTargetClasses(), original.GetTargetClasses());

  // Verify it's a deep copy by modifying original
  original.SetPriority(999);
  EXPECT_NE(copied.GetPriority(), original.GetPriority());
  EXPECT_EQ(copied.GetPriority(), 5);
}

TEST(PolygonCopyMoveTest, CopyAssignment) {
  std::vector<Point> vertices = {Point{1.0f, 2.0f}, Point{3.0f, 4.0f}};
  std::vector<int32_t> target_classes = {1, 2, 3};
  Polygon original{vertices, PolygonType::INCLUSION, 5, target_classes};
  Polygon assigned;

  assigned = original;  // Copy assignment

  // Verify deep copy
  EXPECT_EQ(assigned.GetVertices().size(), original.GetVertices().size());
  EXPECT_EQ(assigned.GetType(), original.GetType());
  EXPECT_EQ(assigned.GetPriority(), original.GetPriority());
  EXPECT_EQ(assigned.GetTargetClasses(), original.GetTargetClasses());
}

TEST(PolygonCopyMoveTest, MoveConstructor) {
  std::vector<Point> vertices = {Point{1.0f, 2.0f}, Point{3.0f, 4.0f}};
  std::vector<int32_t> target_classes = {1, 2, 3};
  Polygon original{std::move(vertices), PolygonType::INCLUSION, 5,
                   std::move(target_classes)};

  Polygon moved{std::move(original)};  // Move constructor

  // Verify moved object has the data
  EXPECT_EQ(moved.GetVertices().size(), 2);
  EXPECT_EQ(moved.GetType(), PolygonType::INCLUSION);
  EXPECT_EQ(moved.GetPriority(), 5);
  EXPECT_EQ(moved.GetTargetClasses().size(), 3);

  // Verify moved-from object is in valid state
  EXPECT_TRUE(original.GetVertices().empty());
  EXPECT_EQ(original.GetType(), PolygonType::UNSPECIFIED);
  EXPECT_EQ(original.GetPriority(), 0);
  EXPECT_TRUE(original.GetTargetClasses().empty());
}

TEST(PolygonCopyMoveTest, MoveAssignment) {
  std::vector<Point> vertices = {Point{1.0f, 2.0f}, Point{3.0f, 4.0f}};
  std::vector<int32_t> target_classes = {1, 2, 3};
  Polygon original{std::move(vertices), PolygonType::INCLUSION, 5,
                   std::move(target_classes)};
  Polygon assigned;

  assigned = std::move(original);  // Move assignment

  // Verify moved object has the data
  EXPECT_EQ(assigned.GetVertices().size(), 2);
  EXPECT_EQ(assigned.GetType(), PolygonType::INCLUSION);
  EXPECT_EQ(assigned.GetPriority(), 5);
  EXPECT_EQ(assigned.GetTargetClasses().size(), 3);

  // Verify moved-from object is in valid state
  EXPECT_TRUE(original.GetVertices().empty());
  EXPECT_EQ(original.GetType(), PolygonType::UNSPECIFIED);
  EXPECT_EQ(original.GetPriority(), 0);
  EXPECT_TRUE(original.GetTargetClasses().empty());
}

// Test self-assignment
TEST(PolygonCopyMoveTest, SelfAssignment) {
  std::vector<Point> vertices = {Point{1.0f, 2.0f}};
  std::vector<int32_t> target_classes = {1, 2};
  Polygon polygon{vertices, PolygonType::INCLUSION, 5, target_classes};

  polygon = polygon;  // Self-assignment

  // Should remain unchanged
  EXPECT_EQ(polygon.GetVertices().size(), 1);
  EXPECT_EQ(polygon.GetType(), PolygonType::INCLUSION);
  EXPECT_EQ(polygon.GetPriority(), 5);
  EXPECT_EQ(polygon.GetTargetClasses().size(), 2);
}

TEST(PolygonCopyMoveTest, DeepCopyDataIndependence) {
  std::vector<Point> original_vertices = {Point{1.0f, 2.0f}, Point{3.0f, 4.0f},
                                          Point{5.0f, 6.0f}};
  std::vector<int32_t> original_classes = {1, 2, 3, 4};
  Polygon original{original_vertices, PolygonType::INCLUSION, 10,
                   original_classes};

  Polygon copied{original};  // Copy constructor

  // Verify different memory addresses for vectors (deep copy)
  EXPECT_NE(original.GetVertices().data(), copied.GetVertices().data());
  EXPECT_NE(original.GetTargetClasses().data(),
            copied.GetTargetClasses().data());

  // Verify initial data is the same
  EXPECT_EQ(original.GetVertices().size(), copied.GetVertices().size());
  EXPECT_EQ(original.GetTargetClasses(), copied.GetTargetClasses());

  // Modify original polygon's data
  std::vector<Point> modified_vertices = {Point{10.0f, 20.0f}};
  std::vector<int32_t> modified_classes = {99, 88};
  original.SetVertices(modified_vertices);
  original.SetTargetClasses(modified_classes);
  original.SetPriority(999);

  // Verify copied polygon's data is unchanged (true independence)
  EXPECT_NE(original.GetVertices().size(), copied.GetVertices().size());
  EXPECT_NE(original.GetTargetClasses(), copied.GetTargetClasses());
  EXPECT_NE(original.GetPriority(), copied.GetPriority());

  // Verify copied polygon retains original values
  EXPECT_EQ(copied.GetVertices().size(), 3);
  EXPECT_EQ(copied.GetTargetClasses(), original_classes);
  EXPECT_EQ(copied.GetPriority(), 10);
  EXPECT_EQ(copied.GetVertices()[0].GetX(), 1.0f);
  EXPECT_EQ(copied.GetVertices()[2].GetY(), 6.0f);
}

TEST(PolygonCopyMoveTest, DeepCopyAssignmentDataIndependence) {
  std::vector<Point> original_vertices = {Point{7.0f, 8.0f},
                                          Point{9.0f, 10.0f}};
  std::vector<int32_t> original_classes = {5, 6, 7};
  Polygon original{original_vertices, PolygonType::EXCLUSION, 15,
                   original_classes};
  Polygon assigned;

  assigned = original;  // Copy assignment

  // Verify different memory addresses for vectors (deep copy)
  EXPECT_NE(original.GetVertices().data(), assigned.GetVertices().data());
  EXPECT_NE(original.GetTargetClasses().data(),
            assigned.GetTargetClasses().data());

  // Verify initial data is the same
  EXPECT_EQ(original.GetVertices().size(), assigned.GetVertices().size());
  EXPECT_EQ(original.GetTargetClasses(), assigned.GetTargetClasses());

  // Modify original by adding more vertices and classes
  std::vector<Point> current_vertices = original.GetVertices();
  current_vertices.push_back(Point{100.0f, 200.0f});
  original.SetVertices(current_vertices);

  std::vector<int32_t> current_classes = original.GetTargetClasses();
  current_classes.push_back(999);
  original.SetTargetClasses(current_classes);

  // Verify assigned polygon's data is unchanged
  EXPECT_NE(original.GetVertices().size(), assigned.GetVertices().size());
  EXPECT_NE(original.GetTargetClasses(), assigned.GetTargetClasses());

  // Verify assigned polygon retains original values
  EXPECT_EQ(assigned.GetVertices().size(), 2);
  EXPECT_EQ(assigned.GetTargetClasses(), original_classes);
  EXPECT_EQ(assigned.GetType(), PolygonType::EXCLUSION);
  EXPECT_EQ(assigned.GetPriority(), 15);
}

// Test Scale functionality
TEST(PolygonScaleTest, BasicScaling) {
  std::vector<Point> vertices = {Point{1.0, 2.0}, Point{3.0, 4.0},
                                 Point{5.0, 6.0}};
  std::vector<int32_t> target_classes = {1, 2, 3};
  Polygon polygon{vertices, PolygonType::INCLUSION, 10, target_classes};

  // Scale by 2x horizontally and 3x vertically
  polygon.Scale(2.0, 3.0);

  // Verify vertices are scaled correctly
  const auto& scaled_vertices = polygon.GetVertices();
  EXPECT_EQ(scaled_vertices.size(), 3);
  EXPECT_DOUBLE_EQ(scaled_vertices[0].GetX(), 2.0);   // 1.0 * 2.0
  EXPECT_DOUBLE_EQ(scaled_vertices[0].GetY(), 6.0);   // 2.0 * 3.0
  EXPECT_DOUBLE_EQ(scaled_vertices[1].GetX(), 6.0);   // 3.0 * 2.0
  EXPECT_DOUBLE_EQ(scaled_vertices[1].GetY(), 12.0);  // 4.0 * 3.0
  EXPECT_DOUBLE_EQ(scaled_vertices[2].GetX(), 10.0);  // 5.0 * 2.0
  EXPECT_DOUBLE_EQ(scaled_vertices[2].GetY(), 18.0);  // 6.0 * 3.0

  // Verify other members are unchanged
  EXPECT_EQ(polygon.GetType(), PolygonType::INCLUSION);
  EXPECT_EQ(polygon.GetPriority(), 10);
  EXPECT_EQ(polygon.GetTargetClasses(), target_classes);
}

TEST(PolygonScaleTest, UniformScaling) {
  std::vector<Point> vertices = {Point{10.0, 20.0}, Point{30.0, 40.0}};
  std::vector<int32_t> target_classes = {5, 6};
  Polygon polygon{vertices, PolygonType::EXCLUSION, 7, target_classes};

  // Uniform scaling by 0.5
  polygon.Scale(0.5, 0.5);

  // Verify vertices are scaled correctly
  const auto& scaled_vertices = polygon.GetVertices();
  EXPECT_EQ(scaled_vertices.size(), 2);
  EXPECT_DOUBLE_EQ(scaled_vertices[0].GetX(), 5.0);   // 10.0 * 0.5
  EXPECT_DOUBLE_EQ(scaled_vertices[0].GetY(), 10.0);  // 20.0 * 0.5
  EXPECT_DOUBLE_EQ(scaled_vertices[1].GetX(), 15.0);  // 30.0 * 0.5
  EXPECT_DOUBLE_EQ(scaled_vertices[1].GetY(), 20.0);  // 40.0 * 0.5

  // Verify other members are unchanged
  EXPECT_EQ(polygon.GetType(), PolygonType::EXCLUSION);
  EXPECT_EQ(polygon.GetPriority(), 7);
  EXPECT_EQ(polygon.GetTargetClasses(), target_classes);
}

TEST(PolygonScaleTest, NegativeScaling) {
  std::vector<Point> vertices = {Point{1.0, 2.0}, Point{3.0, 4.0}};
  std::vector<int32_t> target_classes = {1};
  Polygon polygon{vertices, PolygonType::INCLUSION, 1, target_classes};

  // Negative scaling (reflection)
  polygon.Scale(-1.0, -2.0);

  // Verify vertices are scaled correctly with negative factors
  const auto& scaled_vertices = polygon.GetVertices();
  EXPECT_EQ(scaled_vertices.size(), 2);
  EXPECT_DOUBLE_EQ(scaled_vertices[0].GetX(), -1.0);  // 1.0 * -1.0
  EXPECT_DOUBLE_EQ(scaled_vertices[0].GetY(), -4.0);  // 2.0 * -2.0
  EXPECT_DOUBLE_EQ(scaled_vertices[1].GetX(), -3.0);  // 3.0 * -1.0
  EXPECT_DOUBLE_EQ(scaled_vertices[1].GetY(), -8.0);  // 4.0 * -2.0

  // Verify other members are unchanged
  EXPECT_EQ(polygon.GetType(), PolygonType::INCLUSION);
  EXPECT_EQ(polygon.GetPriority(), 1);
  EXPECT_EQ(polygon.GetTargetClasses(), target_classes);
}

TEST(PolygonScaleTest, EmptyPolygon) {
  std::vector<Point> vertices;  // Empty
  std::vector<int32_t> target_classes = {1, 2};
  Polygon polygon{vertices, PolygonType::UNSPECIFIED, 0, target_classes};

  // Scale empty polygon
  polygon.Scale(5.0, 10.0);

  // Verify polygon remains empty but other members unchanged
  EXPECT_TRUE(polygon.GetVertices().empty());
  EXPECT_EQ(polygon.GetType(), PolygonType::UNSPECIFIED);
  EXPECT_EQ(polygon.GetPriority(), 0);
  EXPECT_EQ(polygon.GetTargetClasses(), target_classes);
}

// Test Polygon Contains method
TEST(PolygonContainsTest, SquarePolygon) {
  // Create a square polygon: (0,0), (4,0), (4,4), (0,4)
  std::vector<Point> vertices = {Point{0.0, 0.0}, Point{4.0, 0.0},
                                 Point{4.0, 4.0}, Point{0.0, 4.0}};
  Polygon square{vertices, PolygonType::INCLUSION, 1, {}};

  // Test points clearly inside
  EXPECT_TRUE(square.Contains(2.0, 2.0));  // Center
  EXPECT_TRUE(square.Contains(1.0, 1.0));  // Inside
  EXPECT_TRUE(square.Contains(3.0, 3.0));  // Inside

  // Test points clearly outside
  EXPECT_FALSE(square.Contains(-1.0, 2.0));  // Left of square
  EXPECT_FALSE(square.Contains(5.0, 2.0));   // Right of square
  EXPECT_FALSE(square.Contains(2.0, -1.0));  // Below square
  EXPECT_FALSE(square.Contains(2.0, 5.0));   // Above square

  // Test corner points (typically considered outside)
  EXPECT_FALSE(square.Contains(0.0, 0.0));  // Bottom-left corner
  EXPECT_FALSE(square.Contains(4.0, 0.0));  // Bottom-right corner
  EXPECT_FALSE(square.Contains(4.0, 4.0));  // Top-right corner
  EXPECT_FALSE(square.Contains(0.0, 4.0));  // Top-left corner

  // Test edge points (typically considered outside)
  EXPECT_FALSE(square.Contains(2.0, 0.0));  // Bottom edge
  EXPECT_FALSE(square.Contains(4.0, 2.0));  // Right edge
  EXPECT_FALSE(square.Contains(2.0, 4.0));  // Top edge
  EXPECT_FALSE(square.Contains(0.0, 2.0));  // Left edge
}

TEST(PolygonContainsTest, TrianglePolygon) {
  // Create a triangle: (0,0), (4,0), (2,3)
  std::vector<Point> vertices = {Point{0.0, 0.0}, Point{4.0, 0.0},
                                 Point{2.0, 3.0}};
  Polygon triangle{vertices, PolygonType::INCLUSION, 1, {}};

  // Test points inside triangle
  EXPECT_TRUE(triangle.Contains(2.0, 1.0));  // Center area
  EXPECT_TRUE(triangle.Contains(1.5, 0.5));  // Inside
  EXPECT_TRUE(triangle.Contains(2.5, 0.5));  // Inside

  // Test points outside triangle
  EXPECT_FALSE(triangle.Contains(-1.0, 1.0));  // Left of triangle
  EXPECT_FALSE(triangle.Contains(5.0, 1.0));   // Right of triangle
  EXPECT_FALSE(triangle.Contains(2.0, 4.0));   // Above triangle
  EXPECT_FALSE(triangle.Contains(0.0, 2.0));   // Outside left side
  EXPECT_FALSE(triangle.Contains(4.0, 2.0));   // Outside right side
}

TEST(PolygonContainsTest, ContainsPointObject) {
  // Create a simple square
  std::vector<Point> vertices = {Point{0.0, 0.0}, Point{2.0, 0.0},
                                 Point{2.0, 2.0}, Point{0.0, 2.0}};
  Polygon square{vertices, PolygonType::INCLUSION, 1, {}};

  // Test Contains(Point) overload
  Point inside_point{1.0, 1.0};
  Point outside_point{3.0, 3.0};

  EXPECT_TRUE(square.Contains(inside_point));
  EXPECT_FALSE(square.Contains(outside_point));
}

TEST(PolygonContainsTest, InvalidPolygons) {
  // Test polygon with less than 3 vertices
  std::vector<Point> invalid_vertices = {Point{0.0, 0.0}, Point{1.0, 1.0}};
  Polygon invalid_polygon{invalid_vertices, PolygonType::INCLUSION, 1, {}};

  EXPECT_FALSE(invalid_polygon.Contains(0.5, 0.5));

  // Test empty polygon
  Polygon empty_polygon;
  EXPECT_FALSE(empty_polygon.Contains(0.0, 0.0));
}

TEST(PolygonContainsTest, ComplexPolygon) {
  // Create an L-shaped polygon
  std::vector<Point> vertices = {Point{0.0, 0.0}, Point{3.0, 0.0},
                                 Point{3.0, 1.0}, Point{1.0, 1.0},
                                 Point{1.0, 3.0}, Point{0.0, 3.0}};
  Polygon l_shape{vertices, PolygonType::INCLUSION, 1, {}};

  // Test points in different parts of the L
  EXPECT_TRUE(l_shape.Contains(0.5, 0.5));  // Bottom part
  EXPECT_TRUE(l_shape.Contains(2.5, 0.5));  // Bottom right part
  EXPECT_TRUE(l_shape.Contains(0.5, 2.5));  // Top left part

  // Test points in the "notch" of the L (should be outside)
  EXPECT_FALSE(l_shape.Contains(2.0, 2.0));  // In the notch area
  EXPECT_FALSE(l_shape.Contains(2.5, 1.5));  // In the notch area

  // Test points clearly outside
  EXPECT_FALSE(l_shape.Contains(-1.0, 1.0));  // Left of polygon
  EXPECT_FALSE(l_shape.Contains(4.0, 1.0));   // Right of polygon
  EXPECT_FALSE(l_shape.Contains(1.0, 4.0));   // Above polygon
}

}  // namespace aa::shared
