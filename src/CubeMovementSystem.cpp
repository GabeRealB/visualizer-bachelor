#include <visualizer/CubeMovementSystem.hpp>

#include <GLFW/glfw3.h>

#include <visualizer/CubeTickInfo.hpp>
#include <visualizer/Systems.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

CubeMovementSystem::CubeMovementSystem()
    : m_accumulator{ 0 }
    , m_currentTime{ 0 }
    , m_tickInterval{ 1.0 }
    , m_cubesQuery{ EntityQuery{}.with<CubeTickInfo, Transform>() }
    , m_componentManager{}
{
    m_currentTime = glfwGetTime();
}

void CubeMovementSystem::run(void*)
{
    auto currentTime{ glfwGetTime() };
    auto deltaTime{ currentTime - m_currentTime };
    m_currentTime = currentTime;

    m_accumulator += deltaTime;
    if (m_accumulator >= m_tickInterval) {
        m_accumulator = 0;
        m_cubesQuery.query(*m_componentManager)
            .forEach<CubeTickInfo, Transform>([](CubeTickInfo* tickInfo, Transform* transform) {
                tick(*tickInfo);
                tick(*tickInfo, *transform);
            });
    }
}

void CubeMovementSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }
void CubeMovementSystem::terminate() { m_componentManager = nullptr; }

}