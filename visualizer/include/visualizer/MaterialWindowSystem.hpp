#pragma once

#include <visualizer/EntityDBQuery.hpp>
#include <visualizer/EntityDatabase.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class MaterialWindowSystem : public System {
public:
    MaterialWindowSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    bool m_active;
    EntityDBQuery m_camera_query;
    EntityDBQuery m_materials_query;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}