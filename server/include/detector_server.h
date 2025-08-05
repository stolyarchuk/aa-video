#pragma once

#include <string_view>

#include "detector_service.h"

namespace aa::server {

/**
 * @brief High-level server wrapper for detector service using CRTP
 *
 * Provides a convenient interface to manage the lifecycle of the detector
 * service server, including initialization, startup, and shutdown operations.
 * Uses composition with DetectorServiceImpl instead of inheritance.
 */
class DetectorServer {
 public:
  /**
   * @brief Construct a new Detector Server object
   *
   * @param address Server listening address (e.g., "localhost:50051")
   */
  explicit DetectorServer(std::string_view address);

  /**
   * @brief Destroy the Detector Server object
   */
  ~DetectorServer() = default;

  // Disable copy construction and assignment
  DetectorServer(const DetectorServer&) = delete;
  DetectorServer& operator=(const DetectorServer&) = delete;

  // Enable move construction and assignment
  DetectorServer(DetectorServer&&) noexcept = default;
  DetectorServer& operator=(DetectorServer&&) noexcept = default;

  /**
   * @brief Initialize the server components
   */
  void Initialize();

  /**
   * @brief Start the server and begin listening for requests
   */
  void Start();

  /**
   * @brief Shutdown the server gracefully
   */
  void Shutdown();

  grpc::Status CheckHealth(const aa::shared::CheckHealthRequest*,
                           aa::shared::CheckHealthResponse*) const;

 private:
  std::unique_ptr<DetectorServiceImpl> service_;
};

}  // namespace aa::server
