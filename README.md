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
pip install click commentjson
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

## Controls

### General

| Key          | Description                     |
| ------------ | ------------------------------- |
| R            | Reset                           |
| P            | Pause                           |
| Y            | Run x 1                         |
| X            | Run x 100                       |
| C            | Run x 1000                      |
| V            | Run x 10000                     |
| B            | Run x 100000                    |
| F            | Switch free/fixed               |
| G            | Switch orthographic/perspective |
| M            | Open materials menu             |
| Alt          | Show/Hide mouse                 |
| Tab          | Switch active window            |
| Scroll wheel | Change window size              |

### Orthographic (fixed)

| Key | Description |
| --- | ----------- |
| Q   | Zoom out    |
| E   | Zoom in     |

### Orthographic (free)

| Key | Description |
| --- | ----------- |
| W   | Move up     |
| A   | Move left   |
| S   | Move down   |
| D   | Move Right  |
| Q   | Zoom out    |
| E   | Zoom in     |

### Perspective (fixed)

| Key | Description  |
| --- | ------------ |
| W   | Rotate up    |
| A   | Rotate left  |
| S   | Rotate down  |
| D   | Rotate Right |
| Q   | Zoom out     |
| E   | Zoom in      |

### Perspective (free)

| Key   | Description    |
| ----- | -------------- |
| W     | Move forwards  |
| A     | Move backwards |
| S     | Move left      |
| D     | Move Right     |
| Q     | Move down      |
| E     | Move up        |
| Mouse | Rotate         |
