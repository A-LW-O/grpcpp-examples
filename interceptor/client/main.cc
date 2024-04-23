#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "keyvaluestore.grpc.pb.h"
#include "caching_interceptor.h"

namespace kvs = keyvaluestore;

class KeyValueStoreClient : public grpc::ClientBidiReactor<kvs::Request, kvs::Response> {
 public:
  KeyValueStoreClient(std::shared_ptr<grpc::Channel> channel,
                      std::vector<std::string> keys)
      : stub_(kvs::KeyValueStore::NewStub(channel)), keys_(std::move(keys)) {
    stub_->async()->GetValues(&context_, this);
    assert(!keys_.empty());
    request_.set_key(keys_[0]);
    StartWrite(&request_);
    StartCall();
  }

  void OnReadDone(bool ok) override {
    if (ok) {
      std::cout << request_.key() << " : " << response_.value() << std::endl;
      if (++counter_ < keys_.size()) {
        request_.set_key(keys_[counter_]);
        StartWrite(&request_);
      } else {
        StartWritesDone();
      }
    }
  }

  void OnWriteDone(bool ok) override {
    if (ok) {
      StartRead(&response_);
    }
  }

  void OnDone(const grpc::Status& status) override {
    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      std::cout << "RPC failed";
    }
    std::unique_lock<std::mutex> l(mu_);
    done_ = true;
    cv_.notify_all();
  }

  void Await() {
    std::unique_lock<std::mutex> l(mu_);
    while (!done_) {
      cv_.wait(l);
    }
  }

 private:
  std::unique_ptr<kvs::KeyValueStore::Stub> stub_;
  std::vector<std::string> keys_;
  size_t counter_ = 0;
  grpc::ClientContext context_;
  bool done_ = false;
  kvs::Request request_;
  kvs::Response response_;
  std::mutex mu_;
  std::condition_variable cv_;
};

int main() {
  grpc::ChannelArguments args;
  std::vector<
      std::unique_ptr<grpc::experimental::ClientInterceptorFactoryInterface>>
      interceptor_creators;
  interceptor_creators.push_back(std::make_unique<CachingInterceptorFactory>());
  auto channel = grpc::experimental::CreateCustomChannelWithInterceptors(
      "localhost:50051", grpc::InsecureChannelCredentials(), args,
      std::move(interceptor_creators));
  std::vector<std::string> keys = {"key1", "key2", "key3", "key4",
                                   "key5", "key1", "key2", "key4"};
  KeyValueStoreClient client(channel, keys);
  client.Await();
  return 0;
}
