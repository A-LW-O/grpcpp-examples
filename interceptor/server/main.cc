#include <grpcpp/support/server_interceptor.h>
#include <grpcpp/grpcpp.h>

#include "keyvaluestore.grpc.pb.h"

namespace grpcexp = grpc::experimental;
namespace kvs = keyvaluestore;

class LoggingInterceptor : public grpcexp::Interceptor {
public:
  void Intercept(grpcexp::InterceptorBatchMethods* methods) override {
    if (methods->QueryInterceptionHookPoint(grpcexp::InterceptionHookPoints::POST_RECV_INITIAL_METADATA)) {
      std::cout << "Got a new streaming RPC" << std::endl;
    }
    methods->Proceed();
  }
};

class LoggingInterceptorFactory : public grpcexp::ServerInterceptorFactoryInterface {
 public:
   grpcexp::Interceptor* CreateServerInterceptor(grpcexp::ServerRpcInfo* info) override {
    return new LoggingInterceptor();
  }
};

struct kv_pair {
  const char* key;
  const char* value;
};

static const kv_pair kvs_map[] = {
    {"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"},
    {"key4", "value4"}, {"key5", "value5"},
};

const char* get_value_from_map(const char* key) {
  for (size_t i = 0; i < sizeof(kvs_map) / sizeof(kv_pair); ++i) {
    if (strcmp(key, kvs_map[i].key) == 0) {
      return kvs_map[i].value;
    }
  }
  return "";
}

class KeyValueStoreServiceImpl final : public kvs::KeyValueStore::CallbackService {
  grpc::ServerBidiReactor<kvs::Request, kvs::Response>* GetValues(
      grpc::CallbackServerContext* context) override {
    class Reactor : public grpc::ServerBidiReactor<kvs::Request, kvs::Response> {
     public:
      explicit Reactor() { StartRead(&request_); }

      void OnReadDone(bool ok) override {
        if (!ok) {
          return Finish(grpc::Status::CANCELLED);
        }
        response_.set_value(get_value_from_map(request_.key().c_str()));
        StartWrite(&response_);
      }

      void OnWriteDone(bool ok) override {
        if (!ok) {
          return Finish(grpc::Status::CANCELLED);
        }
        StartRead(&request_);
      }

      void OnDone() override { delete this; }

     private:
      kvs::Request request_;
      kvs::Response response_;
    };

    return new Reactor();
  }
};

void RunServer() {
  auto builder = grpc::ServerBuilder();
  auto server_address = "0.0.0.0:50051";
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  auto service = KeyValueStoreServiceImpl();
  builder.RegisterService(&service);

  auto creators = std::vector<std::unique_ptr<grpcexp::ServerInterceptorFactoryInterface>>();
  creators.push_back(std::unique_ptr<grpcexp::ServerInterceptorFactoryInterface>(
      new LoggingInterceptorFactory()));
  builder.experimental().SetInterceptorCreators(std::move(creators));

  auto server = builder.BuildAndStart();
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
