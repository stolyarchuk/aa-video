#include "detector_server.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include "frame.h"
#include "logging.h"
#include "polygon.h"

namespace aa::server {

DetectorServer::DetectorServer(aa::shared::Options options)
    : options_{std::move(options)} {
  service_ = std::make_unique<DetectorServiceImpl>(
      options_.Get<std::string>("address"));
}

void DetectorServer::Initialize() {
  // Initialize neural network first
  InitializeNetwork();

  service_->Register<DetectorServiceMethods::kCheckHealth>(
      [this](auto request, auto response) {
        return CheckHealth(request, response);
      });
  service_->Register<DetectorServiceMethods::kProcessFrame>(
      [this](auto request, auto response) {
        return ProcessFrame(request, response);
      });
}

void DetectorServer::Start() {
  service_->Build();
  service_->Wait();
}

void DetectorServer::Shutdown() { service_->Stop(); }

grpc::Status DetectorServer::CheckHealth(
    const aa::proto::CheckHealthRequest*,
    aa::proto::CheckHealthResponse*) const {
  // For now, we assume the server is always healthy
  AA_LOG_INFO("Health check passed");

  return grpc::Status::OK;
}

void DetectorServer::InitializeNetwork() {
  // Load neural network model
  if (!LoadModel()) {
    throw std::runtime_error("Failed to initialize model " +
                             options_.Get<cv::String>("model"));
  }

  // Set DNN backend and target after loading model
  dnn_network_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
  dnn_network_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

bool DetectorServer::LoadModel() {
  try {
    auto model_path = options_.Get<std::string>("model");
    AA_LOG_INFO("Loading model from: " << model_path);

    // Load model using OpenCV DNN
    dnn_network_ = cv::dnn::readNetFromONNX(model_path);

    if (dnn_network_.empty()) {
      AA_LOG_ERROR("Failed to load model: network is empty");
      return false;
    }

    // Get and log network information
    try {
      auto layer_names = dnn_network_.getLayerNames();
      auto unconnected_layers = dnn_network_.getUnconnectedOutLayers();

      AA_LOG_INFO("Model loaded successfully");
      AA_LOG_INFO("Model has " << layer_names.size() << " layers");
      AA_LOG_INFO("Model has " << unconnected_layers.size()
                               << " output layers");

      // Log output layer names
      std::vector<cv::String> output_names =
          dnn_network_.getUnconnectedOutLayersNames();
      for (const auto& name : output_names) {
        AA_LOG_INFO("Output layer: " << name);
      }

    } catch (const cv::Exception& info_error) {
      AA_LOG_WARNING(
          "Could not analyze model structure: " << info_error.what());
    }

    AA_LOG_INFO("Model loaded successfully and ready for object detection");
    return true;

  } catch (const cv::Exception& e) {
    // Log OpenCV-specific errors
    AA_LOG_ERROR("OpenCV error loading model: " << e.what());
    return false;
  } catch (const std::exception& e) {
    // Log general errors
    AA_LOG_ERROR("Error loading model: " << e.what());
    return false;
  }
}

cv::Mat DetectorServer::PreprocessFrame(const cv::Mat& frame) {
  // Get network input size from Options parameters
  int width = options_.Get<int>("width");
  int height = options_.Get<int>("height");
  cv::Size network_input_size(width, height);
  cv::Scalar mean_values(0, 0, 0);    // No mean subtraction
  double scale_factor = 1.0 / 255.0;  // Normalize to [0,1]
  bool swap_rb = true;                // Convert BGR to RGB
  bool crop = false;

  AA_LOG_DEBUG("Using preprocessing: " << network_input_size.width << "x"
                                       << network_input_size.height
                                       << ", RGB, scale=1/255");

  try {
    cv::Mat resized_frame;

    // Letterbox resize to maintain aspect ratio
    double scale =
        std::min(static_cast<double>(network_input_size.width) / frame.cols,
                 static_cast<double>(network_input_size.height) / frame.rows);

    int new_width = static_cast<int>(frame.cols * scale);
    int new_height = static_cast<int>(frame.rows * scale);

    cv::resize(frame, resized_frame, cv::Size(new_width, new_height), 0, 0,
               cv::INTER_LINEAR);

    // Create padded image (letterboxing)
    cv::Mat padded_frame = cv::Mat::zeros(network_input_size, CV_8UC3);
    padded_frame.setTo(cv::Scalar(114, 114, 114));  // Gray padding

    int dx = (network_input_size.width - new_width) / 2;
    int dy = (network_input_size.height - new_height) / 2;

    resized_frame.copyTo(padded_frame(cv::Rect(dx, dy, new_width, new_height)));

    // Create blob from letterboxed image
    cv::Mat blob = cv::dnn::blobFromImage(
        padded_frame,        // Letterboxed input image
        scale_factor,        // Scale factor (1/255)
        network_input_size,  // Target size (already achieved)
        mean_values,         // No mean subtraction
        swap_rb,             // BGR to RGB conversion
        crop                 // No additional cropping
    );

    // Verify blob dimensions
    if (blob.dims == 4 && blob.size[1] == 3 &&
        blob.size[2] == network_input_size.height &&
        blob.size[3] == network_input_size.width) {
      AA_LOG_DEBUG("Preprocessed blob dimensions: ["
                   << blob.size[0] << ", " << blob.size[1] << ", "
                   << blob.size[2] << ", " << blob.size[3] << "]");
      return blob;
    } else {
      AA_LOG_WARNING("Unexpected blob dimensions");
      return blob;  // Return anyway, let the network handle it
    }

  } catch (const cv::Exception& e) {
    AA_LOG_ERROR("OpenCV error in preprocessing: " << e.what());
    return cv::Mat();  // Return empty Mat on error
  }
}

cv::Mat DetectorServer::RunInference(const cv::Mat& blob) {
  try {
    // Set the input blob to the network
    try {
      dnn_network_.setInput(blob, "images");
      AA_LOG_DEBUG("Using 'images' input layer");
    } catch (const cv::Exception&) {
      // Fallback to generic input name
      dnn_network_.setInput(blob);
      AA_LOG_DEBUG("Using default input layer");
    }

    // Run forward pass through the network
    cv::Mat output = dnn_network_.forward();

    AA_LOG_DEBUG("Inference completed");

    // Log output tensor information for debugging
    if (!output.empty()) {
      std::string shape_str = "[";
      for (int i = 0; i < output.dims; ++i) {
        if (i > 0) shape_str += ", ";
        shape_str += std::to_string(output.size[i]);
      }
      shape_str += "]";
      AA_LOG_DEBUG("Network output shape: " << shape_str << " (dims: "
                                            << output.dims << ")");
    }

    return output;

  } catch (const cv::Exception& e) {
    AA_LOG_ERROR("OpenCV error during inference: " << e.what());
    return cv::Mat();  // Return empty Mat on error
  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error during inference: " << e.what());
    return cv::Mat();
  }
}

std::vector<Detection> DetectorServer::PostprocessDetections(
    const cv::Mat& network_output,
    [[maybe_unused]] const cv::Mat& original_frame,
    [[maybe_unused]] const std::vector<aa::shared::Polygon>& polygons) {
  std::vector<Detection> detections;

  try {
    if (network_output.empty()) {
      return detections;
    }

    std::vector<Detection> raw_detections = ParseNetworkOutput(network_output);
    std::vector<Detection> nms_detections =
        ApplyNonMaximumSuppression(raw_detections);
    std::vector<Detection> scaled_detections =
        ScaleDetectionsToOriginalFrame(nms_detections, original_frame);

    // Apply polygon-based filtering to detections
    detections = FilterDetectionsByPolygons(scaled_detections, polygons);

  } catch (const cv::Exception& e) {
    AA_LOG_ERROR("OpenCV error in post-processing: " << e.what());
  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error in post-processing: " << e.what());
  }

  return detections;
}

grpc::Status DetectorServer::ProcessFrame(
    const aa::proto::ProcessFrameRequest* request,
    aa::proto::ProcessFrameResponse* response) const {
  try {
    if (request->polygons_size() == 0) {
      AA_LOG_ERROR("No polygons provided in request");
      response->set_success(false);
      return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                          "No polygons provided in request");
    }

    std::vector<aa::shared::Polygon> polygons;
    polygons.reserve(request->polygons_size());

    for (int i = 0; i < request->polygons_size(); ++i) {
      auto polygon = aa::shared::Polygon::FromProto(request->polygons(i));

      // Skip polygons with unspecified type
      if (polygon.GetType() == aa::shared::PolygonType::UNSPECIFIED) {
        AA_LOG_WARNING("Skipping polygon at index "
                       << i << " with UNSPECIFIED type");
        continue;
      }

      polygons.push_back(std::move(polygon));
    }

    // Check if we have any valid polygons after filtering
    if (polygons.empty()) {
      AA_LOG_ERROR(
          "No valid polygons found after filtering out UNSPECIFIED types");
      response->set_success(false);
      return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                          "No valid polygons found");
    }

    std::sort(polygons.begin(), polygons.end(),
              [](const aa::shared::Polygon& a, const aa::shared::Polygon& b) {
                return a.GetPriority() > b.GetPriority();
              });

    auto frame = aa::shared::Frame::FromProto(request->frame());

    // Use YOLOv7 standard input size or fallback to 224x224
    auto model_path = options_.Get<std::string>("model");
    bool is_yolo_model = model_path.find("yolo") != std::string::npos ||
                         model_path.find("YOLO") != std::string::npos;

    cv::Size model_size =
        is_yolo_model ? cv::Size(640, 640) : cv::Size(224, 224);

    cv::Mat input_frame = frame.ToMat();
    cv::Size original_size = input_frame.size();

    auto scale_x = static_cast<double>(model_size.width) / original_size.width;
    auto scale_y =
        static_cast<double>(model_size.height) / original_size.height;

    std::vector<aa::shared::Polygon> scaled_polygons;
    scaled_polygons.reserve(polygons.size());

    for (auto polygon : polygons) {
      // Scale polygon coordinates to match model input size
      polygon.Scale(scale_x, scale_y);
      scaled_polygons.push_back(std::move(polygon));
    }

    cv::Mat resized_frame;
    cv::resize(input_frame, resized_frame, model_size, 0, 0, cv::INTER_LINEAR);

    // Step 1: Preprocess frame for neural network
    cv::Mat preprocessed_blob =
        const_cast<DetectorServer*>(this)->PreprocessFrame(resized_frame);

    if (preprocessed_blob.empty()) {
      AA_LOG_ERROR("Failed to preprocess frame");
      response->set_success(false);
      return grpc::Status(grpc::StatusCode::INTERNAL,
                          "Failed to preprocess frame");
    }

    // Step 2: Run inference using YOLOv7 model
    cv::Mat network_output =
        const_cast<DetectorServer*>(this)->RunInference(preprocessed_blob);

    if (network_output.empty()) {
      AA_LOG_ERROR("Failed to run inference on preprocessed blob");
      response->set_success(false);
      return grpc::Status(grpc::StatusCode::INTERNAL,
                          "Failed to run inference");
    }

    // Step 3: Post-process results to extract detections
    std::vector<Detection> detections =
        const_cast<DetectorServer*>(this)->PostprocessDetections(
            network_output, resized_frame, scaled_polygons);

    // Step 4: Draw bounding boxes on the result frame
    cv::Mat result_frame_with_detections = input_frame.clone();
    DrawBoundingBoxes(result_frame_with_detections, detections);

    // Convert the result frame with detections to protobuf format
    auto result_frame = aa::shared::Frame(result_frame_with_detections);
    auto proto_result_frame = result_frame.ToProto();

    // Set the result frame in the response
    response->mutable_result()->CopyFrom(proto_result_frame);
    response->set_success(true);

    AA_LOG_INFO("Processed frame successfully. Found " << detections.size()
                                                       << " detections.");

    return grpc::Status::OK;
  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error processing frame: " << e.what());
    return grpc::Status(grpc::StatusCode::INTERNAL, "Frame processing failed");
  }
}

