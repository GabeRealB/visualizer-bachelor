#pragma once

#include <visualizer/EntityDBQuery.hpp>
#include <visualizer/EntityDatabase.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class CameraSwitchingSystem : public System {
public:
    CameraSwitchingSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    EntityDBQuery m_camera_switcher_query;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}