#include <visualizer/CameraTypeSwitchingSystem.hpp>

#include <glad\glad.h>

#include <GLFW/glfw3.h>

#include <visualizer/Camera.hpp>
#include <visualizer/FixedCamera.hpp>
#include <visualizer/FreeFly.hpp>

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
    auto window{ glfwGetCurrentContext() };
    auto rKey{ glfwGetKey(window, GLFW_KEY_R) };

    static bool rPressed = false;

    if (rKey == GLFW_PRESS) {
        rPressed = true;
    }

    if (rKey == GLFW_RELEASE && rPressed) {
        rPressed = false;

        m_cameraQuery.query(*m_componentManager)
            .filter<Camera>([](const Camera* camera) { return camera->m_active; })
            .forEach<Camera>([](Camera* camera) { camera->m_fixed = !camera->m_fixed; });
    }
}

}