std::vector<Detection> DetectorServer::ParseNetworkOutput(
    const cv::Mat& network_output) {
  std::vector<Detection> detections;

  if (network_output.empty()) {
    AA_LOG_WARNING("Empty network output");
    return detections;
  }

  try {
    // Output format can vary:
    // - 2D: [num_detections, 85]
    // - 3D: [batch_size, num_detections, 85]
    // - 4D: [batch_size, 1, num_detections, 85]
    // where 85 = 4 (bbox: x, y, w, h) + 1 (confidence) + 80 (COCO classes)

    // Validate output dimensions - need at least 2D tensor
    if (network_output.dims < 2) {
      AA_LOG_ERROR("Invalid output dimensions: " << network_output.dims
                                                 << " (expected at least 2)");
      return detections;
    }

    // Validate data pointer
    if (network_output.data == nullptr) {
      AA_LOG_ERROR("Network output data is null");
      return detections;
    }

    const float confidence_threshold =
        0.1f;  // Lower minimum confidence to catch more detections
    const float* data = reinterpret_cast<const float*>(network_output.data);

    // Handle different output tensor shapes
    int num_detections = 0;
    int detection_size = 0;

    if (network_output.dims == 2) {
      // 2D tensor: [num_detections, 85]
      num_detections = network_output.size[0];
      detection_size = network_output.size[1];
      AA_LOG_DEBUG("2D output tensor: [" << num_detections << ", "
                                         << detection_size << "]");
    } else if (network_output.dims == 3) {
      // 3D tensor: [batch_size, num_detections, 85]
      num_detections = network_output.size[1];
      detection_size = network_output.size[2];
      AA_LOG_DEBUG("3D output tensor: [" << network_output.size[0] << ", "
                                         << num_detections << ", "
                                         << detection_size << "]");
    } else if (network_output.dims == 4) {
      // 4D tensor: [batch_size, 1, num_detections, 85]
      num_detections = network_output.size[2];
      detection_size = network_output.size[3];
      AA_LOG_DEBUG("4D output tensor: ["
                   << network_output.size[0] << ", " << network_output.size[1]
                   << ", " << num_detections << ", " << detection_size << "]");
    } else {
      AA_LOG_ERROR("Unsupported output tensor dimensions: "
                   << network_output.dims << " (supported: 2D, 3D, 4D)");
      return detections;
    }

    // Validate detection size - need at least 5 values (4 bbox + 1 confidence)
    if (detection_size < 5) {
      AA_LOG_ERROR("Invalid detection size: " << detection_size
                                              << " (expected at least 5)");
      return detections;
    }

    // Validate number of detections
    if (num_detections <= 0) {
      AA_LOG_WARNING("No detections in network output");
      return detections;
    }

    AA_LOG_DEBUG("Parsing output: " << num_detections << " detections, "
                                    << detection_size
                                    << " values per detection");

    if (detection_size != 85) {
      AA_LOG_WARNING("Unexpected detection size: " << detection_size
                                                   << " (expected 85)");
    }

    for (int i = 0; i < num_detections; ++i) {
      const float* detection_data = data + i * detection_size;

      // Additional bounds check: ensure we don't exceed the data buffer
      size_t expected_data_size = 1;
      for (int d = 0; d < network_output.dims; ++d) {
        expected_data_size *= static_cast<size_t>(network_output.size[d]);
      }
      size_t current_offset = static_cast<size_t>(i * detection_size);

      if (current_offset + detection_size > expected_data_size) {
        AA_LOG_ERROR("Data access would exceed buffer bounds at detection "
                     << i);
        break;
      }

      // Extract bbox coordinates (center_x, center_y, width, height)
      float center_x = detection_data[0];
      float center_y = detection_data[1];
      float width = detection_data[2];
      float height = detection_data[3];
      float box_confidence = detection_data[4];

      // Skip detections with low box confidence
      if (box_confidence < confidence_threshold) {
        continue;
      }

      // Find the class with highest confidence
      float max_class_confidence = 0.0f;
      int best_class_id = -1;

      for (int j = 5; j < detection_size; ++j) {
        float class_confidence = detection_data[j];
        if (class_confidence > max_class_confidence) {
          max_class_confidence = class_confidence;
          best_class_id = j - 5;  // COCO class IDs start from 0
        }
      }

      // Calculate final confidence as box_confidence * class_confidence
      float final_confidence = box_confidence * max_class_confidence;

      if (final_confidence < confidence_threshold || best_class_id < 0) {
        continue;
      }

      // Convert from center format to top-left format and scale to network size
      // YOLO outputs normalized coordinates [0,1], scale to network input size
      int network_width = options_.Get<int>("width");
      int network_height = options_.Get<int>("height");

      float scaled_center_x = center_x * network_width;
      float scaled_center_y = center_y * network_height;
      float scaled_width = width * network_width;
      float scaled_height = height * network_height;

      float x = scaled_center_x - scaled_width / 2.0f;
      float y = scaled_center_y - scaled_height / 2.0f;

      // Ensure coordinates are within valid network input range
      x = std::max(
          0.0f, std::min(x, static_cast<float>(network_width) - scaled_width));
      y = std::max(0.0f, std::min(y, static_cast<float>(network_height) -
                                         scaled_height));
      scaled_width = std::max(
          1.0f, std::min(scaled_width, static_cast<float>(network_width) - x));
      scaled_height = std::max(
          1.0f,
          std::min(scaled_height, static_cast<float>(network_height) - y));

      // Create detection with COCO class ID
      Detection detection;
      detection.bbox = cv::Rect(static_cast<int>(x), static_cast<int>(y),
                                static_cast<int>(scaled_width),
                                static_cast<int>(scaled_height));
      detection.class_id = best_class_id;  // COCO class ID (0-79)
      detection.confidence = final_confidence;

      detections.push_back(detection);
      AA_LOG_DEBUG("Added detection: class="
                   << detection.class_id << " confidence=" << std::fixed
                   << std::setprecision(2) << detection.confidence << " bbox=("
                   << detection.bbox.x << "," << detection.bbox.y << ","
                   << detection.bbox.width << "," << detection.bbox.height
                   << ")");
    }

    AA_LOG_INFO("Parsed " << detections.size()
                          << " valid detections with COCO classes");
    return detections;

  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error parsing network output: " << e.what());
    return detections;
  }
}

