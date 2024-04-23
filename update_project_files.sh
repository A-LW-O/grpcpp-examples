#!/bin/sh

#set -e
#set -x

script_dir=$(dirname "$(readlink -f "$0")")
cd $script_dir

build_example_project="build_examples/CMake/helloworld_debug"

project_dirs=(
  "helloworld"
  "async"
  "callback"
  "route"
  "route_async"
  "cancellation"
  "deadlines"
  "metadata"
  "auth_ssl"
  "auth_ssl_ticket"
  "keepalive"
  "reflection"
  "compression"
  "interceptor"
)

set -x
for dst_dir in "${project_dirs[@]}"; do
  cp -f "${build_example_project}/build.sh" "${dst_dir}/"
  cp -f "${build_example_project}/run.sh" "${dst_dir}/"
  cp -f "${build_example_project}/compile_flags.txt" "${dst_dir}/"

  cp -f "${build_example_project}/CMakeLists.txt" "${dst_dir}/"
  cp -f "${build_example_project}/proto/CMakeLists.txt" "${dst_dir}/proto/"
  cp -f "${build_example_project}/server/CMakeLists.txt" "${dst_dir}/server/"
  cp -f "${build_example_project}/client/CMakeLists.txt" "${dst_dir}/client/"
  cp -f "${build_example_project}/helper/CMakeLists.txt" "${dst_dir}/helper/"
done
set +x

# ## auth_ssl
src_project_dirs="auth_ssl"
dst_project_dirs=(
  "auth_ssl_ticket"
)

for dst_dir in "${dst_project_dirs[@]}"; do
  cp -f "${src_project_dirs}/build_ca.sh" "${dst_dir}/"
  cp -f "${src_project_dirs}/.gitignore" "${dst_dir}/"
done
