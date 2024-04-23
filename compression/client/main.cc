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
    auto request = hw::HelloRequest();
    request.set_name(user);
    auto reply = hw::HelloReply();
    auto status = stub_->SayHello(&context, request, &reply);
    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC failed";
    } else {
      return reply.message();
    }
  }

private:
  std::unique_ptr<hw::Greeter::Stub> stub_;
};

int main() {
  auto args = grpc::ChannelArguments();
  args.SetCompressionAlgorithm(GRPC_COMPRESS_GZIP);
  auto greeter_client = GreeterClient(grpc::CreateCustomChannel("localhost:50051", grpc::InsecureChannelCredentials(), args));
  auto user = "world world world world";
  auto reply = greeter_client.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;
  
  return 0;
}