std::vector<Detection> DetectorServer::ApplyNonMaximumSuppression(
    const std::vector<Detection>& detections) {
  if (detections.empty()) {
    return detections;
  }

  try {
    // NMS parameters - Lower thresholds to keep more detections
    const float score_threshold = 0.1f;  // Lower confidence threshold
    const float nms_threshold = 0.45f;   // IoU threshold for NMS

    // Use batched NMS with class offsets to prevent cross-class suppression
    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    std::vector<int> class_ids;

    boxes.reserve(detections.size());
    confidences.reserve(detections.size());
    class_ids.reserve(detections.size());

    // Find maximum coordinate for class offset calculation
    float max_coord = 0.0f;
    for (const auto& detection : detections) {
      if (detection.confidence >= score_threshold) {
        max_coord = std::max(max_coord,
                             static_cast<float>(std::max(
                                 {detection.bbox.x + detection.bbox.width,
                                  detection.bbox.y + detection.bbox.height})));
      }
    }

    // Prepare data with class-based spatial offsets
    for (const auto& detection : detections) {
      if (detection.confidence >= score_threshold) {
        // Add class-based offset to prevent cross-class NMS
        float class_offset = detection.class_id * (max_coord + 1.0f);
        cv::Rect offset_bbox = detection.bbox;
        offset_bbox.x += static_cast<int>(class_offset);
        offset_bbox.y += static_cast<int>(class_offset);

        boxes.push_back(offset_bbox);
        confidences.push_back(detection.confidence);
        class_ids.push_back(detection.class_id);
      }
    }

    AA_LOG_DEBUG("Applying batched NMS to "
                 << boxes.size() << " detections with class offsets");

    // Apply Non-Maximum Suppression with class offsets
    std::vector<int> nms_indices;
    cv::dnn::NMSBoxes(boxes, confidences, score_threshold, nms_threshold,
                      nms_indices);

    // Build final detection list with original coordinates
    std::vector<Detection> nms_detections;
    nms_detections.reserve(nms_indices.size());

    for (int idx : nms_indices) {
      Detection nms_detection;
      // Use original detection data (without offsets)
      const auto& original_detection = detections[idx];
      nms_detection.bbox = original_detection.bbox;
      nms_detection.confidence = original_detection.confidence;
      nms_detection.class_id = original_detection.class_id;
      nms_detections.push_back(nms_detection);
    }

    AA_LOG_INFO("Batched NMS reduced detections from "
                << detections.size() << " to " << nms_detections.size());

    return nms_detections;

  } catch (const cv::Exception& e) {
    AA_LOG_ERROR("OpenCV error in NMS: " << e.what());
    return detections;  // Return original detections on error
  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error in NMS: " << e.what());
    return detections;  // Return original detections on error
  }
}

