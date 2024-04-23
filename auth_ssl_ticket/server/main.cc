#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>

#include "helloworld.grpc.pb.h"
#include "helper.h"

namespace hw = helloworld;
using namespace std::string_literals;

class GreeterServiceImpl final : public hw::Greeter::Service {
  grpc::Status SayHello(grpc::ServerContext* context, const hw::HelloRequest* request, hw::HelloReply* reply) override {
    auto metadata = context->client_metadata();
    auto ticket_it = metadata.find("x-custom-auth-ticket");

    if (ticket_it == metadata.end() || ticket_it->second != "super-secret-ticket") {
      return grpc::Status(grpc::StatusCode::PERMISSION_DENIED, "Invalid ticket");
    }

    auto prefix = "Hello "s;
    reply->set_message(prefix + request->name());

    return grpc::Status::OK;
  }
};

void RunServer() {
  grpc::SslServerCredentialsOptions ssl_options;
  ssl_options.pem_root_certs = helper::ReadFile("ca/ca/ca.crt");
  ssl_options.pem_key_cert_pairs.push_back({helper::ReadFile("ca/server/server.key"), helper::ReadFile("ca/server/server.crt")});
  auto server_creds = grpc::SslServerCredentials(ssl_options);

  auto builder = grpc::ServerBuilder();
  auto server_address = "0.0.0.0:50051";
  builder.AddListeningPort("0.0.0.0:50051", server_creds);

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
