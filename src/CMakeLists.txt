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

set(VISUALIZER_INCLUDES
        ../include/visualizer/AlignedMemory.hpp
        ../include/visualizer/Camera.hpp
        ../include/visualizer/CameraInitializationSystem.hpp
        ../include/visualizer/CameraMovementSystem.hpp
        ../include/visualizer/ComponentManager.hpp
        ../include/visualizer/CompositingSystem.hpp
        ../include/visualizer/CubeInitializationSystem.hpp
        ../include/visualizer/CubeMovementSystem.hpp
        ../include/visualizer/CubeTickInfo.hpp
        ../include/visualizer/Entity.hpp
        ../include/visualizer/EntityArchetype.hpp
        ../include/visualizer/EntityArchetype.impl
        ../include/visualizer/EntityComponentMap.hpp
        ../include/visualizer/EntityComponentMap.impl
        ../include/visualizer/EntityManager.hpp
        ../include/visualizer/EntityQuery.hpp
        ../include/visualizer/EntityQuery.impl
        ../include/visualizer/Framebuffer.hpp
        ../include/visualizer/GenericBuffer.hpp
        ../include/visualizer/Mesh.hpp
        ../include/visualizer/MeshDrawingSystem.hpp
        ../include/visualizer/Parent.hpp
        ../include/visualizer/Renderbuffer.hpp
        ../include/visualizer/RenderLayer.hpp
        ../include/visualizer/Scene.hpp
        ../include/visualizer/Shader.hpp
        ../include/visualizer/System.hpp
        ../include/visualizer/SystemManager.hpp
        ../include/visualizer/SystemManager.impl
        ../include/visualizer/Texture.hpp
        ../include/visualizer/Transform.hpp
        ../include/visualizer/TupleUtils.hpp
        ../include/visualizer/TypeId.hpp
        ../include/visualizer/UniqueTypes.hpp
        ../include/visualizer/VertexAttributeBuffer.hpp
        ../include/visualizer/Visualizer.hpp
        ../include/visualizer/VisualizerConfiguration.hpp
        ../include/visualizer/World.hpp
        )

set(VISUALIZER_SRC
        CameraInitializationSystem.cpp
        CameraMovementSystem.cpp
        ComponentManager.cpp
        CompositingSystem.cpp
        CubeInitializationSystem.cpp
        CubeMovementSystem.cpp
        Entity.cpp
        EntityArchetype.cpp
        EntityComponentMap.cpp
        EntityManager.cpp
        EntityQuery.cpp
        Framebuffer.cpp
        GenericBuffer.cpp
        Mesh.cpp
        MeshDrawingSystem.cpp
        Renderbuffer.cpp
        Scene.cpp
        Shader.cpp
        SystemManager.cpp
        Texture.cpp
        Transform.cpp
        VertexAttributeBuffer.cpp
        Visualizer.cpp
        VisualizerConfiguration.cpp
        World.cpp
        )


add_library(visualizer STATIC ${VISUALIZER_INCLUDES} ${VISUALIZER_SRC})
target_link_libraries(visualizer PUBLIC freetype nlohmann_json::nlohmann_json glm::glm glfw glad stb)
target_include_directories(visualizer PUBLIC "${CMAKE_SOURCE_DIR}/include")
foreach (FLAG IN LISTS COMPILER_FLAGS)
    target_compile_options(visualizer PUBLIC ${FLAG})
    message(STATUS "Added ${FLAG} to the compilation flags")
endforeach ()

add_custom_command(TARGET visualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${PROJECT_SOURCE_DIR}/config.json"
        $<TARGET_FILE_DIR:visualizer>)

add_custom_command(TARGET visualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/shader"
        "$<TARGET_FILE_DIR:visualizer>/shader")

add_custom_command(TARGET visualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/textures"
        "$<TARGET_FILE_DIR:visualizer>/textures")

add_executable(visualizer_main main.cpp)
target_link_libraries(visualizer_main PRIVATE visualizer)