#pragma once

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class FixedCameraMovementSystem : public System {
public:
    FixedCameraMovementSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    double m_currentTime;
    EntityQuery m_cameraQuery;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}
