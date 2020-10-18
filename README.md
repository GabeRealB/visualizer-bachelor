## Requirements

- C++20 support (GCC-10, Clang-10)
- CMake 3.13
- Python 3

## Building

```bash
git submodule update --init --recurse
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install --feature-flags=manifests
echo "gemm_mdh.json" > ./mdh2vis/mdh_path.txt
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
```