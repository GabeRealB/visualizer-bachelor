#pragma once

#include <memory>

#include <visualizer/EntityDatabase.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/Framebuffer.hpp>
#include <visualizer/System.hpp>
#include <visualizer/Texture.hpp>

namespace Visualizer {

class MeshDrawingSystem : public System {
public:
    MeshDrawingSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    EntityQuery m_mesh_query;
    EntityQuery m_camera_query;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}