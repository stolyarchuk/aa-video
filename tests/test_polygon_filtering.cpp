#include <gtest/gtest.h>

#include <opencv2/opencv.hpp>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "point.h"
#include "polygon.h"

namespace aa::shared {

// Test the core polygon containment logic that's used by the detector server
TEST(PolygonFilteringCoreTest, PolygonContainsAccuracyTest) {
  // Test the polygon Contains method with OpenCV's pointPolygonTest as
  // reference
  std::vector<aa::shared::Point> vertices = {
      aa::shared::Point{100.0, 100.0}, aa::shared::Point{300.0, 150.0},
      aa::shared::Point{250.0, 300.0}, aa::shared::Point{50.0, 250.0}};

  aa::shared::Polygon test_polygon(vertices, aa::shared::PolygonType::INCLUSION,
                                   1, {});

  // Convert to OpenCV format for comparison
  std::vector<cv::Point2f> cv_vertices;
  for (const auto& vertex : vertices) {
    cv_vertices.emplace_back(vertex.GetX(), vertex.GetY());
  }

  // Test multiple points and compare results
  std::vector<std::pair<double, double>> test_points = {
      {175.0, 200.0},  // Should be inside
      {50.0, 50.0},    // Should be outside
      {350.0, 200.0},  // Should be outside
      {150.0, 175.0},  // Should be inside
      {100.0, 300.0},  // Should be outside
      {200.0, 225.0}   // Should be inside
  };

  for (const auto& [x, y] : test_points) {
    bool our_result = test_polygon.Contains(x, y);
    double cv_result =
        cv::pointPolygonTest(cv_vertices, cv::Point2f(x, y), false);
    bool cv_inside = cv_result > 0;

    // Our implementation considers points on edges as outside, OpenCV considers
    // them inside So we only compare for points that are clearly inside or
    // outside
    if (std::abs(cv_result) > 1e-6) {  // Not on edge
      EXPECT_EQ(our_result, cv_inside)
          << "Point (" << x << ", " << y << ") - Our: " << our_result
          << ", OpenCV: " << cv_inside << " (distance: " << cv_result << ")";
    }
  }
}

// Test polygon filtering logic behavior
TEST(PolygonFilteringCoreTest, InclusionPolygonLogic) {
  // Create an inclusion polygon - square area
  std::vector<aa::shared::Point> vertices = {
      aa::shared::Point{100.0, 100.0}, aa::shared::Point{300.0, 100.0},
      aa::shared::Point{300.0, 300.0}, aa::shared::Point{100.0, 300.0}};

  aa::shared::Polygon inclusion_polygon(
      vertices, aa::shared::PolygonType::INCLUSION, 1,
      std::vector<int32_t>{0, 1});  // Person and bicycle classes

  // Test points inside the polygon
  EXPECT_TRUE(inclusion_polygon.Contains(200.0, 200.0));  // Center
  EXPECT_TRUE(inclusion_polygon.Contains(150.0, 150.0));  // Inside
  EXPECT_TRUE(inclusion_polygon.Contains(250.0, 250.0));  // Inside

  // Test points outside the polygon
  EXPECT_FALSE(inclusion_polygon.Contains(50.0, 50.0));  // Outside left-top
  EXPECT_FALSE(
      inclusion_polygon.Contains(350.0, 350.0));  // Outside right-bottom
  EXPECT_FALSE(inclusion_polygon.Contains(200.0, 50.0));   // Outside top
  EXPECT_FALSE(inclusion_polygon.Contains(200.0, 350.0));  // Outside bottom

  // Test polygon properties
  EXPECT_EQ(inclusion_polygon.GetType(), aa::shared::PolygonType::INCLUSION);
  EXPECT_EQ(inclusion_polygon.GetPriority(), 1);

  const auto& target_classes = inclusion_polygon.GetTargetClasses();
  EXPECT_EQ(target_classes.size(), 2);
  EXPECT_TRUE(std::find(target_classes.begin(), target_classes.end(), 0) !=
              target_classes.end());
  EXPECT_TRUE(std::find(target_classes.begin(), target_classes.end(), 1) !=
              target_classes.end());
}

TEST(PolygonFilteringCoreTest, ExclusionPolygonLogic) {
  // Create an exclusion polygon - triangular area
  std::vector<aa::shared::Point> vertices = {aa::shared::Point{0.0, 0.0},
                                             aa::shared::Point{200.0, 0.0},
                                             aa::shared::Point{100.0, 150.0}};

  aa::shared::Polygon exclusion_polygon(
      vertices, aa::shared::PolygonType::EXCLUSION, 2, {});

  // Test points inside the triangle
  EXPECT_TRUE(exclusion_polygon.Contains(100.0, 50.0));  // Center area
  EXPECT_TRUE(exclusion_polygon.Contains(75.0, 37.5));   // Inside left
  EXPECT_TRUE(exclusion_polygon.Contains(125.0, 37.5));  // Inside right

  // Test points outside the triangle
  EXPECT_FALSE(exclusion_polygon.Contains(-10.0, 50.0));   // Left of triangle
  EXPECT_FALSE(exclusion_polygon.Contains(210.0, 50.0));   // Right of triangle
  EXPECT_FALSE(exclusion_polygon.Contains(100.0, 200.0));  // Above triangle
  EXPECT_FALSE(exclusion_polygon.Contains(100.0, -10.0));  // Below triangle

  // Test polygon properties
  EXPECT_EQ(exclusion_polygon.GetType(), aa::shared::PolygonType::EXCLUSION);
  EXPECT_EQ(exclusion_polygon.GetPriority(), 2);
  EXPECT_TRUE(
      exclusion_polygon.GetTargetClasses().empty());  // No class restrictions
}

TEST(PolygonFilteringCoreTest, ComplexPolygonShapes) {
  // Create an L-shaped inclusion polygon
  std::vector<aa::shared::Point> vertices = {
      aa::shared::Point{0.0, 0.0},     // Bottom-left
      aa::shared::Point{150.0, 0.0},   // Bottom-middle
      aa::shared::Point{150.0, 50.0},  // Inner corner 1
      aa::shared::Point{50.0, 50.0},   // Inner corner 2
      aa::shared::Point{50.0, 100.0},  // Top-left of vertical part
      aa::shared::Point{0.0, 100.0}    // Top-left
  };

  aa::shared::Polygon l_polygon(vertices, aa::shared::PolygonType::INCLUSION, 1,
                                {});

  // Test points in the horizontal part of the L
  EXPECT_TRUE(l_polygon.Contains(75.0, 25.0));  // Middle of horizontal part
  EXPECT_TRUE(
      l_polygon.Contains(125.0, 25.0));  // Right side of horizontal part

  // Test points in the vertical part of the L
  EXPECT_TRUE(l_polygon.Contains(25.0, 75.0));  // Middle of vertical part

  // Test points in the "notch" of the L (should be outside)
  EXPECT_FALSE(l_polygon.Contains(100.0, 75.0));  // In the notch area
  EXPECT_FALSE(l_polygon.Contains(125.0, 80.0));  // In the notch area

  // Test points clearly outside
  EXPECT_FALSE(l_polygon.Contains(-10.0, 50.0));  // Left of polygon
  EXPECT_FALSE(l_polygon.Contains(200.0, 50.0));  // Right of polygon
  EXPECT_FALSE(l_polygon.Contains(25.0, 150.0));  // Above polygon
  EXPECT_FALSE(l_polygon.Contains(75.0, -10.0));  // Below polygon
}

// Test performance comparison with OpenCV
TEST(PolygonFilteringCoreTest, PerformanceComparison) {
  // Create a complex polygon with many vertices
  std::vector<aa::shared::Point> vertices;
  const int num_vertices = 20;
  const double radius = 100.0;
  const double center_x = 200.0, center_y = 200.0;

  // Create a rough circle with many vertices
  for (int i = 0; i < num_vertices; ++i) {
    double angle = 2.0 * M_PI * i / num_vertices;
    double x = center_x + radius * std::cos(angle);
    double y = center_y + radius * std::sin(angle);
    vertices.emplace_back(x, y);
  }

  aa::shared::Polygon complex_polygon(
      vertices, aa::shared::PolygonType::INCLUSION, 1, {});

  // Convert to OpenCV format
  std::vector<cv::Point2f> cv_vertices;
  for (const auto& vertex : vertices) {
    cv_vertices.emplace_back(vertex.GetX(), vertex.GetY());
  }

  // Test many points for performance and accuracy comparison
  const int num_test_points = 1000;
  int our_matches = 0, cv_matches = 0, agreement_count = 0;

  auto start_our = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < num_test_points; ++i) {
    double x = 100.0 + (i * 200.0) / num_test_points;  // X from 100 to 300
    double y = 100.0 + ((i * 7) % 200);  // Y from 100 to 300 (pseudo-random)

    bool our_result = complex_polygon.Contains(x, y);
    if (our_result) our_matches++;
  }

