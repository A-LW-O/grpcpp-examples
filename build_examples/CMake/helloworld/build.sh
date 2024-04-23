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

[ $does_exist_conanfile = true ] && conan install . --build=missing --output-folder=build/$conan_output_folder

[ ! -d build ] && mkdir build

cd build

[ $does_exist_conanfile = true ] && . $conan_output_folder/conanbuild.sh

# ## Generate build system
#compiler="env CC=/usr/bin/clang CXX=/usr/bin/clang++"
compiler="env CC=/usr/bin/gcc CXX=/usr/bin/g++"
cmake_option=("-G Ninja")
if [ $does_exist_conanfile = true ]; then
  cmake_option+=("-DCMAKE_TOOLCHAIN_FILE=$conan_output_folder/conan_toolchain.cmake")
  cmake_option+=("-DCMAKE_BUILD_TYPE=Release")
fi
$compiler cmake .. ${cmake_option[*]}

# ## Compile/Link
cmake --build .

[ $does_exist_conanfile = true ] && . $conan_output_folder/deactivate_conanbuild.sh

cd ..
