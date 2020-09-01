#include <visualizer/CompositingSystem.hpp>

#include <algorithm>

#include <visualizer/Composition.hpp>

namespace Visualizer {

CompositingSystem::CompositingSystem()
    : m_quad{}
    , m_entityQuery{ EntityQuery{}.with<Composition>() }
    , m_componentManager{}
{
    GLuint indices[]{ 0, 1, 2, 0, 2, 3 };
    glm::vec4 vertices[]{
        { -1.0f, -1.0f, 0.0f, 0.0f },
        { 1.0f, -1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f, 0.0f },
        { -1.0f, 1.0f, 0.0f, 0.0f },
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
}

void CompositingSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }

void CompositingSystem::terminate() { m_componentManager = nullptr; }

void CompositingSystem::run(void*)
{
    Framebuffer::defaultFramebuffer().bind(FramebufferBinding::ReadWrite);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_quad.bind();

    m_entityQuery.query(*m_componentManager).forEach<Composition>([&](Composition* composition) {
        for (auto& operation : composition->operations) {
            operation.material.m_shader->bind();
            operation.material.m_materialVariables.set("transMatrix", getModelMatrix(operation.transform));

            std::size_t i{ 0 };
            for (const auto& source : operation.source) {
                TextureSampler<Texture2D> sampler{ source, static_cast<TextureSlot>(i) };
                operation.material.m_materialVariables.set("view_" + std::to_string(i), sampler);
                i++;
            }

            operation.material.m_shader->apply(operation.material.m_materialVariables);

            operation.destination->bind(FramebufferBinding::ReadWrite);

            glDrawElements(
                m_quad.primitiveType(), static_cast<GLsizei>(m_quad.getIndexCount()), m_quad.indexType(), nullptr);

            operation.material.m_shader->unbind();
        }
    });

    m_quad.unbind();
}

}