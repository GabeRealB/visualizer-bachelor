#include <visualizer/CompositingSystem.hpp>

#include <algorithm>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <visualizer/Composition.hpp>
#include <visualizer/Visualizer.hpp>

namespace Visualizer {

CompositingSystem::CompositingSystem()
    : m_quad{}
    , m_copyEntityQuery{ EntityQuery{}.with<Copy>() }
    , m_draggableEntityQuery{ EntityQuery{}.with<Composition, Draggable>() }
    , m_compositionEntityQuery{ EntityQuery{}.with<Composition>() }
    , m_selected{ std::nullopt }
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

    if (isDetached()) {
        auto queryResult{ m_draggableEntityQuery.query(*m_componentManager) };

        auto window{ glfwGetCurrentContext() };
        auto mouseButtonState{ glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) };
        if (mouseButtonState == GLFW_PRESS) {
            double mouseX;
            double mouseY;

            getRelativeMousePosition(mouseX, mouseY);

            if (!m_selected) {
                queryResult.forEach<Draggable>([&](Draggable* draggable) {
                    for (auto& box : draggable->boxes) {
                        if (mouseX >= box.xStart && mouseX <= box.xEnd && mouseY <= box.yStart && mouseY >= box.yEnd) {
                            m_selected = box.id;
                        }
                    }
                });
            } else {
                queryResult.forEach<Composition, Draggable>([&](Composition* composition, Draggable* draggable) {
                    for (auto& operation : composition->operations) {
                        if (operation.id == m_selected.value()) {
                            operation.transform.position.x = static_cast<float>(mouseX);
                            operation.transform.position.y = static_cast<float>(mouseY);

                            auto scale{ operation.transform.scale };
                            auto minPos{ operation.transform.position - scale };
                            auto maxPos{ operation.transform.position + scale };

                            if (minPos.x <= -1.0f) {
                                operation.transform.position.x = -1.0f + scale.x;
                            } else if (maxPos.x >= 1.0f) {
                                operation.transform.position.x = 1.0f - scale.x;
                            }

                            if (minPos.y <= -1.0f) {
                                operation.transform.position.y = -1.0f + scale.y;
                            } else if (maxPos.y >= 1.0f) {
                                operation.transform.position.y = 1.0f - scale.y;
                            }

                            for (auto& box : draggable->boxes) {
                                if (box.id == m_selected.value()) {
                                    box.xStart = operation.transform.position.x - scale.x;
                                    box.xEnd = operation.transform.position.x + scale.x;
                                    box.yStart = operation.transform.position.y + scale.y;
                                    box.yEnd = operation.transform.position.y - scale.y;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                });
            }
        } else if (mouseButtonState == GLFW_RELEASE) {
            m_selected = std::nullopt;
        }
    }

    m_copyEntityQuery.query(*m_componentManager).forEach<Copy>([](Copy* copy) {
        for (auto& operation : copy->operations) {
            operation.source->copyTo(*operation.destination, operation.flags, operation.filter);
        }
    });

    m_compositionEntityQuery.query(*m_componentManager).forEach<Composition>([&](Composition* composition) {
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