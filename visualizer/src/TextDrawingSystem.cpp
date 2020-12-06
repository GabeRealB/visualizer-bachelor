#include <visualizer/TextDrawingSystem.hpp>

#include <visualizer/Camera.hpp>
#include <visualizer/Transform.hpp>
#include <visualizer/UIText.hpp>

namespace Visualizer {

TextDrawingSystem::TextDrawingSystem()
    : m_textQuery{ EntityDBQuery{}.with_component<UIText, Transform>() }
    , m_cameraQuery{ EntityDBQuery{}.with_component<Camera>() }
    , m_bitmap_mesh{}
    , m_bitmap_material{}
    , m_characters{}
    , m_entity_database{}
{
    FT_Library freeType;
    FT_Init_FreeType(&freeType);

    FT_Face face{};
    FT_New_Face(freeType, "fonts/arial.ttf", 0, &face);
    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (std::size_t i = 0; i < 128; ++i) {
        if (FT_Load_Char(face, static_cast<FT_ULong>(i), FT_LOAD_RENDER)) {
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

void TextDrawingSystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void TextDrawingSystem::terminate() { m_entity_database = nullptr; }

void TextDrawingSystem::run(void*)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_entity_database->enter_secure_context([&](EntityDatabaseContext& database_context) {
        auto textUIs{ m_textQuery.query_db_window(database_context) };

        m_cameraQuery.query_db_window(database_context).for_each<const Camera>([&](const Camera* camera) {
            camera->m_renderTarget->bind(FramebufferBinding::ReadWrite);
            auto cameraViewport{ camera->m_renderTarget->viewport() };

            auto projectionMatrix{ glm::ortho(
                0.0f, static_cast<float>(cameraViewport.width), 0.0f, static_cast<float>(cameraViewport.height)) };
            (void)projectionMatrix;
        });
    });
}

}