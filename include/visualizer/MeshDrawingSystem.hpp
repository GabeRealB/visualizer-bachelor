#pragma once

#include <memory>

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/Framebuffer.hpp>
#include <visualizer/System.hpp>
#include <visualizer/Texture.hpp>

namespace Visualizer {

class MeshDrawingSystem : public System {
public:
    MeshDrawingSystem(std::shared_ptr<Texture2D> texture);

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    EntityQuery m_meshQuery;
    EntityQuery m_cameraQuery;
    Framebuffer m_frameBuffer;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}