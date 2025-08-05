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

template <class Impl>
class RpcServerFromThis {
 public:
  constexpr explicit RpcServerFromThis(std::string_view address)
      : address_{address}, service_impl_{static_cast<Impl&>(*this)} {
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  }

  void Build() {
    grpc::ServerBuilder builder;

    builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_impl_);

    server_ = std::unique_ptr<grpc::Server>(builder.BuildAndStart());  // NOLINT
  }

  void Wait() {
    AA_LOG_INFO("server started listening on " << address_);
    server_->Wait();
  }

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
  std::string address_;
  Impl& service_impl_;
  std::unique_ptr<grpc::Server> server_;
};

template <typename Signature>
class Observer {
 public:
  constexpr Observer() = default;
  Observer(const Observer&) = delete;
  Observer& operator=(const Observer&) = delete;

 private:
  template <typename Observers>
  friend class Observable;

  using Method = std::function<Signature>;
  Method method_;
};

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
