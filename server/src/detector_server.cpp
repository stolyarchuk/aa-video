#include "detector_server.h"

#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include "logging.h"

namespace aa::server {

DetectorServer::DetectorServer(aa::shared::Options options)
    : options_{std::move(options)} {
  service_ = std::make_unique<DetectorServiceImpl>(options_.GetAddress());
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
    const aa::shared::CheckHealthRequest*,
    aa::shared::CheckHealthResponse*) const {
  return grpc::Status::OK;
}

void DetectorServer::InitializeNetwork() {
  // Load ResNet model first
  if (!LoadModel()) {
    throw std::runtime_error("Failed to initialize ResNet model");
  }

  // Set DNN backend and target after loading model
  dnn_network_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
  dnn_network_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

bool DetectorServer::LoadModel() {
  try {
    std::string model_path = options_.GetModelPath();
    AA_LOG_INFO("Loading model from: " << model_path);

    dnn_network_ = cv::dnn::readNetFromONNX(model_path);

    // Check if network is empty (indicates loading failure)
    if (dnn_network_.empty()) {
      AA_LOG_ERROR("Failed to load model: network is empty");
      return false;
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
  // Modern ResNet preprocessing parameters (ImageNet standard)
  const cv::Size network_input_size(224, 224);  // Standard ResNet input size
  const cv::Scalar mean_values(123.675, 116.28, 103.53);  // ImageNet mean (BGR)
  const double scale_factor = 1.0 / 255.0;                // Normalize to [0,1]
  const bool swap_rb = true;                              // Convert BGR to RGB
  const bool crop = false;  // No center cropping for inference

  try {
    // Create blob from image using OpenCV 4.8.1 blobFromImage
    cv::Mat blob = cv::dnn::blobFromImage(
        frame,               // Input image
        scale_factor,        // Scale factor for pixel values
        network_input_size,  // Target size for network input
        mean_values,         // Mean subtraction values (BGR order)
        swap_rb,             // Swap red and blue channels (BGR -> RGB)
        crop,                // Center crop
        CV_32F               // Output data type (float32)
    );

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
    const cv::Mat& network_output, const cv::Mat& original_frame) {
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
      if (i < network_output.dims - 1) AA_LOG_DEBUG("x");
    }

  } catch (const cv::Exception& e) {
    AA_LOG_ERROR("OpenCV error in post-processing: " << e.what());
  } catch (const std::exception& e) {
    AA_LOG_ERROR("Error in post-processing: " << e.what());
  }

  return detections;
}

grpc::Status DetectorServer::ProcessFrame(
    const aa::shared::ProcessFrameRequest* /* request */,
    aa::shared::ProcessFrameResponse* /* response */) const {
  try {
    // TODO: Extract cv::Mat from request frame data
    // For now, create a dummy frame for testing
    cv::Mat input_frame = cv::Mat::zeros(480, 640, CV_8UC3);

    // Step 1: Preprocess frame for neural network
    cv::Mat preprocessed_blob =
        const_cast<DetectorServer*>(this)->PreprocessFrame(input_frame);
    if (preprocessed_blob.empty()) {
      return grpc::Status(grpc::StatusCode::INTERNAL,
                          "Failed to preprocess frame");
    }

    // Step 2: Run inference using ResNet model
    cv::Mat network_output =
        const_cast<DetectorServer*>(this)->RunInference(preprocessed_blob);
    if (network_output.empty()) {
      return grpc::Status(grpc::StatusCode::INTERNAL,
                          "Failed to run inference");
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
