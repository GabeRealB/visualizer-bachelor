#pragma once

#include <memory>
#include <vector>

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityQuery.hpp>
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
    CompositingSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    Mesh m_quad;
    EntityQuery m_entityQuery;
    ShaderEnvironment m_shaderEnvironment;
    std::shared_ptr<ShaderProgram> m_program;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}