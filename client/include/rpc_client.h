#pragma once
#include <string_view>

#include <grpcpp/grpcpp.h>

namespace aa::client {

/**
 * @brief Template base class for gRPC client implementations
 *
 * Provides common functionality for gRPC clients including connection
 * management, request timeout handling, and service stub creation. Uses CRTP
 * (Curiously Recurring Template Pattern) to provide type-safe service
 * integration.
 *
 * @tparam Impl The gRPC service implementation type (e.g., DetectorService)
 *
 * @grpc
 * @threadsafe Thread-safe for concurrent requests
 *
 * Example usage:
 * @code
 * class MyClient : public RpcClient<MyService> {
 *   public:
 *     explicit MyClient(std::string_view address) : RpcClient{address} {}
 *
 *     grpc::Status CallMethod(const Request& req, Response* res) {
 *       return DoRequest(&MyService::Stub::CallMethod, req, res);
 *     }
 * };
 * @endcode
 */
template <class Impl>
class RpcClient {
 public:
  /**
   * @brief Construct a new RPC Client with specified remote address and timeout
   *
   * @param remote Server address in format "host:port" (e.g.,
   * "localhost:50051")
   * @param timeout Request timeout in milliseconds (default: 10000ms)
   *
   * @grpc Creates insecure gRPC channel for communication
   */
  explicit RpcClient(std::string_view remote, std::size_t timeout = 10000)
      : channel_{CreateChannel(remote.data(),
                               grpc::InsecureChannelCredentials())},
        service_stub_{std::make_unique<typename Impl::Stub>(channel_)} {
    timeout > 0 ? timeout_ = timeout : timeout_ = 100;
  }

  /**
   * @brief Get the underlying gRPC channel
   *
   * @return std::shared_ptr<grpc::Channel> The gRPC channel for this client
   */
  std::shared_ptr<grpc::Channel> Channel() { return channel_; }

  /**
   * @brief Virtual destructor for proper cleanup of derived classes
   */
  virtual ~RpcClient() = default;

 protected:
  /**
   * @brief Execute a gRPC request with timeout and error handling
   *
   * Template method that handles the common pattern of gRPC requests including
   * context setup, deadline management, and method invocation.
   *
   * @tparam Func gRPC method function type
   * @tparam Req Request message type
   * @tparam Res Response message type
   * @param func gRPC service method to invoke
   * @param req Request message
   * @param res Response message pointer
   * @return grpc::Status Result of the gRPC call
   */
  template <typename Func, typename Req, typename Res>
  grpc::Status DoRequest(Func&& func, const Req& req, Res* res) {
    grpc::ClientContext ctx;
    SetRequestDeadline(&ctx);

    return std::invoke(std::forward<Func>(func), *service_stub_, &ctx, req,
                       res);
  }

  /**
   * @brief Create a gRPC client writer for streaming requests
   *
   * @tparam Req Request message type
   * @tparam Func gRPC streaming method function type
   * @tparam Res Response message type
   * @param func gRPC streaming method to invoke
   * @param ctx Client context for the stream
   * @param res Response message pointer
   * @return std::unique_ptr<grpc::ClientWriter<Req>> Writer for streaming
   * requests
   */
  template <typename Req, typename Func, typename Res>
  std::unique_ptr<grpc::ClientWriter<Req>> CreateWriter(
      Func&& func, grpc::ClientContext* ctx, Res* res) {
    return std::invoke(std::forward<Func>(func), *service_stub_, ctx, res);
  }

 private:
  std::shared_ptr<grpc::Channel> channel_;  ///< gRPC communication channel
  std::unique_ptr<typename Impl::Stub>
      service_stub_;     ///< Service-specific gRPC stub
  std::size_t timeout_;  ///< Request timeout in milliseconds

  /**
   * @brief Set request deadline based on configured timeout
   *
   * @param ctx Client context to configure (can be nullptr)
   */
  void SetRequestDeadline(grpc::ClientContext* ctx) noexcept {
    if (ctx == nullptr) return;

    using std::chrono::milliseconds;
    using std::chrono::system_clock;
    auto deadline = system_clock::now() + milliseconds{timeout_};
    ctx->set_deadline(deadline);
  }
};

}  // namespace aa::client
