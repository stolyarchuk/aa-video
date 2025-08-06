#pragma once
#include <functional>
#include <memory>
#include <string_view>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "logging.h"
// #include "manual_reset_event.h"

namespace aa::server {

/**
 * @brief CRTP base class for gRPC server implementations
 *
 * Provides common gRPC server functionality including server building,
 * lifecycle management, and graceful shutdown. Uses CRTP (Curiously Recurring
 * Template Pattern) to allow derived classes to inherit server capabilities.
 *
 * @tparam Impl The derived service implementation class
 *
 * @grpc
 * @threadsafe Methods are thread-safe for concurrent access
 *
 * Features:
 * - Automatic health check service registration
 * - Proto reflection for debugging
 * - Insecure server credentials for development
 * - Configurable shutdown timeout
 *
 * Example usage:
 * @code
 * class MyServiceImpl : public MyService::Service,
 *                       public RpcServerFromThis<MyServiceImpl> {
 *   public:
 *     explicit MyServiceImpl(std::string_view address)
 *         : RpcServerFromThis{address} {}
 * };
 * @endcode
 */
template <class Impl>
class RpcServerFromThis {
 public:
  /**
   * @brief Construct a new RPC Server with specified listening address
   *
   * @param address Server address in format "host:port" (e.g.,
   * "localhost:50051")
   *
   * Automatically enables default health check service and proto reflection.
   */
  constexpr explicit RpcServerFromThis(std::string_view address)
      : address_{address}, service_impl_{static_cast<Impl&>(*this)} {
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  }

  /**
   * @brief Build and start the gRPC server
   *
   * Creates the server with insecure credentials and registers the service.
   * Must be called before Wait().
   */
  void Build() {
    grpc::ServerBuilder builder;

    builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_impl_);

    server_ = std::unique_ptr<grpc::Server>(builder.BuildAndStart());  // NOLINT
  }

  /**
   * @brief Wait for the server to be shutdown
   *
   * Blocks the calling thread until the server is shut down.
   * Build() must be called first.
   */
  void Wait() {
    AA_LOG_INFO("server started listening on " << address_);
    server_->Wait();
  }

  /**
   * @brief Shutdown the server gracefully
   *
   * @param deadline Maximum time to wait for graceful shutdown (default: 100ms)
   *
   * Attempts to shutdown the server gracefully within the specified deadline.
   * Safe to call multiple times or on a server that was never started.
   */
  void Stop(const std::chrono::milliseconds& deadline =
                std::chrono::milliseconds{100}) {
    if (server_) {
      server_->Shutdown(std::chrono::system_clock::now() + deadline);
      AA_LOG_INFO("server stopped listening on " << address_);
    } else {
      AA_LOG_DEBUG("Stop called on server that was never started");
    }
  }

 private:
  std::string address_;  ///< Server listening address
  Impl& service_impl_;   ///< Reference to derived service implementation
  std::unique_ptr<grpc::Server> server_;  ///< gRPC server instance
};

/**
 * @brief Observer pattern implementation for method registration
 *
 * Template class that stores a callable method for later invocation.
 * Used internally by the Observable pattern for service method registration.
 *
 * @tparam Signature Function signature for the stored method
 */
template <typename Signature>
class Observer {
 public:
  /**
   * @brief Default constructor for Observer
   */
  constexpr Observer() = default;

  /**
   * @brief Deleted copy constructor - Observers are not copyable
   */
  Observer(const Observer&) = delete;

  /**
   * @brief Deleted copy assignment - Observers are not copyable
   */
  Observer& operator=(const Observer&) = delete;

 private:
  template <typename Observers>
  friend class Observable;

  using Method = std::function<Signature>;  ///< Stored method function
  Method method_;                           ///< The actual callable method
};

/**
 * @brief Observable pattern implementation for service method management
 *
 * Provides method registration and invocation capabilities for gRPC services.
 * Allows services to register handlers for different method types and invoke
 * them with proper error handling and context management.
 *
 * @tparam Observers Observer table type containing method signatures
 */
template <typename Observers>
class Observable {
 private:
  using ObserverTable = typename Observers::ObserverTable;

 public:
  template <size_t ObserverId, typename F>
  auto Register(F&& func) {
    std::get<ObserverId>(methods_).method_ = std::move(func);
  }

 protected:
  constexpr Observable() = default;
  template <size_t ObserverId, typename... Args>
  grpc::Status Invoke(grpc::ServerContext* ctx, Args&&... args) const {
    using grpc::Status;
    using grpc::StatusCode;

    if (ctx->IsCancelled())
      return {StatusCode::CANCELLED,
              "deadline exceeded or client cancelled, abandoning."};

    return std::get<ObserverId>(methods_).method_(std::forward<Args>(args)...);
  }

 private:
  ObserverTable methods_;
};

template <typename Request, typename Response>
using ServiceMethod = Observer<grpc::Status(const Request*, Response*)>;

template <typename Request, typename Response>
using ServiceStream =
    Observer<grpc::Status(grpc::ServerReader<Request>*, Response*)>;

}  // namespace aa::server
