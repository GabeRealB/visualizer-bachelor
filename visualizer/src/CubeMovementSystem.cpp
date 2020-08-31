#include <visualizer/CubeMovementSystem.hpp>

#include <GLFW/glfw3.h>

#include <visualizer/Iteration.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

CubeMovementSystem::CubeMovementSystem()
    : m_accumulator{ 0 }
    , m_currentTime{ 0 }
    , m_tickInterval{ 1.0 }
    , m_cubesQueryActivation{ EntityQuery{}.with<EntityActivation>() }
    , m_cubesQueryHomogeneous{ EntityQuery{}.with<HomogeneousIteration, Transform>() }
    , m_cubesQueryHeterogeneous{ EntityQuery{}.with<HeterogeneousIteration, Transform>() }
    , m_componentManager{}
{
    m_currentTime = glfwGetTime();
}

void CubeMovementSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void CubeMovementSystem::terminate() { m_componentManager = nullptr; }

void reverseTransform(const HomogeneousIteration& iteration, Transform& transform)
{
    auto position = iteration.positions[iteration.index];

    auto posX{ transform.scale.x * position.x };
    auto posY{ -transform.scale.y * position.y };
    auto posZ{ transform.scale.z * position.z };

    transform.position -= glm::vec3{ posX, posY, posZ };
}

void reverseTransform(const HeterogeneousIteration& iteration, Transform& transform)
{
    auto scale = iteration.scales[iteration.index];
    auto position = iteration.positions[iteration.index];

    auto halfScale{ scale / 2.0f };
    halfScale.y *= -1.0f;

    auto posX{ scale.x * position.x };
    auto posY{ -scale.y * position.y };
    auto posZ{ scale.z * position.z };

    auto offset{ halfScale + glm::vec3{ posX, posY, posZ } };
    transform.position -= offset;
}

void stepIteration(HomogeneousIteration& iteration)
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

void stepIteration(EntityActivation& iteration, const std::shared_ptr<ComponentManager>& componentManager)
{
    if (++iteration.tick % iteration.ticksPerIteration[iteration.index] != 0) {
        return;
    } else {
        iteration.tick = 0;
    }

    if (++iteration.index >= iteration.entities.size()) {
        iteration.index = 0;

        for (auto entity : iteration.entities) {
            auto& layer{ *static_cast<RenderLayer*>(
                componentManager->getEntityComponentPointer(entity, getTypeId<RenderLayer>())) };
            layer = 0;
        }
    }

    auto& layer{ *static_cast<RenderLayer*>(
        componentManager->getEntityComponentPointer(iteration.entities[iteration.index], getTypeId<RenderLayer>())) };
    layer = iteration.layer;
}

void stepIteration(HeterogeneousIteration& iteration)
{
    if (++iteration.tick % iteration.ticksPerIteration[iteration.index] != 0) {
        return;
    } else {
        iteration.tick = 0;
    }

    if (++iteration.index >= iteration.positions.size()) {
        iteration.index = 0;
    }
}

void computeTransform(const HomogeneousIteration& iteration, Transform& transform)
{
    auto position = iteration.positions[iteration.index];

    auto posX{ transform.scale.x * position.x };
    auto posY{ -transform.scale.y * position.y };
    auto posZ{ transform.scale.z * position.z };

    transform.position += glm::vec3{ posX, posY, posZ };
}

void computeTransform(const HeterogeneousIteration& iteration, Transform& transform)
{
    auto scale = iteration.scales[iteration.index];
    auto position = iteration.positions[iteration.index];

    auto halfScale{ scale / 2.0f };
    halfScale.y *= -1.0f;

    auto posX{ scale.x * position.x };
    auto posY{ -scale.y * position.y };
    auto posZ{ scale.z * position.z };

    auto offset{ halfScale + glm::vec3{ posX, posY, posZ } };

    transform.scale = scale;
    transform.position += offset;
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
        m_cubesQueryActivation.query(*m_componentManager)
            .forEach<EntityActivation>([&componentManager = this->m_componentManager](EntityActivation* iteration) {
                stepIteration(*iteration, componentManager);
            });

        m_cubesQueryHomogeneous.query(*m_componentManager)
            .forEach<HomogeneousIteration, Transform>([](HomogeneousIteration* iteration, Transform* transform) {
                reverseTransform(*iteration, *transform);
                stepIteration(*iteration);
                computeTransform(*iteration, *transform);
            });

        m_cubesQueryHeterogeneous.query(*m_componentManager)
            .forEach<HeterogeneousIteration, Transform>([](HeterogeneousIteration* iteration, Transform* transform) {
                reverseTransform(*iteration, *transform);
                stepIteration(*iteration);
                computeTransform(*iteration, *transform);
            });
    }
}

}