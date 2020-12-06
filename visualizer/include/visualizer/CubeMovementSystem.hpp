#pragma once

#include <visualizer/EntityDBQuery.hpp>
#include <visualizer/EntityDatabase.hpp>
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
    double m_tick_interval;
    EntityDBQuery m_cubes_query_mesh;
    EntityDBQuery m_cubes_query_activation;
    EntityDBQuery m_cubes_query_homogeneous;
    EntityDBQuery m_cubes_query_heterogeneous;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}