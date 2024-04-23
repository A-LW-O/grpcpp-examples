#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

namespace hw = helloworld;
using namespace std::string_literals;

class GreeterServiceImpl final : public hw::Greeter::Service {
  grpc::Status SayHello(grpc::ServerContext* context, const hw::HelloRequest* request, hw::HelloReply* reply) override {
    std::cout << "Client metadata: " << std::endl;
    auto client_metadata = context->client_metadata();
    for (auto iter = client_metadata.begin(); iter != client_metadata.end(); iter++) {
      std::cout << "Header key: " << iter->first << ", value: ";
      size_t isbin = iter->first.find("-bin");
      if ((isbin != std::string::npos) && (isbin + 4 == iter->first.size())) {
        std::cout << std::hex;
        for (auto c : iter->second) {
          std::cout << static_cast<unsigned int>(c);
        }
        std::cout << std::dec;
      } else {
        std::cout << iter->second;
      }
      std::cout << std::endl;
    }

    context->AddInitialMetadata("custom-server-metadata", "initial metadata value");
    context->AddTrailingMetadata("custom-trailing-metadata", "trailing metadata value");

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