std::vector<Detection> DetectorServer::ScaleDetectionsToOriginalFrame(
    const std::vector<Detection>& detections, const cv::Mat& original_frame) {
  if (detections.empty()) {
    return detections;
  }

  try {
    // Get network input size from options
    int network_width = options_.Get<int>("width");
    int network_height = options_.Get<int>("height");
    cv::Size model_size(network_width, network_height);
    cv::Size original_size = original_frame.size();

    // Calculate scaling factors (accounting for letterboxing)
    double scale =
        std::min(static_cast<double>(model_size.width) / original_size.width,
                 static_cast<double>(model_size.height) / original_size.height);

    // Calculate padding offsets from letterboxing
    int new_width = static_cast<int>(original_size.width * scale);
    int new_height = static_cast<int>(original_size.height * scale);
    int dx = (model_size.width - new_width) / 2;
    int dy = (model_size.height - new_height) / 2;

    std::vector<Detection> scaled_detections;
    scaled_detections.reserve(detections.size());

    AA_LOG_DEBUG("Scaling " << detections.size() << " detections from "
                            << model_size.width << "x" << model_size.height
                            << " to " << original_size.width << "x"
                            << original_size.height);

    for (const auto& detection : detections) {
      Detection scaled_detection;

      // Remove letterbox padding and scale to original size
      double x = (detection.bbox.x - dx) / scale;
      double y = (detection.bbox.y - dy) / scale;
      double width = detection.bbox.width / scale;
      double height = detection.bbox.height / scale;

      // Clamp to original frame boundaries
      x = std::max(0.0, std::min(x, static_cast<double>(original_size.width)));
      y = std::max(0.0, std::min(y, static_cast<double>(original_size.height)));
      width = std::max(
          1.0, std::min(width, static_cast<double>(original_size.width) - x));
      height = std::max(
          1.0, std::min(height, static_cast<double>(original_size.height) - y));

      scaled_detection.bbox =
          cv::Rect(static_cast<int>(x), static_cast<int>(y),
                   static_cast<int>(width), static_cast<int>(height));
      scaled_detection.class_id = detection.class_id;
      scaled_detection.confidence = detection.confidence;

      scaled_detections.push_back(scaled_detection);
    }

    AA_LOG_INFO("Scaled " << scaled_detections.size()
                          << " detections to original frame size");
    return scaled_detections;

  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error scaling detections: " << e.what());
    return detections;  // Return original detections on error
  }
}

