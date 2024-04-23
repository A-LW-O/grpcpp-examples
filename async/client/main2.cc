#include <iostream>
#include <string>
#include <thread>
#include <absl/log/check.h>

#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

namespace hw = helloworld;

class GreeterClient {
  struct AsyncClientCall {
    hw::HelloReply reply;
    grpc::ClientContext context;
    grpc::Status status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<hw::HelloReply>> response_reader;
  };

public:
  GreeterClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(hw::Greeter::NewStub(channel)) {
  }

  void SayHello(const std::string& user) {
    auto request = hw::HelloRequest();
    request.set_name(user);

    auto call = new AsyncClientCall();
    call->response_reader = stub_->PrepareAsyncSayHello(&call->context, request, &cq_);
    call->response_reader->StartCall();
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
  }

  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;
    while (cq_.Next(&got_tag, &ok)) {
      AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

      CHECK(ok);

      if (call->status.ok()) {
        std::cout << "Greeter received: " << call->reply.message() << std::endl;
      } else {
        std::cout << "RPC failed: " << call->status.error_code() << ": " << call->status.error_message() << std::endl;
      }

      delete call;
    }
  }

private:
  std::unique_ptr<hw::Greeter::Stub> stub_;
  grpc::CompletionQueue cq_;
};

int main() {
  auto greeter_client = GreeterClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  auto thr = std::thread(&GreeterClient::AsyncCompleteRpc, &greeter_client);

  for (int i = 0; i < 100; i++) {
    auto user = "world" + std::to_string(i);
    greeter_client.SayHello(user);
  }

  thr.join();
  
  return 0;
}
