#include <visualizer/CameraSwitchingSystem.hpp>

#if __has_include(<glad\glad.h>)
#include <glad\glad.h>
#else
#include <glad.h>
#endif

#include <GLFW/glfw3.h>

#include <visualizer/ActiveCameraSwitcher.hpp>
#include <visualizer/Camera.hpp>
#include <visualizer/Visualizer.hpp>

namespace Visualizer {

CameraSwitchingSystem::CameraSwitchingSystem()
    : m_cameraSwitcherQuery{ EntityQuery{}.with<ActiveCameraSwitcher>() }
    , m_componentManager{}
{
}

void CameraSwitchingSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void CameraSwitchingSystem::terminate() { m_componentManager = nullptr; }

void CameraSwitchingSystem::run(void*)
{
    if (isDetached()) {
        return;
    }

    auto window{ glfwGetCurrentContext() };
    auto tabKey{ glfwGetKey(window, GLFW_KEY_TAB) };

    static bool tabPressed = false;

    if (tabKey == GLFW_PRESS) {
        tabPressed = true;
    }

    if (tabKey == GLFW_RELEASE && tabPressed) {
        tabPressed = false;

        m_cameraSwitcherQuery.query(*m_componentManager)
            .forEach<ActiveCameraSwitcher>([componentManager = m_componentManager](ActiveCameraSwitcher* switcher) {
                auto current{ switcher->cameras[switcher->current] };
                switcher->current++;
                if (switcher->current == switcher->cameras.size()) {
                    switcher->current = 0;
                }
                auto next{ switcher->cameras[switcher->current] };

                auto currentCamera{ static_cast<Camera*>(
                    componentManager->getEntityComponentPointer(current, getTypeId<Camera>())) };
                auto nextCamera{ static_cast<Camera*>(
                    componentManager->getEntityComponentPointer(next, getTypeId<Camera>())) };

                currentCamera->m_active = false;
                nextCamera->m_active = true;
            });
    }
}

}