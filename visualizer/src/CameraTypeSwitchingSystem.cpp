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
    : m_cameraQuery{ EntityQuery{}.with<Camera, FreeFly, FixedCamera>() }
    , m_componentManager{}
{
}

void CameraTypeSwitchingSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void CameraTypeSwitchingSystem::terminate() { m_componentManager = nullptr; }

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

    if (fKey == GLFW_RELEASE && fPressed) {
        fPressed = false;

        m_cameraQuery.query(*m_componentManager)
            .filter<Camera>([](const Camera* camera) { return camera->m_active; })
            .forEach<Camera>([](Camera* camera) { camera->m_fixed = !camera->m_fixed; });
    }

    if (gKey == GLFW_RELEASE && gPressed) {
        gPressed = false;

        m_cameraQuery.query(*m_componentManager)
            .filter<Camera>([](const Camera* camera) { return camera->m_active; })
            .forEach<Camera>([](Camera* camera) { camera->perspective = !camera->perspective; });
    }
}

}