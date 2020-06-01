#pragma once

#include <memory>

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityArchetype.hpp>
#include <visualizer/EntityManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/System.hpp>
#include <visualizer/VisualizerConfiguration.hpp>

namespace Visualizer {

class CubeInitializationSystem : public System {
public:
    CubeInitializationSystem(Resolution resolution, OuterCube outerCube, std::shared_ptr<Mesh> cubeMesh);

    struct Data {
        std::shared_ptr<ShaderProgram> shader;
    };

    void run(Data& data);
    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    Resolution m_resolution;
    OuterCube m_outerCube;
    std::shared_ptr<Mesh> m_cubeMesh;

    EntityArchetype m_cubeArchetype;
    EntityArchetype m_cameraArchetype;

    EntityQuery m_cubeQuery;
    EntityQuery m_cameraQuery;

    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<ComponentManager> m_componentManager;
};
}