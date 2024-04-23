#include <map>
#include <absl/log/check.h>

#include <grpcpp/support/client_interceptor.h>

#include "keyvaluestore.grpc.pb.h"

class CachingInterceptor : public grpc::experimental::Interceptor {
 public:
  CachingInterceptor(grpc::experimental::ClientRpcInfo* info) {}

  void Intercept(
      ::grpc::experimental::InterceptorBatchMethods* methods) override {
    bool hijack = false;
    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::
                PRE_SEND_INITIAL_METADATA)) {
      // Hijack all calls
      hijack = true;
      // Create a stream on which this interceptor can make requests
      stub_ = keyvaluestore::KeyValueStore::NewStub(
          methods->GetInterceptedChannel());
      stream_ = stub_->GetValues(&context_);
    }
    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::PRE_SEND_MESSAGE)) {
      // We know that clients perform a Read and a Write in a loop, so we don't
      // need to maintain a list of the responses.
      std::string requested_key;
      const keyvaluestore::Request* req_msg =
          static_cast<const keyvaluestore::Request*>(methods->GetSendMessage());
      if (req_msg != nullptr) {
        requested_key = req_msg->key();
      } else {
        // The non-serialized form would not be available in certain scenarios,
        // so add a fallback
        keyvaluestore::Request req_msg;
        auto* buffer = methods->GetSerializedSendMessage();
        auto copied_buffer = *buffer;
        CHECK(grpc::SerializationTraits<keyvaluestore::Request>::Deserialize(
                  &copied_buffer, &req_msg)
                  .ok());
        requested_key = req_msg.key();
      }

      // Check if the key is present in the map
      auto search = cached_map_.find(requested_key);
      if (search != cached_map_.end()) {
        std::cout << requested_key << " found in map" << std::endl;
        response_ = search->second;
      } else {
        std::cout << requested_key << " not found in cache" << std::endl;
        // Key was not found in the cache, so make a request
        keyvaluestore::Request req;
        req.set_key(requested_key);
        stream_->Write(req);
        keyvaluestore::Response resp;
        stream_->Read(&resp);
        response_ = resp.value();
        // Insert the pair in the cache for future requests
        cached_map_.insert({requested_key, response_});
      }
    }
    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::PRE_SEND_CLOSE)) {
      stream_->WritesDone();
    }
    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::PRE_RECV_MESSAGE)) {
      keyvaluestore::Response* resp =
          static_cast<keyvaluestore::Response*>(methods->GetRecvMessage());
      resp->set_value(response_);
    }
    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::PRE_RECV_STATUS)) {
      auto* status = methods->GetRecvStatus();
      *status = grpc::Status::OK;
    }
    // One of Hijack or Proceed always needs to be called to make progress.
    if (hijack) {
      // Hijack is called only once when PRE_SEND_INITIAL_METADATA is present in
      // the hook points
      methods->Hijack();
    } else {
      // Proceed is an indicator that the interceptor is done intercepting the
      // batch.
      methods->Proceed();
    }
  }

 private:
  grpc::ClientContext context_;
  std::unique_ptr<keyvaluestore::KeyValueStore::Stub> stub_;
  std::unique_ptr<
      grpc::ClientReaderWriter<keyvaluestore::Request, keyvaluestore::Response>>
      stream_;
  std::map<std::string, std::string> cached_map_;
  std::string response_;
};

class CachingInterceptorFactory
    : public grpc::experimental::ClientInterceptorFactoryInterface {
 public:
  grpc::experimental::Interceptor* CreateClientInterceptor(
      grpc::experimental::ClientRpcInfo* info) override {
    return new CachingInterceptor(info);
  }
};
