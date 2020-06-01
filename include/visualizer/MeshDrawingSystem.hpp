#pragma once

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class MeshDrawingSystem : public System {
public:
    MeshDrawingSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    EntityQuery m_meshQuery;
    EntityQuery m_cameraQuery;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}