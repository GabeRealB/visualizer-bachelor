#include <visualizer/CameraInitializationSystem.hpp>

#include <visualizer/Camera.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

CameraInitializationSystem::CameraInitializationSystem()
    : m_cameraArchetype{ EntityArchetype::create<Camera, Transform>() }
    , m_cameraQuery{ m_cameraArchetype }
    , m_entityManager{}
    , m_componentManager{}
{
}

void CameraInitializationSystem::initialize()
{
    m_entityManager = m_world->getManager<EntityManager>();
    m_componentManager = m_world->getManager<ComponentManager>();
}

void CameraInitializationSystem::terminate()
{
    m_entityManager = nullptr;
    m_componentManager = nullptr;
}

void CameraInitializationSystem::run(void* data)
{
    if (data != nullptr) {
        run(*static_cast<CameraInitializationSystem::Data*>(data));
    }
}

void CameraInitializationSystem::run(CameraInitializationSystem::Data& data)
{
    m_entityManager->addEntities(data.m_textures.size(), m_cameraArchetype);

    m_cameraQuery.query(*m_componentManager)
        .iterate<Camera, Transform>([&](std::size_t i, Camera* camera, Transform* transform) {
            camera->m_active = i == 0;
            camera->m_visibleLayers = RenderLayer::layer(i);
            camera->m_renderTarget = std::make_shared<Framebuffer>();
            camera->m_renderTarget->attachBuffer(FramebufferAttachment::Color0, data.m_textures[i]);

            *transform = Transform{
                glm::vec3{ 0.0f, 0.0f, 0.0f },
                glm::vec3{ 0.0f, 0.0f, 20.0f },
                glm::vec3{ 1.0f, 1.0f, 1.0f },
            };
        });
}

}