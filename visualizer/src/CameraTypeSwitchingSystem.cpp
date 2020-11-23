#include <visualizer/CameraTypeSwitchingSystem.hpp>

#if __has_include(<glad\glad.h>)
#include <glad\glad.h>
#else
#include <glad.h>
#endif

#include <GLFW/glfw3.h>

#include <visualizer/Camera.hpp>
#include <visualizer/FixedCamera.hpp>
#include <visualizer/FreeFly.hpp>
#include <visualizer/Visualizer.hpp>

namespace Visualizer {

CameraTypeSwitchingSystem::CameraTypeSwitchingSystem()
    : m_camera_query{ EntityQuery{}.with<Camera, FreeFly, FixedCamera>() }
    , m_entity_database{}
{
}

void CameraTypeSwitchingSystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void CameraTypeSwitchingSystem::terminate() { m_entity_database = nullptr; }

void CameraTypeSwitchingSystem::run(void*)
{
    if (isDetached()) {
        return;
    }

    auto window{ glfwGetCurrentContext() };
    auto fKey{ glfwGetKey(window, GLFW_KEY_F) };
    auto gKey{ glfwGetKey(window, GLFW_KEY_G) };

    static bool fPressed = false;
    static bool gPressed = false;

    if (fKey == GLFW_PRESS) {
        fPressed = true;
    }

    if (gKey == GLFW_PRESS) {
        gPressed = true;
    }

    m_entity_database->enter_secure_context([&](EntityDatabaseContext& database_context) {
        if (fKey == GLFW_RELEASE && fPressed) {
            fPressed = false;

            m_camera_query.query(database_context)
                .filter<Camera>([](const Camera* camera) { return camera->m_active; })
                .forEach<Camera>([](Camera* camera) { camera->m_fixed = !camera->m_fixed; });
        }

        if (gKey == GLFW_RELEASE && gPressed) {
            gPressed = false;

            m_camera_query.query(database_context)
                .filter<Camera>([](const Camera* camera) { return camera->m_active; })
                .forEach<Camera>([](Camera* camera) { camera->perspective = !camera->perspective; });
        }
    });
}

}