#include <absl/log/check.h>

#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

using namespace std::string_literals;
namespace hw = helloworld;

template<class Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
  return static_cast<std::underlying_type_t<Enum>>(e);
}

class ServerImpl final {
  class CallData {
    enum class CallStatus : uint8_t { CREATE, PROCESS, FINISH };

  public:
    CallData(hw::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq)
      : service_(service), cq_(cq), responder_(&context_), status_(CallStatus::CREATE) {
        Proceed();
    }
  
    void Proceed() {
      if (status_ == CallStatus::CREATE) {
        status_ = CallStatus::PROCESS;
        service_->RequestSayHello(&context_, &request_, &responder_, cq_, cq_, this);
      } else if (status_ == CallStatus::PROCESS) {
        new CallData(service_, cq_);

        auto prefix = "Hello "s;
        reply_.set_message(prefix + request_.name());

        status_ = CallStatus::FINISH;
        responder_.Finish(reply_, grpc::Status::OK, this);
      } else {
        CHECK_EQ(to_underlying(status_), to_underlying(CallStatus::FINISH));
        delete this;
      }
    }

  private:
    hw::Greeter::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;

    grpc::ServerContext context_;
    hw::HelloRequest request_;
    hw::HelloReply reply_;
    
    grpc::ServerAsyncResponseWriter<hw::HelloReply> responder_;

    CallStatus status_;
  };

public:
  ~ServerImpl() {
    server_->Shutdown();
    cq_->Shutdown();
  }

  void Run() {
    auto builder = grpc::ServerBuilder();
    auto server_address = "0.0.0.0:50051";
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();

    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    handleRpcs();
  }

  void handleRpcs() {
    new CallData(&service_, cq_.get());

    void* tag;
    bool ok;
    while (true) {
      CHECK(cq_->Next(&tag, &ok));
      CHECK(ok);
      static_cast<CallData*>(tag)->Proceed();
    }
  }

private:
  std::unique_ptr<grpc::Server> server_;
  std::unique_ptr<grpc::ServerCompletionQueue> cq_;
  hw::Greeter::AsyncService service_;
};

int main() {
  auto server = ServerImpl();
  server.Run();
  return 0;
}
