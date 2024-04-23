#include <iostream>
#include <string>

#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

namespace hw = helloworld;

class GreeterClient {
public:
  GreeterClient(std::shared_ptr<grpc::Channel> channel) : stub_(hw::Greeter::NewStub(channel)) {
  }

  std::string SayHello(const std::string& user) {
    auto context = grpc::ClientContext();
    context.AddMetadata("custom-header", "Custom Value");
    char bytes[8] = {'\0', '\1', '\2', '\3', '\4', '\5', '\6', '\7'};
    context.AddMetadata("custom-bin", std::string(bytes, 8));

    auto request = hw::HelloRequest();
    request.set_name(user);
    auto reply = hw::HelloReply();
    auto status = stub_->SayHello(&context, request, &reply);
    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC failed";
    }

    std::cout << "Client received initial metadata from server: "
      << context.GetServerInitialMetadata().find("custom-server-metadata")->second
      << std::endl;
    std::cout << "Client received trailing metadata from server: "
      << context.GetServerTrailingMetadata().find("custom-trailing-metadata")->second
      << std::endl;
    return reply.message();
  }

private:
  std::unique_ptr<hw::Greeter::Stub> stub_;
};

int main() {
  auto greeter_client = GreeterClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  auto user = "world";
  auto reply = greeter_client.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;
  
  return 0;
}
