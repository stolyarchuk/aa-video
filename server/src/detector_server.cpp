#include "detector_server.h"

#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include "logging.h"
#include "polygon.h"
#include "frame.h"

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
  // Load ResNet model first
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

    // Check if this is a ViT model that may have dynamic shapes
    bool is_vit_model = model_path.find("vit") != std::string::npos;

    if (is_vit_model) {
      AA_LOG_WARNING(
          "Detected Vision Transformer model. OpenCV DNN may not support "
          "dynamic shapes.");
      AA_LOG_WARNING(
          "Consider using a different inference backend (ONNX Runtime, "
          "TensorRT, etc.) for ViT models.");

      // Try to load with more permissive settings
      try {
        dnn_network_ = cv::dnn::readNetFromONNX(model_path);
      } catch (const cv::Exception& vit_error) {
        AA_LOG_ERROR(
            "ViT model loading failed with OpenCV DNN: " << vit_error.what());
        AA_LOG_ERROR(
            "Recommendation: Use ResNet models for OpenCV DNN compatibility, "
            "or implement ONNX Runtime backend");
        return false;
      }
    } else {
      dnn_network_ = cv::dnn::readNetFromONNX(model_path);
    }

    // Check if network is empty (indicates loading failure)
    if (dnn_network_.empty()) {
      AA_LOG_ERROR("Failed to load model: network is empty");
      return false;
    }

    // For ViT models, try to get input layer info to verify compatibility
    if (is_vit_model) {
      try {
        auto layer_names = dnn_network_.getLayerNames();
        AA_LOG_INFO("Model has " << layer_names.size() << " layers");

        // Get unconnected output layers to understand the model structure
        auto unconnected_layers = dnn_network_.getUnconnectedOutLayers();
        AA_LOG_INFO("Model has " << unconnected_layers.size()
                                 << " output layers");

      } catch (const cv::Exception& shape_error) {
        AA_LOG_WARNING(
            "Could not analyze model structure: " << shape_error.what());
      }
    }

    AA_LOG_INFO("Model loaded successfully");
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
  // Determine model type for appropriate preprocessing
  auto model_path = options_.Get<std::string>("model");
  bool is_vit_model = model_path.find("vit") != std::string::npos;

  cv::Size network_input_size;
  cv::Scalar mean_values;
  cv::Scalar std_values;
  double scale_factor;
  bool swap_rb;
  bool crop = false;

  if (is_vit_model) {
    // ViT preprocessing parameters
    network_input_size = cv::Size(224, 224);  // ViT-Base patch16-224 input
    mean_values = cv::Scalar(0.485, 0.456, 0.406);  // ImageNet mean (RGB)
    std_values = cv::Scalar(0.229, 0.224, 0.225);   // ImageNet std (RGB)
    scale_factor = 1.0 / 255.0;                     // Normalize to [0,1] first
    swap_rb = true;  // Convert BGR to RGB for ViT
  } else {
    // ResNet preprocessing parameters
    network_input_size = cv::Size(224, 224);  // Standard ResNet input size
    mean_values = cv::Scalar(123.675, 116.28, 103.53);  // ImageNet mean (BGR)
    scale_factor = 1.0 / 255.0;                         // Normalize to [0,1]
    swap_rb = true;                                     // Convert BGR to RGB
  }

  try {
    cv::Mat blob;

    if (is_vit_model) {
      // ViT requires normalization with both mean and std
      blob = cv::dnn::blobFromImage(
          frame,               // Input image
          scale_factor,        // Scale factor for pixel values
          network_input_size,  // Target size for network input
          cv::Scalar(),        // No mean subtraction in blobFromImage for ViT
          swap_rb,             // Swap red and blue channels (BGR -> RGB)
          crop                 // Center crop
      );

      // Manual normalization for ViT: (pixel/255 - mean) / std
      // This requires reshaping and manual computation
      // For now, use standard normalization and log a warning
      AA_LOG_WARNING("ViT normalization may not be optimal with OpenCV DNN");

    } else {
      // Standard ResNet preprocessing
      blob = cv::dnn::blobFromImage(
          frame,               // Input image
          scale_factor,        // Scale factor for pixel values
          network_input_size,  // Target size for network input
          mean_values,         // Mean subtraction values (BGR order)
          swap_rb,             // Swap red and blue channels (BGR -> RGB)
          crop                 // Center crop
      );
    }

    // Verify blob dimensions [N, C, H, W] = [1, 3, 224, 224]
    if (blob.dims == 4 && blob.size[1] == 3 &&
        blob.size[2] == network_input_size.height &&
        blob.size[3] == network_input_size.width) {
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
    dnn_network_.setInput(blob, "data");  // "data" is common input layer name

    // Run forward pass through the network
    cv::Mat output = dnn_network_.forward();

    // For ResNet classification, output shape is typically [1, num_classes]
    // For ResNet-50 on ImageNet: [1, 1000]
    return output;

  } catch (const cv::Exception& e) {
    AA_LOG_ERROR("OpenCV error during inference: " << e.what());
    return cv::Mat();  // Return empty Mat on error
  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error during inference: " << e.what());
    return cv::Mat();
  }
}

