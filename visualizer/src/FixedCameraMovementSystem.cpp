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
    : m_currentTime{ glfwGetTime() }
    , m_cameraQuery{ EntityQuery{}.with<Camera, FixedCamera, Transform>() }
    , m_componentManager{}
{
}

void FixedCameraMovementSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void FixedCameraMovementSystem::terminate() { m_componentManager = nullptr; }

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

    auto fixedCameras{ m_cameraQuery.query(*m_componentManager).filter<Camera>([](const Camera* camera) -> bool {
        return camera->m_fixed;
    }) };

    auto perspectiveCameras{ fixedCameras };
    auto orthographicCameras{ fixedCameras };
    perspectiveCameras.filter<Camera>([](const Camera* camera) -> bool { return camera->perspective; });
    orthographicCameras.filter<Camera>([](const Camera* camera) -> bool { return !camera->perspective; });

    perspectiveCameras.forEach<Camera, FixedCamera, Transform>(
        [&](const Camera* camera, FixedCamera* fixedCamera, Transform* transform) {
            if (camera->m_active) {
                if (wKey == GLFW_PRESS) {
                    fixedCamera->verticalAngle -= movementSpeed;
                    if (fixedCamera->verticalAngle <= glm::radians(3.0f)) {
                        fixedCamera->verticalAngle = glm::radians(3.0f);
                    }
                }

                if (sKey == GLFW_PRESS) {
                    fixedCamera->verticalAngle += movementSpeed;
                    if (fixedCamera->verticalAngle >= glm::radians(177.0f)) {
                        fixedCamera->verticalAngle = glm::radians(177.0f);
                    }
                }

                if (aKey == GLFW_PRESS) {
                    fixedCamera->horizontalAngle -= movementSpeed;
                    if (fixedCamera->horizontalAngle <= 0) {
                        fixedCamera->horizontalAngle += 2 * glm::pi<float>();
                    }
                }

                if (dKey == GLFW_PRESS) {
                    fixedCamera->horizontalAngle += movementSpeed;
                    if (fixedCamera->horizontalAngle >= 2 * glm::pi<float>()) {
                        fixedCamera->horizontalAngle -= 2 * glm::pi<float>();
                    }
                }

                if (qKey == GLFW_PRESS) {
                    fixedCamera->distance += movementSpeed;
                }

                if (eKey == GLFW_PRESS) {
                    fixedCamera->distance -= movementSpeed;
                    if (fixedCamera->distance <= 0.0005f) {
                        fixedCamera->distance = 0.0005f;
                    }
                }
            }

            auto parent{ static_cast<const Parent*>(
                m_componentManager->getEntityComponentPointer(fixedCamera->focus, getTypeId<Parent>())) };
            auto modelMatrix{ getModelMatrix(*static_cast<const Transform*>(
                m_componentManager->getEntityComponentPointer(fixedCamera->focus, getTypeId<Transform>()))) };

            while (parent != nullptr) {
                auto parentTransform{ static_cast<const Transform*>(
                    m_componentManager->getEntityComponentPointer(parent->m_parent, getTypeId<Transform>())) };
                modelMatrix = getModelMatrix(*parentTransform) * modelMatrix;
                parent = static_cast<const Parent*>(
                    m_componentManager->getEntityComponentPointer(parent->m_parent, getTypeId<Parent>()));
            }

            glm::vec4 position{ glm::sin(fixedCamera->verticalAngle) * glm::sin(fixedCamera->horizontalAngle)
                    * fixedCamera->distance,
                glm::cos(fixedCamera->verticalAngle) * fixedCamera->distance,
                glm::sin(fixedCamera->verticalAngle) * glm::cos(fixedCamera->horizontalAngle) * fixedCamera->distance,
                1.0f };

            auto cameraPosition{ modelMatrix * position };
            auto focusPosition{ modelMatrix * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f } };

            auto direction{ focusPosition - cameraPosition };
            auto directionNormalized{ glm::normalize(glm::vec3{ direction }) };

            auto rotation{ glm::quatLookAt(directionNormalized, glm::vec3{ 0.0f, 1.0f, 0.0f }) };

            transform->position = cameraPosition;
            transform->rotation = rotation;
        });

    orthographicCameras.forEach<Camera, FixedCamera, Transform>(
        [&](Camera* camera, FixedCamera* fixedCamera, Transform* transform) {
            if (camera->m_active) {
                fixedCamera->horizontalAngle = 0.0f;
                fixedCamera->verticalAngle = glm::pi<float>() / 2;

                if (qKey == GLFW_PRESS) {
                    fixedCamera->distance += movementSpeed;
                }

                if (eKey == GLFW_PRESS) {
                    fixedCamera->distance -= movementSpeed;
                    if (fixedCamera->distance <= 0.0005f) {
                        fixedCamera->distance = 0.0005f;
                    }
                }
            }

            auto parent{ static_cast<const Parent*>(
                m_componentManager->getEntityComponentPointer(fixedCamera->focus, getTypeId<Parent>())) };
            auto modelMatrix{ getModelMatrix(*static_cast<const Transform*>(
                m_componentManager->getEntityComponentPointer(fixedCamera->focus, getTypeId<Transform>()))) };

            while (parent != nullptr) {
                auto parentTransform{ static_cast<const Transform*>(
                    m_componentManager->getEntityComponentPointer(parent->m_parent, getTypeId<Transform>())) };
                modelMatrix = getModelMatrix(*parentTransform) * modelMatrix;
                parent = static_cast<const Parent*>(
                    m_componentManager->getEntityComponentPointer(parent->m_parent, getTypeId<Parent>()));
            }

            glm::vec4 position{ glm::sin(fixedCamera->verticalAngle) * glm::sin(fixedCamera->horizontalAngle)
                    * fixedCamera->distance,
                glm::cos(fixedCamera->verticalAngle) * fixedCamera->distance,
                glm::sin(fixedCamera->verticalAngle) * glm::cos(fixedCamera->horizontalAngle) * fixedCamera->distance,
                1.0f };

            auto cameraPosition{ modelMatrix * position };

            transform->position = cameraPosition;
            transform->rotation = glm::identity<glm::quat>();

            camera->orthographicWidth = fixedCamera->distance;
            camera->orthographicHeight = camera->orthographicWidth / camera->aspect;
        });
}

}