std::vector<Detection> DetectorServer::FilterDetectionsByPolygons(
    const std::vector<Detection>& detections,
    const std::vector<aa::shared::Polygon>& polygons) {
  std::vector<Detection> filtered_detections;

  for (const auto& detection : detections) {
    auto [center_x, center_y] = GetDetectionCenter(detection);

    auto containing_polygons =
        FindContainingPolygons(center_x, center_y, polygons);

    if (containing_polygons.empty()) {
      continue;
    }

    std::sort(containing_polygons.begin(), containing_polygons.end(),
              [](const aa::shared::Polygon* a, const aa::shared::Polygon* b) {
                return a->GetPriority() > b->GetPriority();
              });

    if (ShouldIncludeDetection(detection, containing_polygons)) {
      filtered_detections.push_back(detection);
    }
  }

  return filtered_detections;
}

std::pair<double, double> DetectorServer::GetDetectionCenter(
    const Detection& detection) {
  const cv::Rect& bbox = detection.bbox;
  double center_x = bbox.x + bbox.width / 2.0;
  double center_y = bbox.y + bbox.height / 2.0;
  return {center_x, center_y};
}

std::vector<const aa::shared::Polygon*> DetectorServer::FindContainingPolygons(
    double center_x, double center_y,
    const std::vector<aa::shared::Polygon>& polygons) {
  std::vector<const aa::shared::Polygon*> containing_polygons;

  for (const auto& polygon : polygons) {
    if (polygon.Contains(center_x, center_y)) {
      containing_polygons.push_back(&polygon);
    }
  }

  return containing_polygons;
}

