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
    : m_mouseX{ 0.0 }
    , m_mouseY{ 0.0 }
    , m_currentTime{ glfwGetTime() }
    , m_movementSpeed{ 10.0f }
    , m_rotationSpeed{ 0.005f }
    , m_movementMultiplier{ 1.5f }
    , m_cameraQuery{ EntityQuery{}.with<Camera, FreeFly, Transform>() }
    , m_componentManager{}
{
    glfwGetCursorPos(glfwGetCurrentContext(), &m_mouseX, &m_mouseY);
}

void FreeFlyCameraMovementSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void FreeFlyCameraMovementSystem::terminate() { m_componentManager = nullptr; }

void FreeFlyCameraMovementSystem::run(void*)
{
    if (isDetached()) {
        return;
    }

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

    auto movementSpeed{ m_movementSpeed * deltaTime };
    if (shiftKey == GLFW_PRESS) {
        movementSpeed *= m_movementMultiplier * 10.0f;
    } else if (ctrlKey) {
        movementSpeed *= m_movementMultiplier;
    }

    double mouseX{ 0.0 };
    double mouseY{ 0.0 };
    glfwGetCursorPos(window, &mouseX, &mouseY);
    double mouseXOffset{ m_mouseX - mouseX };
    double mouseYOffset{ m_mouseY - mouseY };
    m_mouseX = mouseX;
    m_mouseY = mouseY;

    constexpr glm::vec3 forward{ 0.0f, 0.0f, 1.0f };
    constexpr glm::vec3 right{ 1.0f, 0.0f, 0.0f };
    constexpr glm::vec3 up{ 0.0f, 1.0f, 0.0f };

    glm::vec3 mouseRotation{ m_rotationSpeed * mouseYOffset, m_rotationSpeed * mouseXOffset, 0.0 };

    auto activeFreeCameras{ m_cameraQuery.query(*m_componentManager).filter<Camera>([](const Camera* camera) -> bool {
        return camera->m_active && !camera->m_fixed;
    }) };

    auto perspectiveCameras{ activeFreeCameras };
    auto orthographicCameras{ activeFreeCameras };
    perspectiveCameras.filter<Camera>([](const Camera* camera) -> bool { return camera->perspective; });
    orthographicCameras.filter<Camera>([](const Camera* camera) -> bool { return !camera->perspective; });

    perspectiveCameras.forEach<Transform>([&](Transform* transform) {
        auto localRight{ transform->rotation * right };
        auto upRotation{ glm::angleAxis(mouseRotation.y, up) };
        auto rightRotation{ glm::angleAxis(mouseRotation.x, localRight) };

        auto rotation{ upRotation * rightRotation * transform->rotation };
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

    orthographicCameras.forEach<Camera, Transform>([&](Camera* camera, Transform* transform) {
        transform->rotation = glm::identity<glm::quat>();

        if (wKey == GLFW_PRESS) {
            transform->position += movementSpeed * up;
        }
        if (sKey == GLFW_PRESS) {
            transform->position -= movementSpeed * up;
        }

        if (aKey == GLFW_PRESS) {
            transform->position -= movementSpeed * right;
        }
        if (dKey == GLFW_PRESS) {
            transform->position += movementSpeed * right;
        }

        if (qKey == GLFW_PRESS) {
            camera->orthographicWidth += movementSpeed;
            camera->orthographicHeight = camera->orthographicWidth / camera->aspect;
        }
        if (eKey == GLFW_PRESS) {
            camera->orthographicWidth -= movementSpeed;
            camera->orthographicHeight = camera->orthographicWidth / camera->aspect;

            camera->orthographicWidth = camera->orthographicWidth <= 5.0f ? 5.0f : camera->orthographicWidth;
            camera->orthographicHeight = camera->orthographicHeight <= 5.0f ? 5.0f : camera->orthographicHeight;
        }
    });
    ;
}

}