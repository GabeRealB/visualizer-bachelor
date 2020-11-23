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
    EntityQuery m_camera_query;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}