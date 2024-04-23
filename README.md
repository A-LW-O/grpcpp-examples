# grpcpp examples

## Purpose of the Project

Inspired by the [ttroy50/cmake-examples](https://github.com/ttroy50/cmake-examples) project, the purpose of this project is to provide you with a basic understanding of how to use `grpcpp`. You don't need to spend too much time learning things other than `grpcpp` (such as how to use `grpcpp` in a project).

## Learn how to build a example

[helloworld_debug](build_examples/CMake/helloworld_debug)

## Example List

[Core](https://grpc.io/docs/what-is-grpc/core-concepts):

-   [helloworld](helloworld)
-   [async](async)
-   [callback](callback)
-   [route](route):
    -   Unary RPC
    -   Server streaming RPC
    -   Client streaming RPC
    -   Bidirectional streaming RPC
-   [route_async](route_async)
-   [cancellation](cancellation): cancellation, callback bidistream
-   [deadlines](deadlines)
-   [metadata](metadata)

Auth:

-   [auth_ssl](auth_ssl)
-   [auth_ssl_token](auth_ssl_token)

Others:

-   [keepalive](keepalive)
-   [reflection](reflection)
-   [compression](compression)
-   [interceptor](interceptor)

## How to contribute

See [update_project_files](update_project_files.sh)
