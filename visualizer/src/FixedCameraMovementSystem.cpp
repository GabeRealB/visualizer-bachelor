#include <visualizer/FixedCameraMovementSystem.hpp>

#include <glad\glad.h>

#include <GLFW/glfw3.h>

#include <visualizer/Camera.hpp>
#include <visualizer/FixedCamera.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

FixedCameraMovementSystem::FixedCameraMovementSystem()
    : m_currentTime{ glfwGetTime() }
    , m_cameraQuery{ EntityQuery{}.with<Camera, FixedCamera, Transform>() }
    , m_componentManager{}
{
}

void FixedCameraMovementSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void FixedCameraMovementSystem::terminate() { m_componentManager = nullptr; }

void FixedCameraMovementSystem::run(void*)
{
    auto window{ glfwGetCurrentContext() };
    auto wKey{ glfwGetKey(window, GLFW_KEY_W) };
    auto aKey{ glfwGetKey(window, GLFW_KEY_A) };
    auto sKey{ glfwGetKey(window, GLFW_KEY_S) };
    auto dKey{ glfwGetKey(window, GLFW_KEY_D) };
    auto qKey{ glfwGetKey(window, GLFW_KEY_Q) };
    auto eKey{ glfwGetKey(window, GLFW_KEY_E) };

    auto shiftKey{ glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) };
    auto ctrlKey{ glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) };
    auto newTime{ glfwGetTime() };
    auto deltaTime{ static_cast<float>(newTime - m_currentTime) };
    m_currentTime = newTime;

    auto movementSpeed{ 1 * deltaTime };
    if (shiftKey == GLFW_PRESS) {
        movementSpeed *= 10 * 10.0f;
    } else if (ctrlKey) {
        movementSpeed *= 10;
    }

    m_cameraQuery.query(*m_componentManager)
        .filter<Camera>([](const Camera* camera) -> bool { return camera->m_active && camera->m_fixed; })
        .forEach<FixedCamera, Transform>([&](FixedCamera* camera, Transform* transform) {
            if (wKey == GLFW_PRESS) {
                camera->verticalAngle += movementSpeed;
                if (camera->verticalAngle >= 2 * glm::pi<float>()) {
                    camera->verticalAngle = 0;
                }
            }

            if (sKey == GLFW_PRESS) {
                camera->verticalAngle -= movementSpeed;
                if (camera->verticalAngle <= -2 * glm::pi<float>()) {
                    camera->verticalAngle = 0;
                }
            }

            if (aKey == GLFW_PRESS) {
                camera->horizontalAngle -= movementSpeed;
                if (camera->horizontalAngle >= 2 * glm::pi<float>()) {
                    camera->horizontalAngle = 0;
                }
            }

            if (dKey == GLFW_PRESS) {
                camera->horizontalAngle += movementSpeed;
                if (camera->horizontalAngle <= -2 * glm::pi<float>()) {
                    camera->horizontalAngle = 0;
                }
            }

            if (qKey == GLFW_PRESS) {
                camera->distance -= movementSpeed;
            }

            if (eKey == GLFW_PRESS) {
                camera->distance += movementSpeed;
            }

            (void)transform;
        });
}

}
