#pragma once

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class CameraTypeSwitchingSystem : public System {
public:
    CameraTypeSwitchingSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    EntityQuery m_cameraQuery;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}