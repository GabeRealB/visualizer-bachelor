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
    // glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glDepthMask(GL_FALSE);
    glDepthFunc(GL_NOTEQUAL);

    auto drawableMeshes{ m_meshQuery.query(*m_componentManager) };

    m_cameraQuery.query(*m_componentManager).forEach<Camera, Transform>([&](Camera* camera, Transform* transform) {
        camera->m_renderTargets["cube"]->bind(FramebufferBinding::ReadWrite);
        auto cameraViewport{ camera->m_renderTargets["cube"]->viewport() };

        if (camera->m_active) {
            glClearColor(0.4f, 0.05f, 0.05f, 1.0f);
        } else {
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_SCISSOR_TEST);
        glScissor(cameraViewport.x + 10, cameraViewport.y + 10, cameraViewport.width - 20, cameraViewport.height - 20);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto viewMatrix{ glm::identity<glm::mat4>() };
        viewMatrix = glm::toMat4(glm::inverse(transform->rotation)) * glm::translate(viewMatrix, -transform->position);

        auto projectionMatrix{ glm::identity<glm::mat4>() };
        if (camera->perspective) {
            projectionMatrix = glm::perspective(camera->fov, camera->aspect, camera->near, camera->far);
        } else {
            projectionMatrix = glm::ortho(-camera->orthographicWidth / 2.0f, camera->orthographicWidth / 2.0f,
                -camera->orthographicHeight / 2.0f, camera->orthographicHeight / 2.0f, -camera->far / 2.0f,
                camera->far / 2.0f);
        }
        auto viewProjectionMatrix = projectionMatrix * viewMatrix;

        ShaderEnvironment cameraVariables{};
        std::shared_ptr<ShaderProgram> lastProgram{ nullptr };

        std::vector<std::tuple<Entity, const std::shared_ptr<Mesh>*, const Material*, glm::mat4>> meshList{};

        drawableMeshes.forEachWithEntity<const std::shared_ptr<Mesh>, const Material, const Transform,
            const RenderLayer>(
            [&](Entity entity, const std::shared_ptr<Mesh>* mesh, const Material* material, const Transform* transform,
                const RenderLayer*) {
                auto modelMatrix{ getModelMatrix(*transform) };

                const Parent* parent;
                if (m_componentManager->has_component(entity, getTypeId<Parent>())) {
                    parent = static_cast<const Parent*>(
                        m_componentManager->getEntityComponentPointer(entity, getTypeId<Parent>()));
                } else {
                    parent = nullptr;
                }

                while (parent != nullptr) {
                    auto parentTransform{ static_cast<const Transform*>(
                        m_componentManager->getEntityComponentPointer(parent->m_parent, getTypeId<Transform>())) };
                    modelMatrix = getModelMatrix(*parentTransform) * modelMatrix;
                    if (m_componentManager->has_component(parent->m_parent, getTypeId<Parent>())) {
                        parent = static_cast<const Parent*>(
                            m_componentManager->getEntityComponentPointer(parent->m_parent, getTypeId<Parent>()));
                    } else {
                        parent = nullptr;
                    }
                }

                meshList.insert(
                    std::upper_bound(meshList.begin(), meshList.end(), entity,
                        [](const Entity& entity,
                            const std::tuple<Entity, const std::shared_ptr<Mesh>*, const Material*, glm::mat4>& tup) {
                            return std::get<0>(tup).id > entity.id;
                        }),
                    { entity, mesh, material, modelMatrix });
            },
            [&](Entity, const std::shared_ptr<Mesh>*, const Material*, const Transform*,
                const RenderLayer* layer) -> bool { return (*layer & camera->m_visibleLayers); });

        for (auto& meshInfo : meshList) {
            auto mesh{ std::get<1>(meshInfo) };
            auto material{ std::get<2>(meshInfo) };
            auto modelMatrix{ std::get<3>(meshInfo) };

            if (lastProgram != material->m_shader) {
                lastProgram = material->m_shader;
                cameraVariables = ShaderEnvironment{ *material->m_shader, ParameterQualifier::Program };
                cameraVariables.set("viewProjectionMatrix", viewProjectionMatrix);
                material->m_shader->bind();
            }

            cameraVariables.set("modelMatrix", modelMatrix);

            lastProgram->apply(cameraVariables);
            lastProgram->apply(material->m_materialVariables);

            auto tmp{ mesh->get() };
            tmp->bind();
            glDrawElements(tmp->primitiveType(), static_cast<GLsizei>(tmp->getIndexCount()), tmp->indexType(), nullptr);
            tmp->unbind();
        }

        if (lastProgram != nullptr) {
            lastProgram->unbind();
        }

        glDisable(GL_SCISSOR_TEST);
    });

    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void MeshDrawingSystem::initialize() { m_componentManager = m_world->getManager<ComponentManager>(); }
void MeshDrawingSystem::terminate() { m_componentManager = nullptr; }

}