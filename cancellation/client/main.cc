#include <iostream>
#include <mutex>
#include <condition_variable>
#include <absl/strings/str_cat.h>

#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

namespace hw = helloworld;

class KeyValueStoreClient final : public grpc::ClientBidiReactor<hw::HelloRequest, hw::HelloReply> {
public:
  KeyValueStoreClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(hw::Greeter::NewStub(channel)) {
      stub_->async()->SayHelloBidiStream(&context_, this);
      request_.set_name("Begin");
      StartWrite(&request_);
      StartCall();
  }

  void OnReadDone(bool ok) override {
    if (!ok) {
      return;
    }

    std::cout << request_.name() << ": " << response_.message() << std::endl;
    if (++counter_ < 10) {
      request_.set_name(absl::StrCat("Count ", counter_));
      StartWrite(&request_);
    } else {
      context_.TryCancel();
    }
    
  }

  void OnWriteDone(bool ok) override {
    if (!ok) {
      return;
    }

    StartRead(&response_);
  }

  void OnDone(const grpc::Status& status) override {
    if (!status.ok()) {
      if (status.error_code() == grpc::StatusCode::CANCELLED) {
        std::cout << "RPC Cancelled!" << std::endl;
      } else {
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      }
      return;
    }

    auto lock = std::unique_lock(mtx_);
    done_ = false;
    lock.unlock();
    cv_.notify_all();
  }

  void Await() {
    auto lock = std::unique_lock(mtx_);
    while (!done_) {
      cv_.wait(lock);
    }
  }

private:
  std::unique_ptr<hw::Greeter::Stub> stub_;
  grpc::ClientContext context_;
  hw::HelloRequest request_;
  hw::HelloReply response_;
  size_t counter_ = 0;
  bool done_ = false;
  std::mutex mtx_;
  std::condition_variable cv_;
};

int main() {
  auto client = KeyValueStoreClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  client.Await();
}
