#include <gtest/gtest.h>

#include <vector>

#include "frame.h"
#include "point.h"
#include "polygon.h"

namespace aa::shared {

// Test Frame copy/move semantics
TEST(FrameCopyMoveTest, CopyConstructor) {
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6};
  Frame original{2, 3, CV_8UC1, 1, data};

  Frame copied{original};  // Copy constructor

  // Verify deep copy
  EXPECT_EQ(copied.GetRows(), original.GetRows());
  EXPECT_EQ(copied.GetCols(), original.GetCols());
  EXPECT_EQ(copied.GetElmType(), original.GetElmType());
  EXPECT_EQ(copied.GetElmSize(), original.GetElmSize());
  EXPECT_EQ(copied.GetData(), original.GetData());

  // Verify it's a deep copy by modifying original
  original.SetRows(999);
  EXPECT_NE(copied.GetRows(), original.GetRows());
  EXPECT_EQ(copied.GetRows(), 2);
}

TEST(FrameCopyMoveTest, CopyAssignment) {
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6};
  Frame original{2, 3, CV_8UC1, 1, data};
  Frame assigned;

  assigned = original;  // Copy assignment

  // Verify deep copy
  EXPECT_EQ(assigned.GetRows(), original.GetRows());
  EXPECT_EQ(assigned.GetCols(), original.GetCols());
  EXPECT_EQ(assigned.GetElmType(), original.GetElmType());
  EXPECT_EQ(assigned.GetElmSize(), original.GetElmSize());
  EXPECT_EQ(assigned.GetData(), original.GetData());
}

TEST(FrameCopyMoveTest, MoveConstructor) {
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6};
  Frame original{2, 3, CV_8UC1, 1, std::move(data)};

  Frame moved{std::move(original)};  // Move constructor

  // Verify moved object has the data
  EXPECT_EQ(moved.GetRows(), 2);
  EXPECT_EQ(moved.GetCols(), 3);
  EXPECT_EQ(moved.GetElmType(), CV_8UC1);
  EXPECT_EQ(moved.GetElmSize(), 1);
  EXPECT_EQ(moved.GetData().size(), 6);

  // Verify moved-from object is in valid state
  EXPECT_EQ(original.GetRows(), 0);
  EXPECT_EQ(original.GetCols(), 0);
  EXPECT_EQ(original.GetElmType(), 0);
  EXPECT_EQ(original.GetElmSize(), 0);
  EXPECT_TRUE(original.GetData().empty());
}

TEST(FrameCopyMoveTest, MoveAssignment) {
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6};
  Frame original{2, 3, CV_8UC1, 1, std::move(data)};
  Frame assigned;

  assigned = std::move(original);  // Move assignment

  // Verify moved object has the data
  EXPECT_EQ(assigned.GetRows(), 2);
  EXPECT_EQ(assigned.GetCols(), 3);
  EXPECT_EQ(assigned.GetElmType(), CV_8UC1);
  EXPECT_EQ(assigned.GetElmSize(), 1);
  EXPECT_EQ(assigned.GetData().size(), 6);

  // Verify moved-from object is in valid state
  EXPECT_EQ(original.GetRows(), 0);
  EXPECT_EQ(original.GetCols(), 0);
  EXPECT_EQ(original.GetElmType(), 0);
  EXPECT_EQ(original.GetElmSize(), 0);
  EXPECT_TRUE(original.GetData().empty());
}

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

TEST(FrameCopyMoveTest, SelfAssignment) {
  std::vector<uint8_t> data = {1, 2, 3};
  Frame frame{2, 3, CV_8UC1, 1, data};

  frame = frame;  // Self-assignment

  // Should remain unchanged
  EXPECT_EQ(frame.GetRows(), 2);
  EXPECT_EQ(frame.GetCols(), 3);
  EXPECT_EQ(frame.GetElmType(), CV_8UC1);
  EXPECT_EQ(frame.GetElmSize(), 1);
  EXPECT_EQ(frame.GetData().size(), 3);
}

// Deep copy verification tests
TEST(FrameCopyMoveTest, DeepCopyDataIndependence) {
  std::vector<uint8_t> original_data = {10, 20, 30, 40, 50};
  Frame original{2, 3, CV_8UC1, 1, original_data};

  Frame copied{original};  // Copy constructor

  // Verify different memory addresses (deep copy)
  EXPECT_NE(original.GetData().data(), copied.GetData().data());

  // Verify initial data is the same
  EXPECT_EQ(original.GetData(), copied.GetData());

  // Modify original frame's data
  std::vector<uint8_t> modified_data = {99, 88, 77, 66, 55};
  original.SetData(modified_data);

  // Verify copied frame's data is unchanged (true independence)
  EXPECT_NE(original.GetData(), copied.GetData());
  EXPECT_EQ(copied.GetData(),
            original_data);  // Should still have original values
  EXPECT_EQ(copied.GetData()[0], 10);
  EXPECT_EQ(copied.GetData()[4], 50);
  EXPECT_EQ(original.GetData()[0], 99);
  EXPECT_EQ(original.GetData()[4], 55);
}

TEST(FrameCopyMoveTest, DeepCopyAssignmentDataIndependence) {
  std::vector<uint8_t> original_data = {100, 200,
                                        250};  // Fixed: 250 instead of 300
  Frame original{1, 3, CV_8UC3, 3, original_data};
  Frame assigned;

  assigned = original;  // Copy assignment

  // Verify different memory addresses (deep copy)
  EXPECT_NE(original.GetData().data(), assigned.GetData().data());

  // Verify initial data is the same
  EXPECT_EQ(original.GetData(), assigned.GetData());

  // Create OpenCV Mat and modify original through it
  cv::Mat original_mat = original.ToMat();
  if (!original_mat.empty() && original_mat.data) {
    original_mat.at<uint8_t>(0, 0) = 255;  // Modify first pixel
  }

  // Update original frame from modified Mat
  Frame new_original{original_mat};
  original = new_original;

  // Verify assigned frame's data is unchanged
  EXPECT_NE(original.GetData(), assigned.GetData());
  EXPECT_EQ(assigned.GetData(),
            original_data);  // Should still have original values
  EXPECT_EQ(assigned.GetData()[0], 100);
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

}  // namespace aa::shared
