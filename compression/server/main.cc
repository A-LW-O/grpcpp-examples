#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

namespace hw = helloworld;

class GreeterServiceImpl final : public hw::Greeter::Service {
  grpc::Status SayHello(grpc::ServerContext* context, const hw::HelloRequest* request, hw::HelloReply* reply) override {
    auto prefix = "Hello ";
    reply->set_message(prefix + request->name());

    return grpc::Status::OK;
  }
};

void RunServer() {
  auto builder = grpc::ServerBuilder();
  builder.SetDefaultCompressionAlgorithm(GRPC_COMPRESS_GZIP);
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

