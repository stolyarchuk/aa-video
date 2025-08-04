#pragma once

#include <string>
#include <map>

namespace aa::shared {

/**
 * @brief Configuration management for video processing
 */
class Config {
 public:
  Config() = default;
  ~Config() = default;

  /**
   * @brief Load configuration from file
   * @param filename Configuration file path
   * @return True if loaded successfully
   */
  bool LoadFromFile(const std::string& filename);

  /**
   * @brief Get string value
   * @param key Configuration key
   * @param default_value Default value if key not found
   * @return Configuration value
   */
  std::string GetString(const std::string& key, const std::string& default_value = "") const;

  /**
   * @brief Get integer value
   * @param key Configuration key
   * @param default_value Default value if key not found
   * @return Configuration value
   */
  int GetInt(const std::string& key, int default_value = 0) const;

  /**
   * @brief Get double value
   * @param key Configuration key
   * @param default_value Default value if key not found
   * @return Configuration value
   */
  double GetDouble(const std::string& key, double default_value = 0.0) const;

  /**
   * @brief Set configuration value
   * @param key Configuration key
   * @param value Configuration value
   */
  void Set(const std::string& key, const std::string& value);

  // Default configuration constants
  static constexpr int kDefaultPort = 50051;
  static constexpr int kDefaultMaxConnections = 10;
  static constexpr int kDefaultFrameBufferSize = 10;
  static constexpr const char* kDefaultServerAddress = "localhost:50051";

 private:
  std::map<std::string, std::string> config_;
};

}  // namespace aa::shared
