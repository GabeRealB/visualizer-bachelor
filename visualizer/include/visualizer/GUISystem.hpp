#pragma once

#include <visualizer/EntityDBQuery.hpp>
#include <visualizer/EntityDatabase.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class GUISystem : public System {
public:
    GUISystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    EntityDBQuery m_canvas_query;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}