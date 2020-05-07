if (DEFINED CMAKE_BUILD_TYPE)
    set(BUILD_TYPE "")
    string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    if (${BUILD_TYPE} STREQUAL "DEBUG")
        set(DISABLE_OPTIMIZATIONS ON)
    endif ()
endif ()

if (WIN32)
    set(COMPILER_FLAGS /W4 /WX /EHca)
else ()
    set(COMPILER_FLAGS -Wall -Wextra -Wpedantic -Werror -fexceptions)
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


add_library(visualizer STATIC VisualizerConfiguration.cpp
        ../include/visualizer/VisualizerConfiguration.hpp
        ../include/visualizer/Mesh.hpp
        ../include/visualizer/Scene.hpp
        ../include/visualizer/SubScene.hpp
        ../include/visualizer/DrawableCube.hpp
        ../include/visualizer/Camera.hpp
        ../include/visualizer/Visualizer.hpp
        ../include/visualizer/Utilities.hpp
        DrawableCube.cpp
        Utilities.cpp
        Visualizer.cpp)
target_link_libraries(visualizer PUBLIC nlohmann_json::nlohmann_json glm::glm glfw glad)
target_include_directories(visualizer PUBLIC "${CMAKE_SOURCE_DIR}/include")
foreach (FLAG IN LISTS COMPILER_FLAGS)
    target_compile_options(visualizer PUBLIC ${FLAG})
    message(STATUS "Added ${FLAG} to the compilation flags")
endforeach ()

add_custom_command(TARGET visualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${PROJECT_SOURCE_DIR}/config.json"
        $<TARGET_FILE_DIR:visualizer>)

add_executable(visualizer_main main.cpp)
target_link_libraries(visualizer_main PRIVATE visualizer)