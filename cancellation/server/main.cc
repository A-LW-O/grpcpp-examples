#include <absl/strings/str_cat.h>

#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

namespace hw = helloworld;

class KeyValueStoreImpl final : public hw::Greeter::CallbackService {
  grpc::ServerBidiReactor<hw::HelloRequest, hw::HelloReply> *
  SayHelloBidiStream(grpc::CallbackServerContext *context) override {
    class Reactor : public grpc::ServerBidiReactor<hw::HelloRequest,
                                                   hw::HelloReply> {
    public:
      explicit Reactor() { StartRead(&request_); }

      void OnReadDone(bool ok) override {
        if (!ok) {
          std::cout << "OnReadDone cancelled!" << std::endl;
          return Finish(grpc::Status::CANCELLED);
        }

        response_.set_message(absl::StrCat(request_.name(), " Ack"));
        StartWrite(&response_);
      }

      void OnWriteDone(bool ok) override {
        if (!ok) {
          std::cout << "OnWriteDone cancelled!" << std::endl;
          return Finish(grpc::Status::CANCELLED);
        }

        StartWrite(&response_);
      }

      void OnDone() override { delete this; }

    private:
      hw::HelloRequest request_;
      hw::HelloReply response_;
    };

    return new Reactor();
  }
};

void RunServer() {
  auto builder = grpc::ServerBuilder();
  auto server_address = "0.0.0.0:50051";
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  auto service = KeyValueStoreImpl();
  builder.RegisterService(&service);

  auto server = builder.BuildAndStart();
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main() { RunServer(); }
