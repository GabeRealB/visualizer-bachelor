## Requirements

- C++20 support (GCC-10, Clang-10)
- CMake 3.13
- Python 3

## Building

```bash
git submodule update --init --recurse
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install --feature-flags=manifests
printf "configs/config_gemm_blocked.json" > ./conf2/config_path.txt
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
```

## Usage

```bash
cd build/visualizer
../conf2/src/conf2 generate
./visualizer_main
```
