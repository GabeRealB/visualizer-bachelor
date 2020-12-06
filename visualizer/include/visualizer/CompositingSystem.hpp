#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <visualizer/EntityDBQuery.hpp>
#include <visualizer/EntityDatabase.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Scene.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/System.hpp>
#include <visualizer/Texture.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

class CompositingSystem : public System {
public:
    CompositingSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    Mesh m_quad;
    EntityDBQuery m_copy_entity_query;
    EntityDBQuery m_draggable_entity_query;
    EntityDBQuery m_composition_entity_query;
    std::optional<std::size_t> m_selected;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}