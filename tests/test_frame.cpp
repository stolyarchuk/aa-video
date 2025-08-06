#include <gtest/gtest.h>

#include <vector>

#include "frame.h"

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

}  // namespace aa::shared
