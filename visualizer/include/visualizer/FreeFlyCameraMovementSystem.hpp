#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

#include <visualizer/EntityDBQuery.hpp>
#include <visualizer/EntityDatabase.hpp>
#include <visualizer/Scene.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class FreeFlyCameraMovementSystem : public System {
public:
    FreeFlyCameraMovementSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    double m_mouse_x;
    double m_mouse_y;
    double m_current_time;
    float m_movement_speed;
    float m_rotation_speed;
    float m_movement_multiplier;
    EntityDBQuery m_camera_query;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}