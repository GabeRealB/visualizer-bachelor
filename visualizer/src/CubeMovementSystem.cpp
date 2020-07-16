#include <visualizer/CubeMovementSystem.hpp>

#include <GLFW/glfw3.h>

#include <visualizer/CubeTickInfo.hpp>
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

void CubeMovementSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void CubeMovementSystem::terminate() { m_componentManager = nullptr; }

void tick(CubeTickInfo& cubeInfo)
{
    if (++cubeInfo.currentTick % cubeInfo.tickRate != 0) {
        return;
    } else {
        cubeInfo.currentTick = 0;
    }

    if (cubeInfo.currentIter[cubeInfo.order[0]] < cubeInfo.limits[cubeInfo.order[0]]) {
        cubeInfo.currentIter[cubeInfo.order[0]]++;
    } else {
        cubeInfo.currentIter[cubeInfo.order[0]] = 0;
        if (cubeInfo.currentIter[cubeInfo.order[1]] < cubeInfo.limits[cubeInfo.order[1]]) {
            cubeInfo.currentIter[cubeInfo.order[1]]++;
        } else {
            cubeInfo.currentIter[cubeInfo.order[1]] = 0;
            if (cubeInfo.currentIter[cubeInfo.order[2]] < cubeInfo.limits[cubeInfo.order[2]]) {
                cubeInfo.currentIter[cubeInfo.order[2]]++;
            } else {
                cubeInfo.currentIter[cubeInfo.order[2]] = 0;
            }
        }
    }
}

void tick(const CubeTickInfo& cubeInfo, Transform& transform)
{
    auto posX{ transform.scale.x * cubeInfo.currentIter.x };
    auto posY{ -transform.scale.y * cubeInfo.currentIter.y };
    auto posZ{ transform.scale.z * cubeInfo.currentIter.z };
    transform.position = cubeInfo.startPos + glm::vec3{ posX, posY, posZ };
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
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        m_tickInterval = std::numeric_limits<double>::max();
    }

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

}