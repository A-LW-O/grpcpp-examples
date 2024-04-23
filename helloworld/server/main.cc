#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

namespace hw = helloworld;
using namespace std::string_literals;

class GreeterServiceImpl final : public hw::Greeter::Service {
  grpc::Status SayHello(grpc::ServerContext* context, const hw::HelloRequest* request, hw::HelloReply* reply) override {
    auto prefix = "Hello "s;
    reply->set_message(prefix + request->name());

    return grpc::Status::OK;
  }
};

void RunServer() {
  auto builder = grpc::ServerBuilder();
  auto server_address = "0.0.0.0:50051";
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  auto service = GreeterServiceImpl();
  builder.RegisterService(&service);

  auto server = builder.BuildAndStart();
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
