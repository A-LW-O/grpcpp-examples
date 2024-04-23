#!/bin/sh

# ## Description
# Assuming the `conanfile` does not have a `layout` field.

#set -e
#set -x

script_dir=$(dirname "$(readlink -f "$0")")
cd $script_dir

# ## Build
rm -rf build

does_exist_conanfile=false
[ -f "conanfile.txt" -o -f "conanfile.py" ] && does_exist_conanfile=true

conan_output_folder="generators"

# Ref: [Building for multiple configurations: Release, Debug, Static and Shared](Building for multiple configurations: Release, Debug, Static and Shared)
#[ $does_exist_conanfile = true ] && conan install . --build=missing --output-folder=build/$conan_output_folder -s "build_type=Debug"

# Ref: [Build the project with Debug while dependencies with Release](https://github.com/conan-io/conan/issues/13478#issuecomment-1475389368)
[ $does_exist_conanfile = true ] && conan install . --build=missing --output-folder=build/$conan_output_folder -s "&:build_type=Debug" -s "build_type=Release"

[ ! -d build ] && mkdir build

cd build

[ $does_exist_conanfile = true ] && . $conan_output_folder/conanbuild.sh

# ## Generate build system
#compiler="env CC=/usr/bin/clang CXX=/usr/bin/clang++"
compiler="env CC=/usr/bin/gcc CXX=/usr/bin/g++"
cmake_option=("-G Ninja")
if [ $does_exist_conanfile = true ]; then
  cmake_option+=("-DCMAKE_TOOLCHAIN_FILE=$conan_output_folder/conan_toolchain.cmake")
  cmake_option+=("-DCMAKE_BUILD_TYPE=Debug")
fi
$compiler cmake .. ${cmake_option[*]}

# ## Compile/Link
cmake --build .

[ $does_exist_conanfile = true ] && . $conan_output_folder/deactivate_conanbuild.sh

cd ..
