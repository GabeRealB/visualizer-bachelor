#include <visualizer/CubeMovementSystem.hpp>

#include <GLFW/glfw3.h>

#include <visualizer/CubeTickInfo.hpp>
#include <visualizer/Iteration.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

CubeMovementSystem::CubeMovementSystem()
    : m_accumulator{ 0 }
    , m_currentTime{ 0 }
    , m_tickInterval{ 1.0 }
    , m_cubesQuery{ EntityQuery{}.with<Iteration, Transform>() }
    , m_componentManager{}
{
    m_currentTime = glfwGetTime();
}

void CubeMovementSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void CubeMovementSystem::terminate() { m_componentManager = nullptr; }

void reverseTransform(const Iteration& iteration, Transform& transform)
{
    auto position = iteration.positions[iteration.index];

    auto posX{ transform.scale.x * position.x };
    auto posY{ -transform.scale.y * position.y };
    auto posZ{ transform.scale.z * position.z };

    transform.position -= glm::vec3{ posX, posY, posZ };
}

void stepIteration(Iteration& iteration)
{
    if (++iteration.tick % iteration.ticksPerIteration != 0) {
        return;
    } else {
        iteration.tick = 0;
    }

    if (++iteration.index >= iteration.positions.size()) {
        iteration.index = 0;
    }
}

void computeTransform(const Iteration& iteration, Transform& transform)
{
    auto position = iteration.positions[iteration.index];

    auto posX{ transform.scale.x * position.x };
    auto posY{ -transform.scale.y * position.y };
    auto posZ{ transform.scale.z * position.z };

    transform.position += glm::vec3{ posX, posY, posZ };
}

void CubeMovementSystem::run(void*)
{
    auto currentTime{ glfwGetTime() };
    auto deltaTime{ currentTime - m_currentTime };
    m_currentTime = currentTime;

    auto window{ glfwGetCurrentContext() };
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        m_tickInterval = 1.0f;
    } else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        m_tickInterval = 0.1f;
    } else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        m_tickInterval = 0.01f;
    } else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        m_tickInterval = 0.001f;
    } else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        m_tickInterval = 0.0001f;
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        m_tickInterval = std::numeric_limits<double>::max();
    }

    m_accumulator += deltaTime;
    if (m_accumulator >= m_tickInterval) {
        m_accumulator = 0;
        m_cubesQuery.query(*m_componentManager)
            .forEach<Iteration, Transform>([](Iteration* iteration, Transform* transform) {
                reverseTransform(*iteration, *transform);
                stepIteration(*iteration);
                computeTransform(*iteration, *transform);
            });
    }
}

}