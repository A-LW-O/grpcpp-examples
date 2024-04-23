find_package(protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

if(protobuf_VERBOSE)
  message(STATUS "Using Protocol Buffers ${protobuf_VERSION}")
endif()

file(GLOB protos "*.proto")
set(target proto)
add_library(${target} SHARED
  ${protos}
)

target_include_directories(${target}
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${protobuf_INCLUDE_DIR}
)

target_link_libraries(${target}
  protobuf::protobuf
  grpc::grpc
)

protobuf_generate(TARGET ${target})
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET ${target} LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

set(PROTO_LIB_NAME "${target}" CACHE STRING "The name of app proto library")
