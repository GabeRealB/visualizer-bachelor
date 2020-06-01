#include <visualizer/CameraMovementSystem.hpp>

#include <GLFW/glfw3.h>

#include <visualizer/Camera.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

CameraMovementSystem::CameraMovementSystem()
    : m_mouseX{ 0.0 }
    , m_mouseY{ 0.0 }
    , m_currentTime{ glfwGetTime() }
    , m_movementSpeed{ 10.0f }
    , m_rotationSpeed{ 0.005f }
    , m_movementMultiplier{ 1.5f }
    , m_cameraQuery{ EntityQuery{}.with<Camera, Transform>() }
    , m_componentManager{}
{
    glfwGetCursorPos(glfwGetCurrentContext(), &m_mouseX, &m_mouseY);
}

void CameraMovementSystem::run(void*)
{
    auto window{ glfwGetCurrentContext() };
    auto wKey{ glfwGetKey(window, GLFW_KEY_W) };
    auto aKey{ glfwGetKey(window, GLFW_KEY_A) };
    auto sKey{ glfwGetKey(window, GLFW_KEY_S) };
    auto dKey{ glfwGetKey(window, GLFW_KEY_D) };
    auto qKey{ glfwGetKey(window, GLFW_KEY_Q) };
    auto eKey{ glfwGetKey(window, GLFW_KEY_E) };

    auto shiftKey{ glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) };
    auto newTime{ glfwGetTime() };
    auto deltaTime{ static_cast<float>(newTime - m_currentTime) };
    m_currentTime = newTime;

    auto movementSpeed{ m_movementSpeed * deltaTime };
    if (shiftKey == GLFW_PRESS) {
        movementSpeed *= m_movementMultiplier;
    }

    double mouseX{ 0.0 };
    double mouseY{ 0.0 };
    glfwGetCursorPos(window, &mouseX, &mouseY);
    double mouseXOffset{ m_mouseX - mouseX };
    double mouseYOffset{ m_mouseY - mouseY };
    m_mouseX = mouseX;
    m_mouseY = mouseY;

    glm::vec3 forward{ 0.0f, 0.0f, 1.0f };
    glm::vec3 right{ 1.0f, 0.0f, 0.0f };
    glm::vec3 up{ 0.0f, 1.0f, 0.0f };

    glm::vec3 mouseRotation{ m_rotationSpeed * mouseYOffset, m_rotationSpeed * mouseXOffset, 0.0 };

    m_cameraQuery.query(*m_componentManager).forEach<Transform>([&](Transform* transform) {
        auto rotation{ glm::quat{ glm::eulerAngles(transform->rotation) + mouseRotation } };
        transform->rotation = rotation;

        auto rotatedForwards{ rotation * forward };
        auto rotatedRight{ rotation * right };
        auto rotatedUp{ rotation * up };

        if (wKey == GLFW_PRESS) {
            transform->position -= movementSpeed * rotatedForwards;
        }
        if (sKey == GLFW_PRESS) {
            transform->position += movementSpeed * rotatedForwards;
        }

        if (aKey == GLFW_PRESS) {
            transform->position -= movementSpeed * rotatedRight;
        }
        if (dKey == GLFW_PRESS) {
            transform->position += movementSpeed * rotatedRight;
        }

        if (qKey == GLFW_PRESS) {
            transform->position -= movementSpeed * rotatedUp;
        }
        if (eKey == GLFW_PRESS) {
            transform->position += movementSpeed * rotatedUp;
        }
    });
}

void CameraMovementSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void CameraMovementSystem::terminate() { m_componentManager = nullptr; }

}