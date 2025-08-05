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
 * std::string input = options.Get<std::string>("input");
 * int width = options.Get<int>("width");
 * double confidence = options.Get<double>("confidence");
 * bool verbose = options.IsVerbose();
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
  Options(int argc, const char* const argv[], std::string_view name);

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
   * @brief Get a configuration value by parameter name
   *
   * @tparam T The type to retrieve (std::string, int, double, bool)
   * @param parameter_name The name of the parameter (e.g., "input", "width",
   * "confidence")
   * @return T The parameter value of the specified type
   */
  template <typename T>
  T Get(const std::string& parameter_name) const {
    return parser_.get<T>(parameter_name);
  }

  /**
   * @brief Check if verbose output is enabled
   *
   * @return true if verbose mode is enabled
   * @return false if verbose mode is disabled
   */
  bool IsVerbose() const;

 private:
  cv::CommandLineParser parser_;
  bool is_valid_;
  std::string instance_name_;

  /**
   * @brief Initialize the command line parser with option definitions
   *
   * @param argc Number of command line arguments
   * @param argv Array of command line argument strings
   */
  void InitializeParser(int argc, const char* const argv[],
                        std::string_view name);

  /**
   * @brief Validate the parsed arguments
   *
   * @return true if all required arguments are present and valid
   * @return false otherwise
   */
  bool ValidateArguments();
};

}  // namespace aa::shared
