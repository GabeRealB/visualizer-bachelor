cmake_minimum_required(VERSION 3.13)

project(visualizer  VERSION 1.0
                    DESCRIPTION ""
                    LANGUAGES CXX)

option(BUILD_TESTS "Build the tests." ON)
option(DISABLE_OPTIMIZATIONS "Disables the optimization flags for debugging purposes."  OFF)
option(ENABLE_NATIVE_ARCH "Enables the flag -march=native if it exists" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy --extra-arg-before=--driver-mode=cl --extra-arg=/EHca)
else()
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -extra-arg=-Wno-unknown-warning-option)
endif()

add_subdirectory(extern)
add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()