bool DetectorServer::ShouldIncludeDetection(
    const Detection& detection,
    const std::vector<const aa::shared::Polygon*>& containing_polygons) {
  if (containing_polygons.empty()) {
    return false;
  }

  const aa::shared::Polygon* highest_priority_polygon = containing_polygons[0];

  if (highest_priority_polygon->GetType() ==
      aa::shared::PolygonType::EXCLUSION) {
    return false;
  } else if (highest_priority_polygon->GetType() ==
             aa::shared::PolygonType::INCLUSION) {
    return IsDetectionClassAllowed(detection, *highest_priority_polygon);
  }

  return false;
}

bool DetectorServer::IsDetectionClassAllowed(
    const Detection& detection, const aa::shared::Polygon& polygon) {
  const auto& target_classes = polygon.GetTargetClasses();

  if (target_classes.empty()) {
    return true;
  }

  auto it = std::find(target_classes.begin(), target_classes.end(),
                      detection.class_id);
  return it != target_classes.end();
}

void DetectorServer::DrawBoundingBoxes(
    cv::Mat& frame, const std::vector<Detection>& detections) const {
  // COCO class names for better visualization
  static const std::vector<std::string> coco_classes = {
      "person",        "bicycle",      "car",
      "motorcycle",    "airplane",     "bus",
      "train",         "truck",        "boat",
      "traffic light", "fire hydrant", "stop sign",
      "parking meter", "bench",        "bird",
      "cat",           "dog",          "horse",
      "sheep",         "cow",          "elephant",
      "bear",          "zebra",        "giraffe",
      "backpack",      "umbrella",     "handbag",
      "tie",           "suitcase",     "frisbee",
      "skis",          "snowboard",    "sports ball",
      "kite",          "baseball bat", "baseball glove",
      "skateboard",    "surfboard",    "tennis racket",
      "bottle",        "wine glass",   "cup",
      "fork",          "knife",        "spoon",
      "bowl",          "banana",       "apple",
      "sandwich",      "orange",       "broccoli",
      "carrot",        "hot dog",      "pizza",
      "donut",         "cake",         "chair",
      "couch",         "potted plant", "bed",
      "dining table",  "toilet",       "tv",
      "laptop",        "mouse",        "remote",
      "keyboard",      "cell phone",   "microwave",
      "oven",          "toaster",      "sink",
      "refrigerator",  "book",         "clock",
      "vase",          "scissors",     "teddy bear",
      "hair drier",    "toothbrush"};

  // Define colors for different classes (cycling through a set of distinct
  // colors)
  static const std::vector<cv::Scalar> colors = {
      cv::Scalar(255, 0, 0),      // Red
      cv::Scalar(0, 255, 0),      // Green
      cv::Scalar(0, 0, 255),      // Blue
      cv::Scalar(255, 255, 0),    // Cyan
      cv::Scalar(255, 0, 255),    // Magenta
      cv::Scalar(0, 255, 255),    // Yellow
      cv::Scalar(128, 0, 128),    // Purple
      cv::Scalar(255, 165, 0),    // Orange
      cv::Scalar(255, 192, 203),  // Pink
      cv::Scalar(0, 128, 0)       // Dark Green
  };

  for (const auto& detection : detections) {
    // Choose color based on class ID
    cv::Scalar color = colors[detection.class_id % colors.size()];

    // Draw bounding box rectangle
    cv::rectangle(frame, detection.bbox, color, 1);

    // Prepare label text
    std::string class_name =
        (detection.class_id < static_cast<int>(coco_classes.size()))
            ? coco_classes[detection.class_id]
            : "class_" + std::to_string(detection.class_id);

    std::string label =
        class_name + " " +
        std::to_string(static_cast<int>(detection.confidence * 100)) + "%";

    // Calculate text size for background rectangle
    int baseline = 0;
    cv::Size text_size =
        cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);

    // Draw background rectangle for text
    cv::Point text_origin(detection.bbox.x, detection.bbox.y - 5);
    cv::Rect text_rect(text_origin.x, text_origin.y - text_size.height,
                       text_size.width, text_size.height + baseline);
    cv::rectangle(frame, text_rect, color, cv::FILLED);

    // Draw text label
    cv::putText(frame, label, text_origin, cv::FONT_HERSHEY_SIMPLEX, 0.5,
                cv::Scalar(255, 255, 255), 1);
  }
}

}  // namespace aa::server
