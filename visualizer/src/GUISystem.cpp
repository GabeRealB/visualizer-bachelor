#include <visualizer/GUISystem.hpp>

#include <algorithm>
#include <imgui.h>
#include <variant>

#include <visualizer/Canvas.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/Visualizer.hpp>

namespace Visualizer {

void render_gui(EntityDatabaseContext&, const LegendGUI&);
void render_gui(EntityDatabaseContext&, CompositionGUI&);

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
        m_canvas_query.query_db_window(database_context).for_each<Canvas>([&](Canvas* canvas) {
            for (auto& gui : canvas->guis) {
                std::visit([&](auto&& g) { render_gui(database_context, g); }, gui);
            }
        });
    });
}

void render_legend_entry(EntityDatabaseContext&, const LegendGUIImage&);
void render_legend_entry(EntityDatabaseContext&, const LegendGUIColor&);

void render_gui(EntityDatabaseContext& database_context, const LegendGUI& gui)
{
    if (gui.entries.empty()) {
        return;
    }

    const float PAD = 10.0f;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->Pos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = (work_pos.x + gui.position.x + PAD);
    window_pos.y = (work_pos.y + gui.position.y + PAD);
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

void render_legend_entry(EntityDatabaseContext&, const LegendGUIImage& image_entry)
{
    assert(!image_entry.texture.expired());

    auto texture = image_entry.texture.lock();
    assert(texture->type() == TextureType::Texture2D);
    auto texture_2d = std::static_pointer_cast<const Texture2D>(texture);

    ImVec2 texture_size;
    if (image_entry.absolute) {
        texture_size = { static_cast<float>(texture_2d->width()), static_cast<float>(texture_2d->height()) };
    } else {
        auto texture_aspect = static_cast<float>(texture_2d->width()) / static_cast<float>(texture_2d->height());
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        texture_size = viewport->Size;
        texture_size.x = std::min(texture_size.x, texture_size.y);
        texture_size.y = texture_size.x / texture_aspect;
    }

    texture_size.x *= image_entry.scaling.x;
    texture_size.y *= image_entry.scaling.y;

    ImGui::Image(reinterpret_cast<void*>(static_cast<std::intptr_t>(texture_2d->id())), texture_size,
        ImVec2{ 0.0f, 0.0f }, ImVec2{ 1.0f, 1.0f });
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", image_entry.description.c_str());
    }
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

void render_gui(EntityDatabaseContext&, CompositionGUI& gui)
{
    if (gui.groups.empty()) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 window_pos = { gui.position.x, gui.position.y };
    auto window_size = viewport->Size;
    window_size.x *= gui.size.x;
    window_size.y *= gui.size.y;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(window_size);
    ImGui::SetNextWindowBgAlpha(0.0f);

    if (!ImGui::Begin("CompositionGUI", nullptr, window_flags)) {
        ImGui::End();
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && (gui.selected_group || gui.selected_window)) {
        gui.selected_group = 0;
        gui.selected_window = 0;
    }

    auto draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSplit(2);

    draw_list->ChannelsSetCurrent(1);
    // Display groups
    std::vector<std::array<ImVec2, 2>> group_rects{};
    group_rects.reserve(gui.groups.size());
    std::size_t group_selected = gui.selected_group;
    std::size_t window_selected = gui.selected_window;
    for (std::size_t i = 0; i < gui.groups.size(); i++) {
        auto& group = gui.groups[i];
        ImGui::PushID(group.group_name.c_str());

        ImVec2 min_pos = viewport->Size;
        ImVec2 max_pos = { 0.0f, 0.0f };
        ImVec2 content_size;

        bool group_created = false;
        for (std::size_t j = 0; j < group.windows.size(); j++) {
            auto& window = group.windows[j];
            assert(!window.texture.expired());
            ImGui::PushID(window.window_name.c_str());

            auto texture_2d = window.texture.lock();
            auto texture_aspect = static_cast<float>(texture_2d->width()) / static_cast<float>(texture_2d->height());

            auto texture_size = viewport->Size;
            texture_size.x = std::min(texture_size.x, texture_size.y);
            texture_size.y = texture_size.x / texture_aspect;
            texture_size.x *= window.scaling.x;
            texture_size.y *= window.scaling.y;

            auto screen_pos = viewport->Size;
            screen_pos.x *= window.position.x + group.position.x;
            screen_pos.y *= 1.0f - (window.position.y + group.position.y);
            screen_pos.y -= texture_size.y;

            constexpr float size_padding = 15.0f;
            auto max_screen_pos = ImVec2{ screen_pos.x + texture_size.x, screen_pos.y + texture_size.y };

            if (screen_pos.x < min_pos.x) {
                min_pos.x = screen_pos.x;
            }
            if (screen_pos.y < min_pos.y) {
                min_pos.y = screen_pos.y;
            }
            if (max_screen_pos.x > max_pos.x) {
                max_pos.x = max_screen_pos.x;
            }
            if (max_screen_pos.y + size_padding > max_pos.y) {
                max_pos.y = max_screen_pos.y + size_padding;
            }
            content_size = { max_screen_pos.x - min_pos.x, max_screen_pos.y - min_pos.y };

            ImGui::SetCursorScreenPos(screen_pos);

            if (!group_created) {
                ImGui::BeginGroup();
                group_created = true;
            }

            auto start_uv = window.flip_vertically ? ImVec2{ 0.0f, 1.0f } : ImVec2{ 0.0f, 0.0f };
            auto end_uv = window.flip_vertically ? ImVec2{ 1.0f, 0.0f } : ImVec2{ 1.0f, 1.0f };

            ImGui::BeginGroup();
            ImGui::Image(
                reinterpret_cast<void*>(static_cast<std::intptr_t>(texture_2d->id())), texture_size, start_uv, end_uv);
            ImGui::Text("%s", window.window_name.c_str());
            ImGui::EndGroup();

            draw_list->ChannelsSetCurrent(0);
            ImGui::SetCursorScreenPos(min_pos);
            ImGui::InvisibleButton("window", content_size);
            if (ImGui::IsMouseHoveringRect(screen_pos, max_screen_pos) && gui.selected_group == 0
                && gui.selected_window == 0) {
                window_selected = j + 1;

                if (group.windows.size() == 1) {
                    group_selected = i + 1;
                }
            }
            draw_list->ChannelsSetCurrent(1);

            ImGui::PopID();
        }
        if (group_created) {
            ImGui::EndGroup();
        }

        constexpr float rect_padding = 5.0f;
        auto rect_min = min_pos;
        auto rect_max = max_pos;

        rect_min.x -= rect_padding;
        rect_min.y -= rect_padding;

        rect_max.x += rect_padding;
        rect_max.y += rect_padding;

        if (!group.transparent) {
            ImGui::SetCursorScreenPos({ rect_min.x, rect_min.y - 15.0f });
            ImGui::Text("%s", group.group_name.c_str());
            draw_list->AddRect(rect_min, rect_max, IM_COL32(255, 255, 255, 255));
            if (ImGui::IsMouseHoveringRect(rect_min, rect_max) && group_selected == 0 && gui.selected_group == 0
                && gui.selected_window == 0) {
                group_selected = i + 1;
            }
        }

        group_rects.push_back({ rect_min, rect_max });

        ImGui::PopID();
    }

    if (group_selected != 0 && isDetached() && !is_frozen()) {
        glm::vec2 mouse_delta = { io.MouseDelta.x, -io.MouseDelta.y };
        mouse_delta.x /= viewport->Size.x;
        mouse_delta.y /= viewport->Size.y;

        auto& group = gui.groups[group_selected - 1];

        if (window_selected == 0) {
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                group.position += mouse_delta;
            }
        } else {
            auto mouse_wheel = io.MouseWheel;
            constexpr float scaling_stepping = 0.01f;
            auto& window = group.windows[window_selected - 1];

            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                window.position += mouse_delta;
            }
            window.scaling.x += mouse_wheel * scaling_stepping;
            window.scaling.y += mouse_wheel * scaling_stepping;

            constexpr float min_size = 0.05f;

            if (window.scaling.x <= min_size) {
                window.scaling.x = min_size;
            }
            if (window.scaling.y <= min_size) {
                window.scaling.y = min_size;
            }
        }
    }

    // Display links
    draw_list->ChannelsSetCurrent(0);
    for (auto& link : gui.group_connections) {
        auto& src_rect = group_rects[link[0]];
        auto& dst_rect = group_rects[link[1]];

        ImVec2 src_mid = { (src_rect[0].x + src_rect[1].x) / 2.0f, (src_rect[0].y + src_rect[1].y) / 2.0f };
        ImVec2 dst_mid = { (dst_rect[0].x + dst_rect[1].x) / 2.0f, (dst_rect[0].y + dst_rect[1].y) / 2.0f };

        ImVec2 link_start;
        ImVec2 link_end;
        ImVec2 p1;
        ImVec2 p2;

        ImVec2 arrow_p1;
        ImVec2 arrow_p2;

        if (dst_mid.x >= src_rect[0].x && dst_mid.x <= src_rect[1].x) {
            if (dst_rect[1].y <= src_rect[0].y) {
                link_start = { src_mid.x, src_rect[0].y };
                link_end = { dst_mid.x, dst_rect[1].y };
                p1 = { link_start.x, link_start.y - 50.0f };
                p2 = { link_end.x, link_end.y + 50.0f };
                arrow_p1 = { link_end.x - 7.5f, link_end.y + 15.0f };
                arrow_p2 = { link_end.x + 7.5f, link_end.y + 15.0f };
            } else {
                link_start = { src_mid.x, src_rect[1].y };
                link_end = { dst_mid.x, dst_rect[0].y };
                p1 = { link_start.x, link_start.y + 50.0f };
                p2 = { link_end.x, link_end.y - 50.0f };
                arrow_p1 = { link_end.x - 7.5f, link_end.y - 15.0f };
                arrow_p2 = { link_end.x + 7.5f, link_end.y - 15.0f };
            }
        } else if (dst_mid.x <= src_rect[0].x) {
            link_start = { src_rect[0].x, src_mid.y };
            link_end = { dst_rect[1].x, dst_mid.y };
            p1 = { link_start.x - 50.0f, link_start.y };
            p2 = { link_end.x + 50.0f, link_end.y };
            arrow_p1 = { link_end.x + 15.0f, link_end.y - 7.5f };
            arrow_p2 = { link_end.x + 15.0f, link_end.y + 7.5f };
        } else {
            link_start = { src_rect[1].x, src_mid.y };
            link_end = { dst_rect[0].x, dst_mid.y };
            p1 = { link_start.x + 50.0f, link_start.y };
            p2 = { link_end.x - 50.0f, link_end.y };
            arrow_p1 = { link_end.x - 15.0f, link_end.y - 7.5f };
            arrow_p2 = { link_end.x - 15.0f, link_end.y + 7.5f };
        }

        constexpr auto line_color = IM_COL32(200, 100, 0, 255);
        draw_list->AddLine(arrow_p1, link_end, line_color, 3.0f);
        draw_list->AddLine(arrow_p2, link_end, line_color, 3.0f);
        draw_list->AddBezierCurve(link_start, p1, p2, link_end, line_color, 3.0f);
    }

    gui.selected_group = group_selected;
    gui.selected_window = window_selected;

    draw_list->ChannelsMerge();
    ImGui::End();
}

}