rm -rf build 2> /dev/null
conan install . --build=missing
pushd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
make -j
popd
