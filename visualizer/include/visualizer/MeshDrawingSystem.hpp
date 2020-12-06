#pragma once

#include <memory>

#include <visualizer/EntityDBQuery.hpp>
#include <visualizer/EntityDatabase.hpp>
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
    EntityDBQuery m_mesh_query;
    EntityDBQuery m_camera_query;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}