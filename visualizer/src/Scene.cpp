#include <visualizer/Scene.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include <visualizer/CameraInitializationSystem.hpp>
#include <visualizer/CameraMovementSystem.hpp>
#include <visualizer/CompositingSystem.hpp>
#include <visualizer/CubeInitializationSystem.hpp>
#include <visualizer/CubeMovementSystem.hpp>
#include <visualizer/MeshDrawingSystem.hpp>
#include <visualizer/SystemManager.hpp>

namespace Visualizer {

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

    if (conf.cubes.size() == 0 || conf.cubes.size() != conf.views.size()) {
        return std::nullopt;
    }

    Scene scene{};
    scene.activeWorld = 0;
    scene.worlds.reserve(conf.cubes.size());

    std::vector<std::shared_ptr<Texture2D>> renderTargets;
    renderTargets.reserve(conf.cubes.size());

    for (std::size_t i = 0; i < renderTargets.capacity(); ++i) {
        auto texture{ std::make_shared<Texture2D>() };
        texture->addAttribute(TextureMinificationFilter::Linear);
        texture->addAttribute(TextureMagnificationFilter::Linear);
        texture->copyData(TextureFormat::RGBA, TextureInternalFormat::Short, 0,
            static_cast<GLsizei>(conf.resolution[0]), static_cast<GLsizei>(conf.resolution[1]), 0, nullptr);
        renderTargets.push_back(texture);
    }

    auto cubeShaderProgram{ createShaderProgram("shader/cube.vs.glsl", "shader/cube.fs.glsl") };
    if (cubeShaderProgram == nullptr) {
        std::cerr << "ERROR: Could not create the cube shader program!" << std::endl;
        return std::nullopt;
    }

    scene.worlds.emplace_back();
    auto& world{ scene.worlds.back() };
    world.addManager<ComponentManager>();
    world.addManager<EntityManager>();
    auto systemManager{ world.addManager<SystemManager>() };

    systemManager->addSystem<CubeInitializationSystem>("initialization"sv);
    systemManager->addSystem<CameraInitializationSystem>("initialization"sv);

    systemManager->addSystem<CubeMovementSystem>("tick"sv);
    systemManager->addSystem<CameraMovementSystem>("tick"sv);

    systemManager->addSystem<MeshDrawingSystem>("draw"sv);
    systemManager->addSystem<CompositingSystem>("composite"sv, conf, renderTargets);

    SystemParameterMap initParameters{};
    CubeInitializationSystem::Data cubeInitData{ conf, cubeShaderProgram };
    CameraInitializationSystem::Data cameraInitData{ renderTargets };
    initParameters.insert<CubeInitializationSystem>(&cubeInitData);
    initParameters.insert<CameraInitializationSystem>(&cameraInitData);
    systemManager->run("initialization"sv, initParameters);

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