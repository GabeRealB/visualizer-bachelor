#include <visualizer/CompositingSystem.hpp>

#include <algorithm>

#include <visualizer/Framebuffer.hpp>

namespace Visualizer {

CompositingSystem::CompositingSystem(
    const VisualizerConfiguration& config, std::vector<std::shared_ptr<Texture2D>> views)
    : m_quad{}
    , m_shaderEnvironment{}
    , m_viewTransforms{}
    , m_program{}
    , m_views{ std::move(views) }
{
    GLuint indices[]{ 0, 1, 2, 0, 2, 3 };
    glm::vec4 vertices[]{
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
    };

    glm::vec4 uvs[]{
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
    };

    m_quad.setVertices(vertices, 4);
    m_quad.setTextureCoordinates0(uvs, 4);
    m_quad.setIndices(indices, 6, GL_TRIANGLES);

    auto vShader{ Shader::create("shader/compositing.vs.glsl", ShaderType::VertexShader) };
    auto fShader{ Shader::create("shader/compositing.fs.glsl", ShaderType::FragmentShader) };
    m_program = ShaderProgram::create(*vShader, *fShader);
    m_shaderEnvironment = ShaderEnvironment{ *m_program, ParameterQualifier::Program };

    m_viewTransforms.reserve(m_views.size());
    for (auto& view : config.views) {
        m_viewTransforms.push_back(Transform{
            glm::vec3{ 0.0f, 0.0f, 0.0f },
            glm::vec3{ (2 * view.position[0]) - 1, (2 * view.position[1]) - 1, 0.0f },
            glm::vec3{ view.size[0] * 2, view.size[0] * 2, 1.0f },
        });
    }
}

void CompositingSystem::initialize() {}

void CompositingSystem::terminate() {}

void CompositingSystem::run(void*)
{
    Framebuffer::defaultFramebuffer().bind(FramebufferBinding::ReadWrite);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_quad.bind();
    m_program->bind();

    for (std::size_t i = 0; i < m_views.size(); ++i) {
        TextureSampler<Texture2D> sampler{ m_views[i], TextureSlot::Slot0 };
        m_shaderEnvironment.set("transMatrix", getModelMatrix(m_viewTransforms[i]));
        m_shaderEnvironment.set("view", sampler);

        m_program->apply(m_shaderEnvironment);

        glDrawElements(
            m_quad.primitiveType(), static_cast<GLsizei>(m_quad.getIndexCount()), m_quad.indexType(), nullptr);
    }

    m_program->unbind();
    m_quad.unbind();
}

}