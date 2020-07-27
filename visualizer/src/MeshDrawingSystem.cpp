#include <visualizer/MeshDrawingSystem.hpp>

#include <memory>

#include <visualizer/Camera.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Parent.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

MeshDrawingSystem::MeshDrawingSystem()
    : m_meshQuery{ EntityQuery{}.with<std::shared_ptr<Mesh>, Material, Transform, RenderLayer>() }
    , m_cameraQuery{ EntityQuery{}.with<Camera, Transform>() }
    , m_componentManager{}
{
}

void MeshDrawingSystem::run(void*)
{
    glEnable(GL_BLEND);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto drawableMeshes{ m_meshQuery.query(*m_componentManager) };

    m_cameraQuery.query(*m_componentManager).forEach<Camera, Transform>([&](Camera* camera, Transform* transform) {
        camera->m_renderTarget->bind(FramebufferBinding::ReadWrite);
        auto cameraViewport{ camera->m_renderTarget->viewport() };
        glClear(GL_COLOR_BUFFER_BIT);

        auto viewMatrix{ glm::identity<glm::mat4>() };
        viewMatrix = glm::toMat4(glm::inverse(transform->rotation)) * glm::translate(viewMatrix, -transform->position);
        auto projectionMatrix{ glm::perspective(70.0f,
            static_cast<float>(cameraViewport.width) / static_cast<float>(cameraViewport.height), 0.3f, 1000.0f) };
        auto viewProjectionMatrix = projectionMatrix * viewMatrix;

        ShaderEnvironment cameraVariables{};
        std::shared_ptr<ShaderProgram> lastProgram{ nullptr };

        drawableMeshes
            .forEachWithEntity<const std::shared_ptr<Mesh>, const Material, const Transform, const RenderLayer>(
                [&](Entity entity, const std::shared_ptr<Mesh>* mesh, const Material* material,
                    const Transform* transform, const RenderLayer*) {
                    if (lastProgram != material->m_shader) {
                        lastProgram = material->m_shader;
                        cameraVariables = ShaderEnvironment{ *material->m_shader, ParameterQualifier::Program };
                        cameraVariables.set("viewProjectionMatrix", viewProjectionMatrix);
                        material->m_shader->bind();
                    }

                    auto modelMatrix{ getModelMatrix(*transform) };
                    auto parent{ static_cast<const Parent*>(
                        m_componentManager->getEntityComponentPointer(entity, getTypeId<Parent>())) };

                    while (parent != nullptr) {
                        auto parentTransform{ static_cast<const Transform*>(
                            m_componentManager->getEntityComponentPointer(parent->m_parent, getTypeId<Transform>())) };
                        modelMatrix = getModelMatrix(*parentTransform) * modelMatrix;
                        parent = static_cast<const Parent*>(
                            m_componentManager->getEntityComponentPointer(parent->m_parent, getTypeId<Parent>()));
                    }
                    cameraVariables.set("modelMatrix", modelMatrix);

                    lastProgram->apply(cameraVariables);
                    lastProgram->apply(material->m_materialVariables);

                    auto tmp{ mesh->get() };
                    tmp->bind();
                    glDrawElements(
                        tmp->primitiveType(), static_cast<GLsizei>(tmp->getIndexCount()), tmp->indexType(), nullptr);
                    tmp->unbind();
                },
                [&](Entity, const std::shared_ptr<Mesh>*, const Material*, const Transform*,
                    const RenderLayer* layer) -> bool { return (*layer & camera->m_visibleLayers); });

        if (lastProgram != nullptr) {
            lastProgram->unbind();
        }
    });

    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
}

void MeshDrawingSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }
void MeshDrawingSystem::terminate() { m_componentManager = nullptr; }

}