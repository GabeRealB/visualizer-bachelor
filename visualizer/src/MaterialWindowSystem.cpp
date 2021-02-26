#include <visualizer/MaterialWindowSystem.hpp>

#if __has_include(<glad\glad.h>)
#include <glad\glad.h>
#else
#include <glad.h>
#endif

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <visualizer/Camera.hpp>
#include <visualizer/RenderLayer.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/Visualizer.hpp>

namespace Visualizer {

MaterialWindowSystem::MaterialWindowSystem()
    : m_active{ false }
    , m_camera_query{ EntityDBQuery{}.with_component<Camera>() }
    , m_materials_query{ EntityDBQuery{}.with_component<Material, RenderLayer>() }
    , m_entity_database{}
{
}

void MaterialWindowSystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void MaterialWindowSystem::terminate() { m_entity_database = nullptr; }

void MaterialWindowSystem::run(void*)
{
    auto window{ glfwGetCurrentContext() };
    auto m_key{ glfwGetKey(window, GLFW_KEY_M) };

    static bool m_pressed = false;

    if (m_key == GLFW_PRESS) {
        m_pressed = true;
    }

    if (m_key == GLFW_RELEASE && m_pressed) {
        if (m_active) {
            freeze(false);
        }
        m_pressed = false;
        m_active = true;
        freeze(true);
    }

    if (m_active) {
        ImGui::Begin("Materials", &m_active);

        m_entity_database->enter_secure_context([&](EntityDatabaseContext& database_context) {
            auto camera_pred = [](const Camera* camera) { return camera->m_active; };
            m_camera_query.query_db_window(database_context)
                .for_each<Camera>(
                    [&](Camera* camera) {
                        auto materials_pred = [&](Entity, const Material*, const RenderLayer* layer) {
                            return *layer & camera->m_visibleLayers;
                        };
                        m_materials_query.query_db_window(database_context)
                            .for_each<Material, RenderLayer>(
                                [&](Entity entity, Material* material, RenderLayer*) {
                                    auto header_name = "Entity: Id. " + std::to_string(entity.id) + " Gen. "
                                        + std::to_string(entity.generation);
                                    if (ImGui::CollapsingHeader(header_name.c_str())) {
                                        ImGui::Indent(10.0f);
                                        ImGui::Text("Pipeline: %s", material->m_pipeline.c_str());
                                        ImGui::Text("Passes: %zu", material->m_passes.size());
                                        ImGui::Spacing();

                                        for (std::size_t i = 0; i < material->m_passes.size(); i++) {
                                            ImGui::PushID(i);
                                            auto header_label = "Pass " + std::to_string(i) + "##" + header_name;
                                            if (ImGui::CollapsingHeader(header_label.c_str())) {
                                                ImGui::Indent(10.0f);
                                                ImGui::Columns(2);
                                                for (auto parameter :
                                                    material->m_passes[i].m_material_variables.parameters()) {
                                                    ImGui::BulletText("Name: %s", parameter.data());
                                                    ImGui::NextColumn();

                                                    switch (material->m_passes[i].m_material_variables.parameter_type(
                                                        parameter)) {
                                                    case ParameterType::Vec4: {
                                                        auto val
                                                            = material->m_passes[i]
                                                                  .m_material_variables.getPtr<glm::vec4>(parameter, 1);
                                                        auto label
                                                            = "Color##" + header_label + std::string{ parameter };
                                                        ImGui::ColorEdit4(label.c_str(), glm::value_ptr(*val));
                                                        break;
                                                    }
                                                    default:
                                                        ImGui::Text("No preview available");
                                                        break;
                                                    }

                                                    ImGui::NextColumn();
                                                }
                                                ImGui::Columns(1);
                                                ImGui::Unindent(10.0f);
                                            }
                                            ImGui::PopID();
                                        }

                                        ImGui::Unindent(10.0f);
                                    }
                                },
                                materials_pred);
                    },
                    camera_pred);
        });

        ImGui::End();

        if (!m_active) {
            freeze(false);
        }
    }
}

}
