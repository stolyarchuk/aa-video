#include "signal_set.h"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>

namespace aa::shared {

// Static member definitions
std::atomic<int> SignalSet::eventfd_{-1};
std::atomic<bool> SignalSet::should_stop_{false};

SignalSet::SignalSet() : is_active_{false} { Initialize(); }

SignalSet::~SignalSet() { Stop(); }

void SignalSet::Initialize() {
  // Create eventfd for async-signal-safe communication
  int fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (fd == -1) {
    AA_LOG_ERROR("Failed to create eventfd: " << std::strerror(errno));
    throw std::runtime_error("Failed to create eventfd");
  }

  eventfd_.store(fd);
  should_stop_.store(false);
  is_active_.store(true);

  // Start signal handling thread
  signal_thread_ =
      std::make_unique<std::thread>(&SignalSet::HandleSignals, this);

  AA_LOG_DEBUG("SignalSet initialized with eventfd " << fd);
}

void SignalSet::HandleSignals() {
  AA_LOG_DEBUG("Signal handling thread started");

  while (!should_stop_.load()) {
    uint64_t signal_data;
    int fd = eventfd_.load();

    if (fd == -1) {
      break;
    }

    // Read from eventfd (blocking)
    ssize_t bytes_read = read(fd, &signal_data, sizeof(signal_data));

    if (bytes_read == sizeof(signal_data)) {
      // signal_data contains the signal number
      int signal = static_cast<int>(signal_data);

      AA_LOG_DEBUG("Received signal " << signal << " via eventfd");

      // Find and call the appropriate handler
      auto it = handlers_.find(signal);
      if (it != handlers_.end()) {
        try {
          it->second(signal);
        } catch (const std::exception& e) {
          AA_LOG_ERROR("Exception in signal handler for signal "
                       << signal << ": " << e.what());
        } catch (...) {
          AA_LOG_ERROR("Unknown exception in signal handler for signal "
                       << signal);
        }
      } else {
        AA_LOG_WARNING("No handler registered for signal " << signal);
      }
    } else if (bytes_read == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data available, sleep briefly and retry
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      } else if (errno == EINTR) {
        // Interrupted by signal, continue
        continue;
      } else {
        AA_LOG_ERROR("Error reading from eventfd: " << std::strerror(errno));
        break;
      }
    }
  }

  AA_LOG_DEBUG("Signal handling thread stopping");
}

void SignalSet::StaticSignalHandler(int signal) {
  // This is called from signal context, so we must be async-signal-safe
  // We only write to eventfd, which is async-signal-safe
  int fd = eventfd_.load();
  if (fd != -1) {
    uint64_t signal_data = static_cast<uint64_t>(signal);
    ssize_t result = write(fd, &signal_data, sizeof(signal_data));
    (void)result;  // Suppress unused variable warning
  }
}

bool SignalSet::IsActive() const { return is_active_.load(); }

void SignalSet::Stop() {
  if (!is_active_.load()) {
    return;
  }

  AA_LOG_DEBUG("Stopping SignalSet");

  // Signal the handling thread to stop
  should_stop_.store(true);

  // Write a dummy value to wake up the handling thread
  int fd = eventfd_.load();
  if (fd != -1) {
    uint64_t dummy = 0;
    write(fd, &dummy, sizeof(dummy));
  }

  // Wait for the thread to finish
  if (signal_thread_ && signal_thread_->joinable()) {
    signal_thread_->join();
  }

  // Cleanup resources
  Cleanup();

  is_active_.store(false);
  AA_LOG_DEBUG("SignalSet stopped");
}

void SignalSet::Cleanup() {
  // Restore original signal handlers
  for (const auto& [signal, original_action] : original_handlers_) {
    if (sigaction(signal, &original_action, nullptr) == -1) {
      AA_LOG_WARNING("Failed to restore original handler for signal "
                     << signal << ": " << std::strerror(errno));
    } else {
      AA_LOG_DEBUG("Restored original handler for signal " << signal);
    }
  }

  original_handlers_.clear();
  registered_signals_.clear();
  handlers_.clear();

  // Close eventfd
  int fd = eventfd_.load();
  if (fd != -1) {
    close(fd);
    eventfd_.store(-1);
    AA_LOG_DEBUG("Closed eventfd " << fd);
  }
}

}  // namespace aa::shared
