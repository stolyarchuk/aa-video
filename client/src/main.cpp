#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>

#include "detector_client.h"

using namespace aa::client;

void PrintUsage(const char* program_name) {
  std::cout << "Usage: " << program_name << " [options]\n";
  std::cout << "Options:\n";
  std::cout << "  --server <address>    Server address (default: localhost:50051)\n";
  std::cout << "  --operation <op>      Operation: blur, edge_detect, resize (default: blur)\n";
  std::cout << "  --input <path>        Input video file or camera index (default: 0)\n";
  std::cout << "  --stream              Use streaming mode\n";
  std::cout << "  --help                Show this help\n";
}

int main(int /* argc */, char** /* argv[] */) {
  // std::string server_address = Config::kDefaultServerAddress;
  std::string operation = "blur";
  std::string input = "0";  // Default to camera 0
  bool use_streaming = false;

  // // Parse command line arguments
  // for (int i = 1; i < argc; ++i) {
  //   std::string arg = argv[i];
  //   if (arg == "--server" && i + 1 < argc) {
  //     server_address = argv[++i];
  //   } else if (arg == "--operation" && i + 1 < argc) {
  //     operation = argv[++i];
  //   } else if (arg == "--input" && i + 1 < argc) {
  //     input = argv[++i];
  //   } else if (arg == "--stream") {
  //     use_streaming = true;
  //   } else if (arg == "--help") {
  //     PrintUsage(argv[0]);
  //     return 0;
  //   }
  // }

  std::cout << "Video Processing Client\n";
  // std::cout << "Server: " << server_address << "\n";
  std::cout << "Operation: " << operation << "\n";
  std::cout << "Input: " << input << "\n";
  std::cout << "Streaming: " << (use_streaming ? "Yes" : "No") << "\n\n";

  // Create client
  DetectorClient("server_address");

  // // Check connection
  // if (!client.IsConnected()) {
  //   std::cerr << "Failed to connect to server at " << server_address << "\n";
  //   return 1;
  // }

  // std::cout << "Connected to server successfully!\n";

  // // Open video source
  // cv::VideoCapture cap;
  // if (input == "0" || input == "1" || input == "2") {
  //   // Camera input
  //   int camera_id = std::stoi(input);
  //   cap.open(camera_id);
  // } else {
  //   // File input
  //   cap.open(input);
  // }

  // if (!cap.isOpened()) {
  //   std::cerr << "Failed to open video source: " << input << "\n";
  //   return 1;
  // }

  // std::cout << "Video source opened successfully!\n";
  // std::cout << "Press 'q' to quit, 's' to save frame, 'c' to change operation\n\n";

  // // Prepare operation parameters
  // std::map<std::string, std::string> parameters;
  // if (operation == "blur") {
  //   parameters["kernel_size"] = "15";
  // } else if (operation == "edge_detect") {
  //   parameters["low_threshold"] = "50";
  //   parameters["high_threshold"] = "150";
  // } else if (operation == "resize") {
  //   parameters["width"] = "640";
  //   parameters["height"] = "480";
  // }

  // // Start streaming if requested
  // if (use_streaming) {
  //   if (!client.StartStreamProcessing(operation, parameters)) {
  //     std::cerr << "Failed to start streaming processing\n";
  //     return 1;
  //   }
  //   std::cout << "Streaming processing started\n";
  // }

  // cv::Mat frame, result;
  // auto start_time = std::chrono::high_resolution_clock::now();
  // int frame_count = 0;

  // while (true) {
  //   cap >> frame;
  //   if (frame.empty()) {
  //     break;
  //   }

  //   bool success = false;
  //   auto frame_start = std::chrono::high_resolution_clock::now();

  //   if (use_streaming) {
  //     success = client.SendFrame(frame) && client.ReceiveFrame(result);
  //   } else {
  //     success = client.ProcessFrame(frame, operation, parameters, result);
  //   }

  //   auto frame_end = std::chrono::high_resolution_clock::now();
  //   auto frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start).count();

  //   if (success) {
  //     // Display original and processed frames
  //     cv::imshow("Original", frame);
  //     cv::imshow("Processed", result);

  //     frame_count++;
  //     if (frame_count % 30 == 0) {  // Print stats every 30 frames
  //       auto current_time = std::chrono::high_resolution_clock::now();
  //       auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
  //       double fps = (frame_count * 1000.0) / elapsed;
  //       std::cout << "FPS: " << fps << ", Frame time: " << frame_time << "ms\n";
  //     }
  //   } else {
  //     std::cerr << "Failed to process frame\n";
  //   }

  //   // Handle key presses
  //   char key = cv::waitKey(1) & 0xFF;
  //   if (key == 'q') {
  //     break;
  //   } else if (key == 's' && success) {
  //     std::string filename = "processed_frame_" + std::to_string(frame_count) + ".jpg";
  //     cv::imwrite(filename, result);
  //     std::cout << "Saved frame to " << filename << "\n";
  //   } else if (key == 'c') {
  //     // Cycle through operations
  //     if (operation == "blur") {
  //       operation = "edge_detect";
  //       parameters.clear();
  //       parameters["low_threshold"] = "50";
  //       parameters["high_threshold"] = "150";
  //     } else if (operation == "edge_detect") {
  //       operation = "resize";
  //       parameters.clear();
  //       parameters["width"] = "640";
  //       parameters["height"] = "480";
  //     } else {
  //       operation = "blur";
  //       parameters.clear();
  //       parameters["kernel_size"] = "15";
  //     }
  //     std::cout << "Changed operation to: " << operation << "\n";

  //     if (use_streaming) {
  //       client.StopStreamProcessing();
  //       client.StartStreamProcessing(operation, parameters);
  //     }
  //   }
  // }

  // if (use_streaming) {
  //   client.StopStreamProcessing();
  // }

  // std::cout << "\nProcessed " << frame_count << " frames\n";
  cv::destroyAllWindows();
  return 0;
}
