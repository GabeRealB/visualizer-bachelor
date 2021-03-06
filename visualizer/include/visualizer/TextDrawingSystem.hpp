#pragma once

#include <array>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include <visualizer/EntityDBQuery.hpp>
#include <visualizer/EntityDatabase.hpp>
#include <visualizer/Framebuffer.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/System.hpp>
#include <visualizer/Texture.hpp>

namespace Visualizer {

class TextDrawingSystem : public System {
public:
    TextDrawingSystem();

    void initialize() final;
    void terminate() final;
    void run(void* data) final;

private:
    struct CharacterInfo {
        FT_Pos m_offset;
        glm::ivec2 m_size;
        glm::ivec2 m_bearing;
        std::shared_ptr<Texture2D> m_texture;
    };

    EntityDBQuery m_textQuery;
    EntityDBQuery m_cameraQuery;

    Mesh m_bitmap_mesh;
    Material m_bitmap_material;

    std::array<CharacterInfo, 128> m_characters;
    std::shared_ptr<EntityDatabase> m_entity_database;
};

}