#pragma once

#include <visualizer/EntityDBQuery.hpp>
#include <visualizer/EntityDatabase.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class CuboidCommandSystem : public System {
public:
    CuboidCommandSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    double m_current_time;
    double m_tick_interval;
    double m_time_accumulator;
    EntityDBQuery m_cuboid_query;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}