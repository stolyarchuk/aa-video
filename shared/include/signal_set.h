#pragma once

#include <atomic>
#include <csignal>
#include <functional>
#include <map>
#include <memory>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "logging.h"

namespace aa::shared {

/**
 * @brief Modern C++ signal handling utility for graceful shutdown
 *
 * This class provides a thread-safe, RAII-based approach to system signal
 * handling using the self-pipe trick with Linux eventfd for optimal
 * performance. It allows registration of lambda handlers for different signals
 * and ensures graceful cleanup.
 *
 * The implementation avoids the pitfalls of traditional signal handling by:
 * - Using eventfd for async-signal-safe communication
 * - Running signal handlers in a dedicated thread
 * - Providing type-safe lambda registration
 * - Ensuring proper cleanup on destruction
 *
 * Example usage:
 * @code
 * aa::shared::SignalSet signal_set;
 *
 * // Register graceful shutdown handlers
 * signal_set.Add(SIGINT, [&](int sig) {
 *   AA_LOG_INFO("Received SIGINT, shutting down gracefully...");
 *   server.Shutdown();
 * });
 *
 * signal_set.Add(SIGTERM, [&](int sig) {
 *   AA_LOG_INFO("Received SIGTERM, shutting down gracefully...");
 *   server.Shutdown();
 * });
 *
 * // Signal handling starts automatically and stops on destruction
 * @endcode
 */
class SignalSet {
 public:
  /**
   * @brief Construct a new Signal Set object
   *
   * Initializes the eventfd and signal handling thread.
   *
   * @throws std::runtime_error if eventfd creation fails
   */
  SignalSet();

  /**
   * @brief Destroy the Signal Set object
   *
   * Automatically stops signal handling and cleans up resources.
   */
  ~SignalSet();

  // Disable copy construction and assignment
  SignalSet(const SignalSet&) = delete;
  SignalSet& operator=(const SignalSet&) = delete;

  // Enable move construction and assignment
  SignalSet(SignalSet&& other) noexcept;
  SignalSet& operator=(SignalSet&& other) noexcept;

  /**
   * @brief Add a signal handler
   *
   * Registers a callable handler for the specified signal. The handler will be
   * called from a dedicated signal handling thread, making it safe to perform
   * complex operations including I/O and memory allocation.
   *
   * @tparam T Callable type (lambda, function pointer, std::function, etc.)
   * @param signal Signal number (e.g., SIGINT, SIGTERM, SIGUSR1)
   * @param handler Callable object that accepts an int parameter (signal
   * number)
   * @throws std::runtime_error if signal registration fails
   */
  template <typename T>
  void Add(int signal, T&& handler) {
    static_assert(std::is_invocable_v<T, int>,
                  "Handler must be callable with int parameter");

    // Store the handler
    handlers_[signal] = std::forward<T>(handler);

    // Save original handler if not already saved
    if (original_handlers_.find(signal) == original_handlers_.end()) {
      struct sigaction original_action;
      if (sigaction(signal, nullptr, &original_action) == -1) {
        AA_LOG_ERROR("Failed to get original signal handler for signal "
                     << signal);
        throw std::runtime_error("Failed to get original signal handler");
      }
      original_handlers_[signal] = original_action;
    }

    // Install our static signal handler
    struct sigaction sa;
    sa.sa_handler = &SignalSet::StaticSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(signal, &sa, nullptr) == -1) {
      AA_LOG_ERROR("Failed to register signal handler for signal " << signal);
      throw std::runtime_error("Failed to register signal handler");
    }

    // Track registered signals
    registered_signals_.push_back(signal);

    AA_LOG_DEBUG("Registered signal handler for signal " << signal);
  }

  /**
   * @brief Check if signal handling is active
   *
   * @return true if the signal handling thread is running
   * @return false if signal handling has been stopped
   */
  bool IsActive() const;

  /**
   * @brief Stop signal handling
   *
   * Gracefully stops the signal handling thread and restores default signal
   * handlers. This is called automatically by the destructor.
   */
  void Stop();

 private:
  /**
   * @brief Static signal handler that writes to eventfd
   *
   * This is the actual signal handler installed via sigaction. It performs
   * minimal work (writing to eventfd) to maintain async-signal-safety.
   *
   * @param signal Signal number that was received
   */
  static void StaticSignalHandler(int signal);

  /**
   * @brief Handle signals in a dedicated thread
   *
   * Reads from eventfd and dispatches to registered handlers. This runs in a
   * separate thread to avoid signal handling restrictions.
   */
  void HandleSignals();

  /**
   * @brief Initialize the signal handling system
   *
   * Creates eventfd and starts the signal handling thread.
   */
  void Initialize();

  /**
   * @brief Cleanup signal handling resources
   *
   * Restores default signal handlers and closes eventfd.
   */
  void Cleanup();

  // Static members for signal communication
  static std::atomic<int> eventfd_;
  static std::atomic<bool> should_stop_;

  // Instance members
  std::map<int, std::function<void(int)>> handlers_;
  std::vector<int> registered_signals_;
  std::unique_ptr<std::thread> signal_thread_;
  std::atomic<bool> is_active_;

  // Original signal handlers for restoration
  std::map<int, struct sigaction> original_handlers_;
};

}  // namespace aa::shared