std::vector<cv::Rect> DetectorServer::PostprocessDetections(
    const cv::Mat& network_output, const cv::Mat& /* original_frame */) {
  std::vector<cv::Rect> detections;

  try {
    // For ResNet classification, we typically get class probabilities
    // not bounding boxes. This method would be more relevant for
    // object detection networks like YOLO, SSD, etc.

    if (network_output.empty()) {
      return detections;  // Return empty if no output
    }

    // TODO: For object detection ResNet variants (e.g., Faster R-CNN with
    // ResNet backbone):
    // 1. Parse detection results (confidence scores, class IDs, bounding boxes)
    // 2. Apply confidence threshold filtering (e.g., confidence > 0.5)
    // 3. Apply Non-Maximum Suppression (NMS) to remove duplicate detections
    // 4. Scale bounding boxes from network coordinates to original frame size
    //
    // Example for classification ResNet:
    // - network_output would be [1, num_classes] with class probabilities
    // - We would find the class with maximum probability
    // - For pure classification, no bounding boxes are produced
    //
    // For this skeleton implementation, we return empty detections
    // since classification ResNet doesn't produce bounding boxes

    // Log network output dimensions for debugging
    AA_LOG_DEBUG("Network output dims: " << network_output.dims << ", size: ");
    for (int i = 0; i < network_output.dims; ++i) {
      AA_LOG_DEBUG(network_output.size[i]);
      if (i < network_output.dims - 1) {
        AA_LOG_DEBUG("x");
      }
    }

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
    }

    std::sort(polygons.begin(), polygons.end(),
              [](const aa::shared::Polygon& a, const aa::shared::Polygon& b) {
                return a.GetPriority() > b.GetPriority();
              });

    auto frame = aa::shared::Frame::FromProto(request->frame());

    cv::Size model_size(224, 224);  // TODO: Get from options

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

    cv::Mat resized_image;
    cv::resize(input_frame, resized_image, model_size, 0, 0, cv::INTER_LINEAR);

    // Step 1: Preprocess frame for neural network
    cv::Mat preprocessed_blob =
        const_cast<DetectorServer*>(this)->PreprocessFrame(resized_image);

    if (preprocessed_blob.empty()) {
      AA_LOG_ERROR("Failed to preprocess frame");
      response->set_success(false);
    }

    // Step 2: Run inference using ResNet model
    cv::Mat network_output =
        const_cast<DetectorServer*>(this)->RunInference(preprocessed_blob);

    if (network_output.empty()) {
      AA_LOG_ERROR("Failed to run inference on preprocessed blob");
      response->set_success(false);
    }

    // Step 3: Post-process results to extract detections
    std::vector<cv::Rect> detections =
        const_cast<DetectorServer*>(this)->PostprocessDetections(network_output,
                                                                 input_frame);

    // Step 4: Populate response with detection results
    // TODO: Convert detections to protobuf response format
    // For now, just set success status

    AA_LOG_INFO("Processed frame successfully. Found " << detections.size()
                                                       << " detections.");

    return grpc::Status::OK;
  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error processing frame: " << e.what());
    return grpc::Status(grpc::StatusCode::INTERNAL, "Frame processing failed");
  }
}

}  // namespace aa::server
