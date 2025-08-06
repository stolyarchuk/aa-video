#pragma once
#include <string_view>

#include <grpcpp/grpcpp.h>

namespace aa::client {

template <class Impl>
class RpcClient {
 public:
  explicit RpcClient(std::string_view remote, std::size_t timeout = 10000)
      : channel_{CreateChannel(remote.data(),
                               grpc::InsecureChannelCredentials())},
        service_stub_{std::make_unique<typename Impl::Stub>(channel_)} {
    timeout > 0 ? timeout_ = timeout : timeout_ = 100;
  }

  std::shared_ptr<grpc::Channel> Channel() { return channel_; }

  virtual ~RpcClient() = default;

 protected:
  template <typename Func, typename Req, typename Res>
  grpc::Status DoRequest(Func&& func, const Req& req, Res* res) {
    grpc::ClientContext ctx;
    SetRequestDeadline(&ctx);

    return std::invoke(std::forward<Func>(func), *service_stub_, &ctx, req,
                       res);
  }

  template <typename Req, typename Func, typename Res>
  std::unique_ptr<grpc::ClientWriter<Req>> CreateWriter(
      Func&& func, grpc::ClientContext* ctx, Res* res) {
    return std::invoke(std::forward<Func>(func), *service_stub_, ctx, res);
  }

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<typename Impl::Stub> service_stub_;
  std::size_t timeout_;

  void SetRequestDeadline(grpc::ClientContext* ctx) noexcept {
    if (ctx == nullptr) return;

    using std::chrono::milliseconds;
    using std::chrono::system_clock;
    auto deadline = system_clock::now() + milliseconds{timeout_};
    ctx->set_deadline(deadline);
  }
};

}  // namespace aa::client
