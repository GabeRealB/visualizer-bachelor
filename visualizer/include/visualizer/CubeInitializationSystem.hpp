#pragma once

#include <memory>

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityArchetype.hpp>
#include <visualizer/EntityManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/System.hpp>
#include <visualizer/Texture.hpp>
#include <visualizer/VisualizerConfiguration.hpp>

namespace Visualizer {

class CubeInitializationSystem : public System {
public:
    CubeInitializationSystem();

    struct Data {
        const VisualizerConfiguration& m_config;
        const std::shared_ptr<ShaderProgram>& m_shader;
    };

    void initialize() final;
    void terminate() final;
    void run(void* data) final;
    void run(Data& data);

private:
    std::shared_ptr<Mesh> m_cubeMesh;
    EntityArchetype m_cubeArchetype;
    EntityArchetype m_childrenArchetype;
    EntityQuery m_cubeQuery;
    EntityQuery m_childrenQuery;
    std::shared_ptr<Texture2D> m_cubeTexture;

    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<ComponentManager> m_componentManager;
};
}