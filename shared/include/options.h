#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>

namespace aa::shared {

/**
 * @brief Options class for handling command line arguments using OpenCV's
 * CommandLineParser
 *
 * This class provides a convenient wrapper around cv::CommandLineParser for
 * handling command line arguments in a type-safe and well-documented manner.
 *
 * Example usage:
 * @code
 * aa::shared::Options options(argc, argv);
 * if (!options.IsValid()) {
 *     options.PrintHelp();
 *     return -1;
 * }
 *
 * std::string input = options.GetInput();
 * int width = options.GetWidth();
 * @endcode
 */
class Options {
 public:
  /**
   * @brief Construct a new Options object
   *
   * @param argc Number of command line arguments
   * @param argv Array of command line argument strings
   */
  Options(int argc, const char* const argv[]);

  /**
   * @brief Check if the command line arguments are valid
   *
   * @return true if all required arguments are present and valid
   * @return false if there are missing or invalid arguments
   */
  bool IsValid() const;

  /**
   * @brief Print help message with all available options
   */
  void PrintHelp() const;

  /**
   * @brief Get the input file/camera path
   *
   * @return std::string Input path (file path, camera index, or video stream)
   */
  std::string GetInput() const;

  /**
   * @brief Get the output file path
   *
   * @return std::string Output file path
   */
  std::string GetOutput() const;

  /**
   * @brief Get the frame width for processing
   *
   * @return int Frame width in pixels
   */
  int GetWidth() const;

  /**
   * @brief Get the frame height for processing
   *
   * @return int Frame height in pixels
   */
  int GetHeight() const;

  /**
   * @brief Get the confidence threshold
   *
   * @return double Confidence threshold value (0.0 - 1.0)
   */
  double GetConfidenceThreshold() const;

  /**
   * @brief Get the detection model path
   *
   * @return std::string Path to the detection model file
   */
  std::string GetModelPath() const;

  /**
   * @brief Get the configuration file path
   *
   * @return std::string Path to the configuration file
   */
  std::string GetConfigPath() const;

  /**
   * @brief Get the server address for gRPC communication
   *
   * @return std::string Server address (e.g., "localhost:50051")
   */
  std::string GetAddress() const;

  /**
   * @brief Check if verbose output is enabled
   *
   * @return true if verbose mode is enabled
   * @return false if verbose mode is disabled
   */
  bool IsVerbose() const;

  /**
   * @brief Check if GPU acceleration should be used
   *
   * @return true if GPU acceleration is enabled
   * @return false if CPU processing should be used
   */
  bool UseGpu() const;

 private:
  cv::CommandLineParser parser_;
  bool is_valid_;

  /**
   * @brief Initialize the command line parser with option definitions
   *
   * @param argc Number of command line arguments
   * @param argv Array of command line argument strings
   */
  void InitializeParser(int argc, const char* const argv[]);

  /**
   * @brief Validate the parsed arguments
   *
   * @return true if all required arguments are present and valid
   * @return false otherwise
   */
  bool ValidateArguments();
};

}  // namespace aa::shared
