#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>

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

    auto cv = std::condition_variable();
    auto mtx = std::mutex();
    bool done =false;
    auto status = grpc::Status();
    stub_->async()->SayHello(&context, &request, &reply, 
        [&cv, &mtx, &done, &status](grpc::Status s) {
          status = std::move(s);
          auto lock = std::lock_guard(mtx);
          done = true;
          cv.notify_one();
        });

    auto lock = std::unique_lock(mtx);
    while (!done) {
      cv.wait(lock);
    }
    lock.unlock();

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
  auto greeter_client = GreeterClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  auto user = "world";
  auto reply = greeter_client.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;
  
  return 0;
}
