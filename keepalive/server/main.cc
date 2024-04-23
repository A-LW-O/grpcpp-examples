#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "helloworld.grpc.pb.h"

namespace hw = helloworld;

class GreeterServiceImpl final : public hw::Greeter::CallbackService {
  grpc::ServerUnaryReactor* SayHello(grpc::CallbackServerContext* context, const hw::HelloRequest* request, hw::HelloReply* reply) override {
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

  builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS,
                             10 * 60 * 1000 /*10 min*/);
  builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS,
                             20 * 1000 /*20 sec*/);
  builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
  builder.AddChannelArgument(
      GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS,
      10 * 1000 /*10 sec*/);

  auto server = builder.BuildAndStart();
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