  auto end_our = std::chrono::high_resolution_clock::now();
  auto our_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_our - start_our);

  auto start_cv = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < num_test_points; ++i) {
    double x = 100.0 + (i * 200.0) / num_test_points;
    double y = 100.0 + ((i * 7) % 200);

    double cv_result =
        cv::pointPolygonTest(cv_vertices, cv::Point2f(x, y), false);
    bool cv_inside = cv_result > 0;

    if (cv_inside) cv_matches++;

    // Check agreement (only for points not on edges)
    if (std::abs(cv_result) > 1e-6) {
      bool our_result = complex_polygon.Contains(x, y);
      if (our_result == cv_inside) agreement_count++;
    }
  }

  auto end_cv = std::chrono::high_resolution_clock::now();
  auto cv_duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end_cv - start_cv);

  // Results should be reasonably similar (allowing for edge case differences)
  double match_ratio = std::abs(our_matches - cv_matches) /
                       static_cast<double>(std::max(our_matches, cv_matches));
  EXPECT_LT(match_ratio, 0.1)
      << "Our algorithm and OpenCV should give similar results";

  // Print performance comparison (for information)
  std::cout << "Performance comparison for " << num_test_points << " points on "
            << num_vertices << "-vertex polygon:\n";
  std::cout << "Our implementation: " << our_duration.count()
            << " microseconds (" << our_matches << " matches)\n";
  std::cout << "OpenCV implementation: " << cv_duration.count()
            << " microseconds (" << cv_matches << " matches)\n";
  std::cout << "Agreement rate: " << (100.0 * agreement_count / num_test_points)
            << "%\n";
}

}  // namespace aa::shared
