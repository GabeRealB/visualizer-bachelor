#include <visualizer/MeshDrawingSystem.hpp>

#include <memory>

#include <visualizer/Camera.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Parent.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

MeshDrawingSystem::MeshDrawingSystem()
    : m_mesh_query{ EntityDBQuery{}.with_component<std::shared_ptr<Mesh>, Material, Transform, RenderLayer>() }
    , m_camera_query{ EntityDBQuery{}.with_component<Camera, Transform>() }
    , m_entity_database{}
{
}

void MeshDrawingSystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void MeshDrawingSystem::terminate() { m_entity_database = nullptr; }

void MeshDrawingSystem::run(void*)
{
    glEnable(GL_BLEND);
    // glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glDepthMask(GL_FALSE);
    glDepthFunc(GL_NOTEQUAL);

    m_entity_database->enter_secure_context([&](EntityDatabaseContext& database_context) {
        auto drawable_meshes{ m_mesh_query.query_db_window(database_context) };

        m_camera_query.query_db_window(database_context)
            .for_each<Camera, Transform>([&](Camera* camera, Transform* transform) {
                camera->m_renderTargets["cube"]->bind(FramebufferBinding::ReadWrite);
                auto camera_viewport{ camera->m_renderTargets["cube"]->viewport() };

                if (camera->m_active) {
                    glClearColor(0.4f, 0.05f, 0.05f, 1.0f);
                } else {
                    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
                }
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glEnable(GL_SCISSOR_TEST);
                glScissor(camera_viewport.x + 10, camera_viewport.y + 10, camera_viewport.width - 20,
                    camera_viewport.height - 20);
                glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                auto view_matrix{ glm::identity<glm::mat4>() };
                view_matrix = glm::toMat4(glm::inverse(transform->rotation))
                    * glm::translate(view_matrix, -transform->position);

                auto projection_matrix{ glm::identity<glm::mat4>() };
                if (camera->perspective) {
                    projection_matrix = glm::perspective(camera->fov, camera->aspect, camera->near, camera->far);
                } else {
                    projection_matrix = glm::ortho(-camera->orthographicWidth / 2.0f, camera->orthographicWidth / 2.0f,
                        -camera->orthographicHeight / 2.0f, camera->orthographicHeight / 2.0f, -camera->far / 2.0f,
                        camera->far / 2.0f);
                }
                auto view_projection_matrix = projection_matrix * view_matrix;

                ShaderEnvironment camera_variables{};
                std::shared_ptr<ShaderProgram> last_program{ nullptr };

                std::vector<std::tuple<Entity, const std::shared_ptr<Mesh>*, const Material*, glm::mat4>> mesh_list{};

                drawable_meshes.for_each<const std::shared_ptr<Mesh>, const Material, const Transform,
                    const RenderLayer>(
                    [&](Entity entity, const std::shared_ptr<Mesh>* mesh, const Material* material,
                        const Transform* transform, const RenderLayer*) {
                        auto model_matrix{ getModelMatrix(*transform) };

                        for (auto parent_entity{ entity };
                             database_context.entity_has_component<Parent>(parent_entity);) {
                            const auto& parent{ database_context.fetch_component_unchecked<Parent>(parent_entity) };
                            model_matrix
                                = getModelMatrix(database_context.fetch_component_unchecked<Transform>(parent.m_parent))
                                * model_matrix;
                            parent_entity = parent.m_parent;
                        }

                        mesh_list.insert(
                            std::upper_bound(mesh_list.begin(), mesh_list.end(), entity,
                                [](const Entity& entity,
                                    const std::tuple<Entity, const std::shared_ptr<Mesh>*, const Material*, glm::mat4>&
                                        tup) { return std::get<0>(tup).id > entity.id; }),
                            { entity, mesh, material, model_matrix });
                    },
                    [&](Entity, const std::shared_ptr<Mesh>*, const Material*, const Transform*,
                        const RenderLayer* layer) -> bool { return (*layer & camera->m_visibleLayers); });

                for (auto& mesh_info : mesh_list) {
                    auto& mesh{ std::get<1>(mesh_info) };
                    auto& material{ std::get<2>(mesh_info) };
                    auto& model_matrix{ std::get<3>(mesh_info) };

                    if (last_program != material->m_shader) {
                        last_program = material->m_shader;
                        camera_variables = ShaderEnvironment{ *material->m_shader, ParameterQualifier::Program };
                        camera_variables.set("viewProjectionMatrix", view_projection_matrix);
                        material->m_shader->bind();
                    }

                    camera_variables.set("modelMatrix", model_matrix);

                    last_program->apply(camera_variables);
                    last_program->apply(material->m_materialVariables);

                    auto tmp{ mesh->get() };
                    tmp->bind();
                    glDrawElements(
                        tmp->primitiveType(), static_cast<GLsizei>(tmp->getIndexCount()), tmp->indexType(), nullptr);
                    tmp->unbind();
                }

                if (last_program != nullptr) {
                    last_program->unbind();
                }

                glDisable(GL_SCISSOR_TEST);
            });
    });

    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

}