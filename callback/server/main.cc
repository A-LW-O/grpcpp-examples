#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

namespace hw = helloworld;
using namespace std::string_literals;

class GreeterServiceImpl final : public hw::Greeter::CallbackService {
  grpc::ServerUnaryReactor* SayHello(grpc::CallbackServerContext* context, const hw::HelloRequest* request, hw::HelloReply* reply) override {
    auto prefix = "Hello "s;
    reply->set_message(prefix + request->name());

    auto reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status::OK);

    return reactor;
  }
};

void RunServer() {
  auto builder = grpc::ServerBuilder();
  auto server_address = "localhost:50051";
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
