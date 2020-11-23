#pragma once

#include <visualizer/EntityDatabase.hpp>
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
    double m_current_time;
    EntityQuery m_camera_query;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}
