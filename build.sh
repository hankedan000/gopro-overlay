rm -rf build 2> /dev/null
conan install . --output-folder=build --build=missing
pushd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
popd
