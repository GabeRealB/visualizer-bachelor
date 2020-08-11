#pragma once

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class CameraSwitchingSystem : public System {
public:
    CameraSwitchingSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    EntityQuery m_cameraSwitcherQuery;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}