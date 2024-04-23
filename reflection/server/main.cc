#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "helloworld.grpc.pb.h"

class GreeterServiceImpl final : public helloworld::Greeter::CallbackService {
  grpc::ServerUnaryReactor* SayHello(grpc::CallbackServerContext* context, const helloworld::HelloRequest* request, helloworld::HelloReply* reply) override {
    auto prefix = "Hello ";
    reply->set_message(prefix + request->name());

    auto reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status::OK);

    return reactor;
  }
};

void RunServer() {
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

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
