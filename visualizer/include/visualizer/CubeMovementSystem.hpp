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
    double m_tick_interval;
    EntityQuery m_cubes_query_mesh;
    EntityQuery m_cubes_query_activation;
    EntityQuery m_cubes_query_homogeneous;
    EntityQuery m_cubes_query_heterogeneous;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}