## Requirements

- C++20 support (GCC-10, Clang-10)
- CMake 3.13
- Python 3

## Building

```bash
sudo apt-get install curl unzip tar libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev
git submodule update --init --recursive
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install --feature-flags=manifests
printf "configs/config_gemm_blocked_32x32x32_simple.json" > ./conf2/config_path.txt
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --target conf2
cmake --build . --target visualizer_main
```

## Usage

```bash
cd build
./conf2/src/conf2 generate -o ./visualizer
cd visualizer
./visualizer_main
```
