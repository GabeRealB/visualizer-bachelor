#include <visualizer/FreeFlyCameraMovementSystem.hpp>

#if __has_include(<glad\glad.h>)
#include <glad\glad.h>
#else
#include <glad.h>
#endif

#include <GLFW/glfw3.h>

#include <visualizer/Camera.hpp>
#include <visualizer/FreeFly.hpp>
#include <visualizer/Transform.hpp>
#include <visualizer/Visualizer.hpp>

namespace Visualizer {

FreeFlyCameraMovementSystem::FreeFlyCameraMovementSystem()
    : m_mouse_x{ 0.0 }
    , m_mouse_y{ 0.0 }
    , m_current_time{ glfwGetTime() }
    , m_movement_speed{ 10.0f }
    , m_rotation_speed{ 0.005f }
    , m_movement_multiplier{ 1.5f }
    , m_camera_query{ EntityDBQuery{}.with_component<Camera, FreeFly, Transform>() }
    , m_entity_database{}
{
    glfwGetCursorPos(glfwGetCurrentContext(), &m_mouse_x, &m_mouse_y);
}

void FreeFlyCameraMovementSystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void FreeFlyCameraMovementSystem::terminate() { m_entity_database = nullptr; }

void FreeFlyCameraMovementSystem::run(void*)
{
    if (isDetached()) {
        return;
    }

    auto window{ glfwGetCurrentContext() };
    auto w_key{ glfwGetKey(window, GLFW_KEY_W) };
    auto a_key{ glfwGetKey(window, GLFW_KEY_A) };
    auto s_key{ glfwGetKey(window, GLFW_KEY_S) };
    auto d_key{ glfwGetKey(window, GLFW_KEY_D) };
    auto q_key{ glfwGetKey(window, GLFW_KEY_Q) };
    auto e_key{ glfwGetKey(window, GLFW_KEY_E) };

    auto shift_key{ glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) };
    auto ctrl_key{ glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) };
    auto new_time{ glfwGetTime() };
    auto delta_time{ static_cast<float>(new_time - m_current_time) };
    m_current_time = new_time;

    auto movement_speed{ m_movement_speed * delta_time };
    if (shift_key == GLFW_PRESS) {
        movement_speed *= m_movement_multiplier * 10.0f;
    } else if (ctrl_key) {
        movement_speed *= m_movement_multiplier;
    }

    double mouse_x{ 0.0 };
    double mouse_y{ 0.0 };
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    double mouse_x_offset{ m_mouse_x - mouse_x };
    double mouse_y_offset{ m_mouse_y - mouse_y };
    m_mouse_x = mouse_x;
    m_mouse_y = mouse_y;

    constexpr glm::vec3 forward{ 0.0f, 0.0f, 1.0f };
    constexpr glm::vec3 right{ 1.0f, 0.0f, 0.0f };
    constexpr glm::vec3 up{ 0.0f, 1.0f, 0.0f };

    glm::vec3 mouse_rotation{ m_rotation_speed * mouse_y_offset, m_rotation_speed * mouse_x_offset, 0.0 };

    m_entity_database->enter_secure_context([&](EntityDatabaseContext& database_context) {
        auto activeFreeCameras{ m_camera_query.query_db_window(database_context)
                                    .filter<Camera, Transform>([](const Camera* camera, const Transform*) -> bool {
                                        return camera->m_active && !camera->m_fixed;
                                    }) };

        auto perspectiveCameras{ activeFreeCameras.filter<Camera, Transform>(
            [](const Camera* camera, const Transform*) -> bool { return camera->perspective; }) };
        auto orthographicCameras{ activeFreeCameras.filter<Camera, Transform>(
            [](const Camera* camera, const Transform*) -> bool { return !camera->perspective; }) };

        perspectiveCameras.for_each<Transform>([&](Transform* transform) {
            auto localRight{ transform->rotation * right };
            auto upRotation{ glm::angleAxis(mouse_rotation.y, up) };
            auto rightRotation{ glm::angleAxis(mouse_rotation.x, localRight) };

            auto rotation{ upRotation * rightRotation * transform->rotation };
            transform->rotation = rotation;

            auto rotatedForwards{ rotation * forward };
            auto rotatedRight{ rotation * right };
            auto rotatedUp{ rotation * up };

            if (w_key == GLFW_PRESS) {
                transform->position -= movement_speed * rotatedForwards;
            }
            if (s_key == GLFW_PRESS) {
                transform->position += movement_speed * rotatedForwards;
            }

            if (a_key == GLFW_PRESS) {
                transform->position -= movement_speed * rotatedRight;
            }
            if (d_key == GLFW_PRESS) {
                transform->position += movement_speed * rotatedRight;
            }

            if (q_key == GLFW_PRESS) {
                transform->position -= movement_speed * rotatedUp;
            }
            if (e_key == GLFW_PRESS) {
                transform->position += movement_speed * rotatedUp;
            }
        });

        orthographicCameras.for_each<Camera, Transform>([&](Camera* camera, Transform* transform) {
            transform->rotation = glm::identity<glm::quat>();

            if (w_key == GLFW_PRESS) {
                transform->position += movement_speed * up;
            }
            if (s_key == GLFW_PRESS) {
                transform->position -= movement_speed * up;
            }

            if (a_key == GLFW_PRESS) {
                transform->position -= movement_speed * right;
            }
            if (d_key == GLFW_PRESS) {
                transform->position += movement_speed * right;
            }

            if (q_key == GLFW_PRESS) {
                camera->orthographicWidth += movement_speed;
                camera->orthographicHeight = camera->orthographicWidth / camera->aspect;
            }
            if (e_key == GLFW_PRESS) {
                camera->orthographicWidth -= movement_speed;
                camera->orthographicHeight = camera->orthographicWidth / camera->aspect;

                camera->orthographicWidth = camera->orthographicWidth <= 5.0f ? 5.0f : camera->orthographicWidth;
                camera->orthographicHeight = camera->orthographicHeight <= 5.0f ? 5.0f : camera->orthographicHeight;
            }
        });
    });
}

}