#include <iostream>
#include <string>

#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"
#include "helper.h"

namespace hw = helloworld;

class MyCustomAuthenticator : public grpc::MetadataCredentialsPlugin {
 public:
  MyCustomAuthenticator(const grpc::string& ticket) : ticket_(ticket) {}

  grpc::Status GetMetadata(
      grpc::string_ref service_url, grpc::string_ref method_name,
      const grpc::AuthContext& channel_auth_context,
      std::multimap<grpc::string, grpc::string>* metadata) override {
    metadata->insert(std::make_pair("x-custom-auth-ticket", ticket_));
    return grpc::Status::OK;
  }

 private:
  grpc::string ticket_;
};

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
  auto channel_creds = grpc::SslCredentials(grpc::SslCredentialsOptions{
                                          .pem_root_certs = helper::ReadFile("ca/ca/ca.crt"),
                                          .pem_private_key = helper::ReadFile("ca/client/client.key"),
                                          .pem_cert_chain = helper::ReadFile("ca/client/client.crt"),
                                          });
  auto call_creds = grpc::MetadataCredentialsFromPlugin(
      std::unique_ptr<grpc::MetadataCredentialsPlugin>(
          new MyCustomAuthenticator("super-secret-ticket")));

  auto channel = grpc::CreateChannel("localhost:50051", 
    grpc::CompositeChannelCredentials(channel_creds, call_creds));
  auto greeter_client = GreeterClient(channel);
  auto user = "world";
  auto reply = greeter_client.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;
  
  return 0;
}
