#pragma once

#include <vector>
#include <cstdint>

#include <opencv2/opencv.hpp>

#include "frame.pb.h"

namespace aa::shared {

/**
 * @brief C++ representation of Frame protobuf message
 *
 * Represents video frame data corresponding to OpenCV cv::Mat structure.
 */
class Frame {
 public:
  /**
   * @brief Default constructor
   */
  Frame() = default;

  /**
   * @brief Copy constructor - performs deep copy
   * @param other Frame to copy from
   */
  Frame(const Frame& other);

  /**
   * @brief Move constructor
   * @param other Frame to move from
   */
  Frame(Frame&& other) noexcept;

  /**
   * @brief Copy assignment operator - performs deep copy
   * @param other Frame to copy from
   * @return Reference to this Frame
   */
  Frame& operator=(const Frame& other);

  /**
   * @brief Move assignment operator
   * @param other Frame to move from
   * @return Reference to this Frame
   */
  Frame& operator=(Frame&& other) noexcept;

  /**
   * @brief Constructor with frame parameters
   * @param rows Number of rows (height)
   * @param cols Number of columns (width)
   * @param elm_type Element type (CV_8UC3, CV_32FC1, etc.)
   * @param elm_size Size of each element in bytes
   * @param data Raw pixel data
   */
  Frame(int32_t rows, int32_t cols, int32_t elm_type, int32_t elm_size,
        std::vector<uint8_t> data);

  /**
   * @brief Constructor from OpenCV Mat
   * @param mat OpenCV Mat to convert
   */
  explicit Frame(const cv::Mat& mat);

  /**
   * @brief Create Frame from protobuf message
   * @param proto_frame Protobuf Frame message
   * @return Frame instance
   */
  static Frame FromProto(const ::aa::proto::Frame& proto_frame);

  /**
   * @brief Convert Frame to protobuf message
   * @return Protobuf Frame message
   */
  ::aa::proto::Frame ToProto() const;

  /**
   * @brief Convert Frame to OpenCV Mat
   * @return OpenCV Mat representation
   */
  cv::Mat ToMat() const;

  // Getters
  int32_t GetRows() const { return rows_; }
  int32_t GetCols() const { return cols_; }
  int32_t GetElmType() const { return elm_type_; }
  int32_t GetElmSize() const { return elm_size_; }
  const std::vector<uint8_t>& GetData() const { return data_; }

  // Setters
  void SetRows(int32_t rows) { rows_ = rows; }
  void SetCols(int32_t cols) { cols_ = cols; }
  void SetElmType(int32_t elm_type) { elm_type_ = elm_type; }
  void SetElmSize(int32_t elm_size) { elm_size_ = elm_size; }
  void SetData(std::vector<uint8_t> data) { data_ = std::move(data); }

 private:
  int32_t rows_{0};            ///< Number of rows (height)
  int32_t cols_{0};            ///< Number of columns (width)
  int32_t elm_type_{0};        ///< Element type (CV_8UC3, CV_32FC1, etc.)
  int32_t elm_size_{0};        ///< Size of each element in bytes
  std::vector<uint8_t> data_;  ///< Raw pixel data
};

}  // namespace aa::shared
