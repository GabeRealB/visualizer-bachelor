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

if (DEFINED CMAKE_BUILD_TYPE)
    set(BUILD_TYPE "")
    string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    if (${BUILD_TYPE} STREQUAL "DEBUG")
        set(DISABLE_OPTIMIZATIONS ON)
    endif ()
endif ()

if (WIN32)
    set(COMPILER_FLAGS /W4 /WX /EHca -Wno-deprecated-volatile)
else ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(COMPILER_FLAGS -Wall -Wextra -Wpedantic -Werror -fexceptions -Wno-deprecated-volatile)
    else ()
        set(COMPILER_FLAGS -Wall -Wextra -Wpedantic -Werror -fexceptions -Wno-volatile)
    endif ()
endif ()

if (NOT DISABLE_OPTIMIZATIONS)
    if (MSVC)
        list(APPEND COMPILER_FLAGS /O2)
    else ()
        list(APPEND COMPILER_FLAGS -O2)
    endif ()
endif ()

if (ENABLE_NATIVE_ARCH AND NOT MSVC)
    list(APPEND COMPILER_FLAGS -march=native)
endif ()

add_library(common_options INTERFACE)
foreach (FLAG IN LISTS COMPILER_FLAGS)
    target_compile_options(common_options INTERFACE ${FLAG})
    message(STATUS "Added ${FLAG} to the compilation flags")
endforeach ()

add_subdirectory(extern)
add_subdirectory(src)
add_subdirectory(tools)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()