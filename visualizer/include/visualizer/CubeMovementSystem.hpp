#pragma once

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class CubeMovementSystem : public System {
public:
    CubeMovementSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    double m_accumulator;
    double m_currentTime;
    double m_tickInterval;
    EntityQuery m_cubesQueryHomogeneous;
    EntityQuery m_cubesQueryHeterogeneous;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}