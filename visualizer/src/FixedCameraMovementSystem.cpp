#include <visualizer/FixedCameraMovementSystem.hpp>

#if __has_include(<glad\glad.h>)
#include <glad\glad.h>
#else
#include <glad.h>
#endif

#include <GLFW/glfw3.h>

#include <visualizer/Camera.hpp>
#include <visualizer/FixedCamera.hpp>
#include <visualizer/Parent.hpp>
#include <visualizer/Transform.hpp>
#include <visualizer/Visualizer.hpp>

namespace Visualizer {

FixedCameraMovementSystem::FixedCameraMovementSystem()
    : m_current_time{ glfwGetTime() }
    , m_camera_query{ EntityDBQuery{}.with_component<Camera, FixedCamera, Transform>() }
    , m_entity_database{}
{
}

void FixedCameraMovementSystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void FixedCameraMovementSystem::terminate() { m_entity_database = nullptr; }

glm::quat safeQuatLookAt(
    glm::vec3 const& lookFrom, glm::vec3 const& lookTo, glm::vec3 const& up, glm::vec3 const& alternativeUp)
{
    glm::vec3 direction = lookTo - lookFrom;
    float directionLength = glm::length(direction);

    // Check if the direction is valid; Also deals with NaN
    if (directionLength <= 0.0001)
        return glm::quat(1, 0, 0, 0); // Just return identity

    // Normalize direction
    direction /= directionLength;

    // Is the normal up (nearly) parallel to direction?
    if (glm::abs(glm::dot(direction, up)) > .9999f) {
        // Use alternative up
        return glm::quatLookAt(direction, alternativeUp);
    } else {
        return glm::quatLookAt(direction, up);
    }
}

void FixedCameraMovementSystem::run(void*)
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

    auto movement_speed{ 1 * delta_time };
    if (shift_key == GLFW_PRESS) {
        movement_speed *= 10 * 10.0f;
    } else if (ctrl_key) {
        movement_speed *= 10;
    }

    m_entity_database->enter_secure_context([&](EntityDatabaseContext& database_context) {
        auto fixed_cameras{
            m_camera_query.query_db_window(database_context)
                .filter<Camera, FixedCamera, Transform>(
                    [](const Camera* camera, const FixedCamera*, const Transform*) -> bool { return camera->m_fixed; })
        };

        auto perspective_cameras{ fixed_cameras.filter<Camera, FixedCamera, Transform>(
            [](const Camera* camera, const FixedCamera*, const Transform*) -> bool { return camera->perspective; }) };
        auto orthographic_cameras{ fixed_cameras.filter<Camera, FixedCamera, Transform>(
            [](const Camera* camera, const FixedCamera*, const Transform*) -> bool { return !camera->perspective; }) };

        perspective_cameras.for_each<Camera, FixedCamera, Transform>(
            [&](const Camera* camera, FixedCamera* fixed_camera, Transform* transform) {
                if (camera->m_active) {
                    if (w_key == GLFW_PRESS) {
                        fixed_camera->verticalAngle -= movement_speed;
                        if (fixed_camera->verticalAngle <= glm::radians(3.0f)) {
                            fixed_camera->verticalAngle = glm::radians(3.0f);
                        }
                    }

                    if (s_key == GLFW_PRESS) {
                        fixed_camera->verticalAngle += movement_speed;
                        if (fixed_camera->verticalAngle >= glm::radians(177.0f)) {
                            fixed_camera->verticalAngle = glm::radians(177.0f);
                        }
                    }

                    if (a_key == GLFW_PRESS) {
                        fixed_camera->horizontalAngle -= movement_speed;
                        if (fixed_camera->horizontalAngle <= 0) {
                            fixed_camera->horizontalAngle += 2 * glm::pi<float>();
                        }
                    }

                    if (d_key == GLFW_PRESS) {
                        fixed_camera->horizontalAngle += movement_speed;
                        if (fixed_camera->horizontalAngle >= 2 * glm::pi<float>()) {
                            fixed_camera->horizontalAngle -= 2 * glm::pi<float>();
                        }
                    }

                    if (q_key == GLFW_PRESS) {
                        fixed_camera->distance += movement_speed;
                    }

                    if (e_key == GLFW_PRESS) {
                        fixed_camera->distance -= movement_speed;
                        if (fixed_camera->distance <= 0.0005f) {
                            fixed_camera->distance = 0.0005f;
                        }
                    }
                }

                auto model_matrix{ getModelMatrix(
                    database_context.fetch_component_unchecked<Transform>(fixed_camera->focus)) };

                for (auto parent_entity{ fixed_camera->focus };
                     database_context.entity_has_component<Parent>(parent_entity);) {
                    const auto& parent{ database_context.fetch_component_unchecked<Parent>(parent_entity) };
                    model_matrix
                        = getModelMatrix(database_context.fetch_component_unchecked<Transform>(parent.m_parent));
                    parent_entity = parent.m_parent;
                }

                glm::vec4 position{ glm::sin(fixed_camera->verticalAngle) * glm::sin(fixed_camera->horizontalAngle)
                        * fixed_camera->distance,
                    glm::cos(fixed_camera->verticalAngle) * fixed_camera->distance,
                    glm::sin(fixed_camera->verticalAngle) * glm::cos(fixed_camera->horizontalAngle)
                        * fixed_camera->distance,
                    1.0f };

                auto camera_position{ model_matrix * position };
                auto focus_position{ model_matrix * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f } };

                auto direction{ focus_position - camera_position };
                auto direction_normalized{ glm::normalize(glm::vec3{ direction }) };

                auto rotation{ glm::quatLookAt(direction_normalized, glm::vec3{ 0.0f, 1.0f, 0.0f }) };

                transform->position = camera_position;
                transform->rotation = rotation;
            });

        orthographic_cameras.for_each<Camera, FixedCamera, Transform>([&](Camera* camera, FixedCamera* fixed_camera,
                                                                          Transform* transform) {
            if (camera->m_active) {
                fixed_camera->horizontalAngle = 0.0f;
                fixed_camera->verticalAngle = glm::pi<float>() / 2;

                if (q_key == GLFW_PRESS) {
                    fixed_camera->distance += movement_speed;
                }

                if (e_key == GLFW_PRESS) {
                    fixed_camera->distance -= movement_speed;
                    if (fixed_camera->distance <= 0.0005f) {
                        fixed_camera->distance = 0.0005f;
                    }
                }
            }

            auto model_matrix{ getModelMatrix(
                database_context.fetch_component_unchecked<Transform>(fixed_camera->focus)) };

            for (auto parent_entity{ fixed_camera->focus };
                 database_context.entity_has_component<Parent>(parent_entity);) {
                const auto& parent{ database_context.fetch_component_unchecked<Parent>(parent_entity) };
                model_matrix = getModelMatrix(database_context.fetch_component_unchecked<Transform>(parent.m_parent));
                parent_entity = parent.m_parent;
            }

            glm::vec4 position{ glm::sin(fixed_camera->verticalAngle) * glm::sin(fixed_camera->horizontalAngle)
                    * fixed_camera->distance,
                glm::cos(fixed_camera->verticalAngle) * fixed_camera->distance,
                glm::sin(fixed_camera->verticalAngle) * glm::cos(fixed_camera->horizontalAngle)
                    * fixed_camera->distance,
                1.0f };

            auto camera_position{ model_matrix * position };

            transform->position = camera_position;
            transform->rotation = glm::identity<glm::quat>();

            camera->orthographicWidth = fixed_camera->distance;
            camera->orthographicHeight = camera->orthographicWidth / camera->aspect;
        });
    });
}

}
