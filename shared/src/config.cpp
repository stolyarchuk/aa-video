#include "aa/shared/config.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace aa::shared {

bool Config::LoadFromFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return false;
  }

  std::string line;
  while (std::getline(file, line)) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == '#') {
      continue;
    }

    // Parse key=value pairs
    size_t equals_pos = line.find('=');
    if (equals_pos != std::string::npos) {
      std::string key = line.substr(0, equals_pos);
      std::string value = line.substr(equals_pos + 1);

      // Trim whitespace
      key.erase(0, key.find_first_not_of(" \t"));
      key.erase(key.find_last_not_of(" \t") + 1);
      value.erase(0, value.find_first_not_of(" \t"));
      value.erase(value.find_last_not_of(" \t") + 1);

      config_[key] = value;
    }
  }

  return true;
}

std::string Config::GetString(const std::string& key, const std::string& default_value) const {
  auto it = config_.find(key);
  return (it != config_.end()) ? it->second : default_value;
}

int Config::GetInt(const std::string& key, int default_value) const {
  auto it = config_.find(key);
  if (it != config_.end()) {
    try {
      return std::stoi(it->second);
    } catch (const std::exception&) {
      // Fall through to default
    }
  }
  return default_value;
}

double Config::GetDouble(const std::string& key, double default_value) const {
  auto it = config_.find(key);
  if (it != config_.end()) {
    try {
      return std::stod(it->second);
    } catch (const std::exception&) {
      // Fall through to default
    }
  }
  return default_value;
}

void Config::Set(const std::string& key, const std::string& value) { config_[key] = value; }

}  // namespace aa::shared
