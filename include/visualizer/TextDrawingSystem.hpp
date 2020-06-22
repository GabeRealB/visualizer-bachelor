#pragma once

#include <array>
#include <memory>

#include <freetype/freetype.h>
#include <glm/glm.hpp>

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityQuery.hpp>
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

    EntityQuery m_textQuery;
    EntityQuery m_cameraQuery;

    Mesh m_bitmapMesh;
    Material m_bitmapMaterial;

    std::array<CharacterInfo, 128> m_characters;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}