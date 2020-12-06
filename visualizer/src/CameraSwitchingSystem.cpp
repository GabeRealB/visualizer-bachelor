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
    : m_camera_switcher_query{ EntityDBQuery{}.with_component<ActiveCameraSwitcher>() }
    , m_entity_database{}
{
}

void CameraSwitchingSystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void CameraSwitchingSystem::terminate() { m_entity_database = nullptr; }

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

        m_entity_database->enter_secure_context([&](EntityDatabaseContext& database_context) {
            m_camera_switcher_query.query_db_window(database_context)
                .for_each<ActiveCameraSwitcher>([&](ActiveCameraSwitcher* switcher) {
                    auto current{ switcher->cameras[switcher->current] };
                    switcher->current++;
                    if (switcher->current == switcher->cameras.size()) {
                        switcher->current = 0;
                    }
                    auto next{ switcher->cameras[switcher->current] };

                    auto& current_camera{ database_context.fetch_component_unchecked<Camera>(current) };
                    auto& next_camera{ database_context.fetch_component_unchecked<Camera>(next) };

                    current_camera.m_active = false;
                    next_camera.m_active = true;
                });
        });
    }
}

}