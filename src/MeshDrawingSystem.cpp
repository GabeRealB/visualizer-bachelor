#include <visualizer/MeshDrawingSystem.hpp>

#include <memory>

#include <visualizer/Camera.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

MeshDrawingSystem::MeshDrawingSystem(std::shared_ptr<Texture2D> texture)
    : m_meshQuery{ EntityQuery{}.with<std::shared_ptr<Mesh>, Material, Transform>() }
    , m_cameraQuery{ EntityQuery{}.with<Camera, Transform>() }
    , m_frameBuffer{}
    , m_componentManager{}
{
    m_frameBuffer.attachBuffer(FramebufferAttachment::Color0, std::move(texture));
}

void MeshDrawingSystem::run(void*)
{
    m_frameBuffer.bind(FramebufferBinding::ReadWrite);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto drawableMeshes{ m_meshQuery.query(*m_componentManager) };

    m_cameraQuery.query(*m_componentManager).forEach<Camera, Transform>([&](Camera* camera, Transform* transform) {
        auto viewMatrix{ glm::identity<glm::mat4>() };
        viewMatrix = glm::toMat4(glm::inverse(transform->rotation)) * glm::translate(viewMatrix, -transform->position);
        auto viewProjectionMatrix = camera->projectionMatrix * viewMatrix;

        ShaderEnvironment cameraVariables{};
        std::shared_ptr<ShaderProgram> lastProgram{ nullptr };

        drawableMeshes.forEach<std::shared_ptr<Mesh>, Material, const Transform>(
            [&](std::shared_ptr<Mesh>* mesh, Material* material, const Transform* transform) {
                if (lastProgram != material->m_shader) {
                    lastProgram = material->m_shader;
                    cameraVariables = ShaderEnvironment{ *material->m_shader, ParameterQualifier::Program };
                    cameraVariables.set("viewProjectionMatrix", viewProjectionMatrix);
                    material->m_shader->bind();
                }

                cameraVariables.set("modelMatrix", getModelMatrix(*transform));
                lastProgram->apply(cameraVariables);
                lastProgram->apply(material->m_materialVariables);

                auto tmp{ mesh->get() };
                tmp->bind();
                glDrawElements(tmp->primitiveType(), tmp->getIndexCount(), tmp->indexType(), nullptr);
                tmp->unbind();
            });

        if (lastProgram != nullptr) {
            lastProgram->unbind();
        }
    });

    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    m_frameBuffer.unbind(FramebufferBinding::ReadWrite);
}

void MeshDrawingSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }
void MeshDrawingSystem::terminate() { m_componentManager = nullptr; }

}