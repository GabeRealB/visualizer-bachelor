#pragma once

#include <memory>
#include <vector>

#include <visualizer/Mesh.hpp>
#include <visualizer/Scene.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/System.hpp>
#include <visualizer/Texture.hpp>
#include <visualizer/Transform.hpp>
#include <visualizer/VisualizerConfiguration.hpp>

namespace Visualizer {

class CompositingSystem : public System {
public:
    CompositingSystem(const VisualizerConfiguration& config, std::vector<std::shared_ptr<Texture2D>> views);

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    Mesh m_quad;
    ShaderEnvironment m_shaderEnvironment;
    std::vector<Transform> m_viewTransforms;
    std::shared_ptr<ShaderProgram> m_program;
    std::vector<std::shared_ptr<Texture2D>> m_views;
};

}