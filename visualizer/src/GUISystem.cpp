#include <visualizer/GUISystem.hpp>

#include <imgui.h>
#include <variant>

#include <visualizer/Canvas.hpp>
#include <visualizer/Shader.hpp>

namespace Visualizer {

void render_gui(EntityDatabaseContext&, const LegendGUI&);

GUISystem::GUISystem()
    : m_canvas_query{ EntityDBQuery{}.with_component<Canvas>() }
    , m_entity_database{}
{
}

void GUISystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void GUISystem::terminate() { m_entity_database = nullptr; }

void GUISystem::run(void*)
{
    m_entity_database->enter_secure_context([&](EntityDatabaseContext& database_context) {
        m_canvas_query.query_db_window(database_context).for_each<Canvas>([&](const Canvas* canvas) {
            for (auto& gui : canvas->guis) {
                std::visit([&](auto&& g) { render_gui(database_context, g); }, gui);
            }
        });
    });
}

void render_legend_entry(EntityDatabaseContext&, const LegendGUIColor&);

void render_gui(EntityDatabaseContext& database_context, const LegendGUI& gui)
{
    const float PAD = 10.0f;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->Pos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = (work_pos.x + PAD);
    window_pos.y = (work_pos.y + PAD);
    window_pos_pivot.x = 0.0f;
    window_pos_pivot.y = 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    window_flags |= ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Legend", nullptr, window_flags)) {
        ImGui::Text("Legend");
        ImGui::Separator();
        for (auto& entry : gui.entries) {
            std::visit([&](auto&& entry) { render_legend_entry(database_context, entry); }, entry);
        }
    }
    ImGui::End();
}

void render_legend_entry(EntityDatabaseContext& database_context, const LegendGUIColor& color_entry)
{
    assert(database_context.entity_has_component<Material>(color_entry.entity));

    auto& material = database_context.fetch_component_unchecked<Material>(color_entry.entity);
    auto& color = *material.m_passes[color_entry.pass].m_material_variables.getPtr<glm::vec4>(color_entry.attribute, 1);

    ImVec4 im_color = { color.r, color.g, color.b, color.a };

    ImGui::ColorButton(color_entry.label.c_str(), im_color);
    ImGui::SameLine(40.0f);
    ImGui::Text("%s", color_entry.label.c_str());
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", color_entry.description.c_str());
    }
}

}