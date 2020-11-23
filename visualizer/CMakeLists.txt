cmake_minimum_required(VERSION 3.13)

project(visualizer VERSION 1.0
        DESCRIPTION ""
        LANGUAGES CXX)

set(VISUALIZER_INCLUDES
        include/visualizer/AlignedMemory.hpp
        include/visualizer/AssetDatabase.hpp
        include/visualizer/Camera.hpp
        include/visualizer/FreeFlyCameraMovementSystem.hpp
        include/visualizer/EntityDatabase.hpp
        include/visualizer/CompositingSystem.hpp
        include/visualizer/Composition.hpp
        include/visualizer/Cube.hpp
        include/visualizer/CubeMovementSystem.hpp
        include/visualizer/Entity.hpp
        include/visualizer/EntityArchetype.hpp
        include/visualizer/EntityArchetype.impl
        include/visualizer/EntityQuery.hpp
        include/visualizer/EntityQuery.impl
        include/visualizer/Framebuffer.hpp
        include/visualizer/FreeFly.hpp
        include/visualizer/GenericBuffer.hpp
        include/visualizer/Iteration.hpp
        include/visualizer/Mesh.hpp
        include/visualizer/MeshDrawingSystem.hpp
        include/visualizer/Parent.hpp
        include/visualizer/Renderbuffer.hpp
        include/visualizer/RenderLayer.hpp
        include/visualizer/Scene.hpp
        include/visualizer/Shader.hpp
        include/visualizer/System.hpp
        include/visualizer/SystemManager.hpp
        include/visualizer/SystemManager.impl
        include/visualizer/Texture.hpp
        include/visualizer/Transform.hpp
        include/visualizer/TupleUtils.hpp
        include/visualizer/TypeId.hpp
        include/visualizer/UniqueTypes.hpp
        include/visualizer/VertexAttributeBuffer.hpp
        include/visualizer/Visualizer.hpp
        include/visualizer/VisualizerConfiguration.hpp
        include/visualizer/World.hpp
        include/visualizer/UIText.hpp
        include/visualizer/TextDrawingSystem.hpp
        include/visualizer/FixedCamera.hpp
        include/visualizer/ActiveCameraSwitcher.hpp
        include/visualizer/CameraSwitchingSystem.hpp
        include/visualizer/CameraTypeSwitchingSystem.hpp
        include/visualizer/FixedCameraMovementSystem.hpp
        include/visualizer/EntityContainer.hpp
        include/visualizer/EntityDatabase.impl)

set(VISUALIZER_SRC
        src/FreeFlyCameraMovementSystem.cpp
        src/EntityDatabase.cpp
        src/CompositingSystem.cpp
        src/CubeMovementSystem.cpp
        src/Entity.cpp
        src/EntityArchetype.cpp
        src/EntityQuery.cpp
        src/Framebuffer.cpp
        src/GenericBuffer.cpp
        src/Mesh.cpp
        src/MeshDrawingSystem.cpp
        src/Renderbuffer.cpp
        src/Scene.cpp
        src/Shader.cpp
        src/SystemManager.cpp
        src/Texture.cpp
        src/Transform.cpp
        src/VertexAttributeBuffer.cpp
        src/Visualizer.cpp
        src/VisualizerConfiguration.cpp
        src/World.cpp
        src/TextDrawingSystem.cpp
        src/AssetDatabase.cpp
        src/CameraSwitchingSystem.cpp src/CameraTypeSwitchingSystem.cpp src/FixedCameraMovementSystem.cpp src/EntityContainer.cpp)


add_library(visualizer STATIC ${VISUALIZER_INCLUDES} ${VISUALIZER_SRC})
target_link_libraries(visualizer PUBLIC common_options visconfig freetype nlohmann_json::nlohmann_json glm glfw glad::glad)
target_include_directories(visualizer PUBLIC include ${STB_INCLUDE_DIRS})

add_custom_command(TARGET visualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:visualizer>/assets")

add_executable(visualizer_main src/main.cpp)
target_link_libraries(visualizer_main PRIVATE visualizer)

if (BUILD_TESTS)
    add_subdirectory(test)
endif ()