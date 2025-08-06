#include "frame.h"

#include <utility>

namespace aa::shared {

Frame::Frame(int32_t rows, int32_t cols, int32_t elm_type, int32_t elm_size,
             std::vector<uint8_t> data)
    : rows_{rows},
      cols_{cols},
      elm_type_{elm_type},
      elm_size_{elm_size},
      data_{std::move(data)} {}

Frame::Frame(const Frame& other)
    : rows_{other.rows_},
      cols_{other.cols_},
      elm_type_{other.elm_type_},
      elm_size_{other.elm_size_},
      data_{other.data_} {}  // Deep copy of vector

Frame::Frame(Frame&& other) noexcept
    : rows_{other.rows_},
      cols_{other.cols_},
      elm_type_{other.elm_type_},
      elm_size_{other.elm_size_},
      data_{std::move(other.data_)} {
  // Reset moved-from object to valid state
  other.rows_ = 0;
  other.cols_ = 0;
  other.elm_type_ = 0;
  other.elm_size_ = 0;
}

Frame& Frame::operator=(const Frame& other) {
  if (this != &other) {  // Self-assignment check
    rows_ = other.rows_;
    cols_ = other.cols_;
    elm_type_ = other.elm_type_;
    elm_size_ = other.elm_size_;
    data_ = other.data_;  // Deep copy of vector
  }
  return *this;
}

Frame& Frame::operator=(Frame&& other) noexcept {
  if (this != &other) {  // Self-assignment check
    rows_ = other.rows_;
    cols_ = other.cols_;
    elm_type_ = other.elm_type_;
    elm_size_ = other.elm_size_;
    data_ = std::move(other.data_);

    // Reset moved-from object to valid state
    other.rows_ = 0;
    other.cols_ = 0;
    other.elm_type_ = 0;
    other.elm_size_ = 0;
  }
  return *this;
}

Frame::Frame(const cv::Mat& mat)
    : rows_{mat.rows},
      cols_{mat.cols},
      elm_type_{mat.type()},
      elm_size_{static_cast<int32_t>(mat.elemSize())} {
  size_t data_size = mat.total() * mat.elemSize();
  data_.resize(data_size);
  std::memcpy(data_.data(), mat.data, data_size);
}

Frame Frame::FromProto(const ::aa::proto::Frame& proto_frame) {
  std::vector<uint8_t> data;
  const std::string& proto_data = proto_frame.data();
  data.reserve(proto_data.size());
  data.assign(proto_data.begin(), proto_data.end());

  return Frame{proto_frame.rows(), proto_frame.cols(), proto_frame.elm_type(),
               proto_frame.elm_size(), std::move(data)};
}

::aa::proto::Frame Frame::ToProto() const {
  ::aa::proto::Frame proto_frame;
  proto_frame.set_rows(rows_);
  proto_frame.set_cols(cols_);
  proto_frame.set_elm_type(elm_type_);
  proto_frame.set_elm_size(elm_size_);
  proto_frame.set_data(data_.data(), data_.size());
  return proto_frame;
}

cv::Mat Frame::ToMat() const {
  cv::Mat mat{rows_, cols_, elm_type_};
  std::memcpy(mat.data, data_.data(), data_.size());
  return mat;
}

}  // namespace aa::shared
