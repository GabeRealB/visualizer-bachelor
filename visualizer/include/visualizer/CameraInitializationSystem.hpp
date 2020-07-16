#pragma once

#include <memory>

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityArchetype.hpp>
#include <visualizer/EntityManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/System.hpp>
#include <visualizer/Texture.hpp>

namespace Visualizer {

class CameraInitializationSystem : public System {
public:
    CameraInitializationSystem();

    struct Data {
        const std::vector<std::shared_ptr<Texture2D>>& m_textures;
    };

    void initialize() final;
    void terminate() final;
    void run(void* data) final;
    void run(Data& data);

private:
    EntityArchetype m_cameraArchetype;
    EntityQuery m_cameraQuery;

    std::shared_ptr<EntityManager> m_entityManager;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}