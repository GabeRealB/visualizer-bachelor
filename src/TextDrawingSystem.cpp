#include <visualizer/TextDrawingSystem.hpp>

#include <visualizer/Camera.hpp>
#include <visualizer/Transform.hpp>
#include <visualizer/UIText.hpp>

namespace Visualizer {

TextDrawingSystem::TextDrawingSystem()
    : m_textQuery{ EntityQuery{}.with<UIText, Transform>() }
    , m_cameraQuery{ EntityQuery{}.with<Camera>() }
    , m_bitmapMesh{}
    , m_bitmapMaterial{}
    , m_characters{}
    , m_componentManager{}
{
    FT_Library freeType;
    FT_Init_FreeType(&freeType);

    FT_Face face{};
    FT_New_Face(freeType, "fonts/arial.ttf", 0, &face);
    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (std::size_t i = 0; i < 128; ++i) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            continue;
        }

        auto texture{ std::make_shared<Texture2D>() };
        texture->copyData(TextureFormat::R, TextureInternalFormat::Byte, 0, face->glyph->bitmap.width,
            face->glyph->bitmap.rows, 0, face->glyph->bitmap.buffer);
        texture->addAttribute(TextureMinificationFilter::Linear);
        texture->addAttribute(TextureMagnificationFilter::Linear);

        m_characters[i] = { face->glyph->advance.x, glm::ivec2{ face->glyph->bitmap.width, face->glyph->bitmap.rows },
            glm::ivec2{ face->glyph->bitmap_left, face->glyph->bitmap_top }, std::move(texture) };
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    FT_Done_Face(face);
    FT_Done_FreeType(freeType);
}

void TextDrawingSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void TextDrawingSystem::terminate() { m_componentManager = nullptr; }

void TextDrawingSystem::run(void*)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto textUIs{ m_textQuery.query(*m_componentManager) };

    m_cameraQuery.query(*m_componentManager).forEach<const Camera>([&](const Camera* camera) {
        camera->m_renderTarget->bind(FramebufferBinding::ReadWrite);
        auto cameraViewport{ camera->m_renderTarget->viewport() };

        auto projectionMatrix{ glm::ortho(
            0.0f, static_cast<float>(cameraViewport.width), 0.0f, static_cast<float>(cameraViewport.height)) };
    });
}

}