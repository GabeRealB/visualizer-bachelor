#include <visualizer/Scene.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include <visualizer/CameraMovementSystem.hpp>
#include <visualizer/CubeInitializationSystem.hpp>
#include <visualizer/CubeMovementSystem.hpp>
#include <visualizer/MeshDrawingSystem.hpp>
#include <visualizer/SystemManager.hpp>
#include <visualizer/Texture.hpp>

namespace Visualizer {

std::shared_ptr<Mesh> generateCubeMesh()
{
    auto mesh{ std::make_shared<Mesh>() };

    glm::vec4 vertices[]{
        { -0.5f, -0.5f, 0.5f, 1.0f }, // lower-left-front
        { 0.5f, -0.5f, 0.5f, 1.0f }, // lower-right-front
        { 0.5f, 0.5f, 0.5f, 1.0f }, // top-right-front
        { -0.5f, 0.5f, 0.5f, 1.0f }, // top-left-front

        { -0.5f, -0.5f, -0.5f, 1.0f }, // lower-left-back
        { 0.5f, -0.5f, -0.5f, 1.0f }, // lower-right-back
        { 0.5f, 0.5f, -0.5f, 1.0f }, // top-right-back
        { -0.5f, 0.5f, -0.5f, 1.0f }, // top-left-back
    };

    GLuint indices[]{
        0, 1, 2, 0, 2, 3, // front
        3, 2, 6, 3, 6, 7, // top
        1, 5, 6, 1, 6, 2, // right
        4, 0, 3, 4, 3, 7, // left
        4, 5, 1, 4, 1, 0, // bottom
        5, 4, 7, 5, 7, 6 // back
    };

    mesh->setVertices(vertices, 8);
    mesh->setIndices(indices, 48, GL_TRIANGLES);
    return mesh;
}

std::shared_ptr<Visualizer::ShaderProgram> createShaderProgram(
    const std::filesystem::path& vs, const std::filesystem::path& fs)
{
    auto vertexShader{ Visualizer::Shader::create(vs, Visualizer::ShaderType::VertexShader) };
    if (!vertexShader) {
        return nullptr;
    }

    auto fragmentShader{ Visualizer::Shader::create(fs, Visualizer::ShaderType::FragmentShader) };
    if (!fragmentShader) {
        return nullptr;
    }

    return Visualizer::ShaderProgram::create(*vertexShader, *fragmentShader);
}

std::optional<Scene> loadScene(const VisualizerConfiguration& conf)
{
    using namespace std::literals;

    Scene scene{};
    auto cubeMesh{ generateCubeMesh() };

    auto cubeShaderProgram{ createShaderProgram("shader/cube.vs.glsl", "shader/cube.fs.glsl") };
    if (cubeShaderProgram == nullptr) {
        std::cerr << "ERROR: Could not create the cube shader program!" << std::endl;
        return std::nullopt;
    }

    std::vector<std::shared_ptr<Texture2D>> renderTargets;
    renderTargets.reserve(conf.cubes.size());

    for (auto& outerCube : conf.cubes) {
        scene.worlds.emplace_back();
        auto& world{ scene.worlds.back() };
        auto componentManager{ world.addManager<ComponentManager>() };
        auto entityManager{ world.addManager<EntityManager>() };
        auto systemManager{ world.addManager<SystemManager>() };

        systemManager->addSystem<CubeInitializationSystem>("initialization"sv, conf.resolution, outerCube, cubeMesh);

        systemManager->addSystem<CubeMovementSystem>("tick"sv);
        systemManager->addSystem<CameraMovementSystem>("tick"sv);

        auto texture{ std::make_shared<Texture2D>() };
        texture->addAttribute(TextureMinificationFilter::Linear);
        texture->addAttribute(TextureMagnificationFilter::Linear);
        texture->copyData(TextureFormat::RGBA, TextureInternalFormat::Short, 0,
            static_cast<GLsizei>(conf.resolution[0]), static_cast<GLsizei>(conf.resolution[1]), 0, nullptr);
        renderTargets.push_back(texture);

        systemManager->addSystem<MeshDrawingSystem>("draw"sv, texture);
    }

    auto initializationData{ CubeInitializationSystem::Data{ std::move(cubeShaderProgram) } };
    for (auto& world : scene.worlds) {
        world.getManager<SystemManager>()->run("initialization"sv, &initializationData);
    }

    return scene;
}

void tick(Scene& scene)
{
    using namespace std::literals;

    for (auto& world : scene.worlds) {
        auto systemManager{ world.getManager<SystemManager>() };
        systemManager->run("tick"sv);
    }
}

void draw(const Scene& scene)
{
    using namespace std::literals;

    for (auto& world : scene.worlds) {
        auto systemManager{ world.getManager<SystemManager>() };
        systemManager->run("draw"sv);
        systemManager->run("post-process"sv);
    }

    for (auto& world : scene.worlds) {
        auto systemManager{ world.getManager<SystemManager>() };
        systemManager->run("composite"sv);
    }
}

}