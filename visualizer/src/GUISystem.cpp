#include <visualizer/GUISystem.hpp>

#if __has_include(<glad\glad.h>)
#include <glad\glad.h>
#else
#include <glad.h>
#endif

#include <GLFW/glfw3.h>
#include <algorithm>
#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <sstream>
#include <variant>

#include <visualizer/Canvas.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/Visualizer.hpp>

namespace Visualizer {

void render_gui(EntityDatabaseContext&, const Canvas&, const LegendGUI&);
void render_gui(EntityDatabaseContext&, const Canvas&, CompositionGUI&);
void render_gui(EntityDatabaseContext&, const Canvas&, ConfigDumpGUI&);

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
                std::visit([&](auto&& g) { render_gui(database_context, *canvas, g); }, gui);
            }
        });
    });
}

void render_legend_entry(EntityDatabaseContext&, const LegendGUIImage&);
void render_legend_entry(EntityDatabaseContext&, const LegendGUIColor&);

void render_gui(EntityDatabaseContext& database_context, const Canvas&, const LegendGUI& gui)
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

void render_legend_entry(EntityDatabaseContext&, const LegendGUIColor& color_entry)
{
    ImVec4 im_color = {
        color_entry.color.r,
        color_entry.color.g,
        color_entry.color.b,
        color_entry.color.a,
    };
    ImVec4 im_caption_color = {
        color_entry.caption_color.r,
        color_entry.caption_color.g,
        color_entry.caption_color.b,
        color_entry.caption_color.a,
    };

    ImGui::ColorButton(color_entry.label.c_str(), im_color);
    ImGui::SameLine(40.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, im_caption_color);
    ImGui::Text("%s", color_entry.caption.c_str());
    ImGui::PopStyleColor();
}

void render_gui(EntityDatabaseContext&, const Canvas&, CompositionGUI& gui)
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

    ImVec4 color = { gui.background_color.r, gui.background_color.g, gui.background_color.b, gui.background_color.a };

    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(window_size);
    ImGui::SetNextWindowBgAlpha(gui.background_color.a);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, color);

    if (!ImGui::Begin("CompositionGUI", nullptr, window_flags)) {
        ImGui::End();
    }

    ImGui::PopStyleColor();

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

            ImVec4 window_caption_color = {
                window.caption_color.r,
                window.caption_color.g,
                window.caption_color.b,
                window.caption_color.a,
            };

            ImGui::BeginGroup();
            ImGui::PushStyleColor(ImGuiCol_Text, window_caption_color);
            ImGui::Image(
                reinterpret_cast<void*>(static_cast<std::intptr_t>(texture_2d->id())), texture_size, start_uv, end_uv);
            ImGui::Text("%s", window.window_name.c_str());
            ImGui::PopStyleColor();
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
            ImVec4 group_caption_color = {
                group.caption_color.r,
                group.caption_color.g,
                group.caption_color.b,
                group.caption_color.a,
            };

            ImGui::SetCursorScreenPos({ rect_min.x, rect_min.y - 15.0f });
            ImGui::PushStyleColor(ImGuiCol_Text, group_caption_color);
            ImGui::Text("%s", group.group_name.c_str());
            ImGui::PopStyleColor();
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

bool replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void render_gui(EntityDatabaseContext& database_context, const Canvas& canvas, ConfigDumpGUI& gui)
{
    if (!gui.active) {
        auto window{ glfwGetCurrentContext() };
        auto n_key{ glfwGetKey(window, GLFW_KEY_N) };

        if (n_key == GLFW_RELEASE && gui.n_key_state == GLFW_PRESS) {
            gui.active = true;
            freeze(true);
        }
        gui.n_key_state = n_key;
    }

    if (gui.active) {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
        if (!ImGui::Begin("Config dump", &gui.active, window_flags)) {
            ImGui::End();
            gui.active = false;
            freeze(false);
            return;
        }

        if (ImGui::Button("Dump config!")) {
            const auto& composition_gui = [&]() -> auto&
            {
                for (auto& gui : canvas.guis) {
                    if (std::holds_alternative<CompositionGUI>(gui)) {
                        return std::get<CompositionGUI>(gui);
                    }
                }

                std::abort();
                return std::get<CompositionGUI>(canvas.guis.front());
            }
            ();

            std::string config_dump = gui.config_template;

            for (auto& group : composition_gui.groups) {
                std::ostringstream group_position_key_ss{};
                group_position_key_ss << R"(")" << group.group_id << "_position"
                                      << R"(")";

                std::ostringstream position_ss{};
                position_ss << "[" << group.position.x << "," << group.position.y << "]";

                if (replace(config_dump, group_position_key_ss.str(), position_ss.str())) {
                    for (auto& composition_window : group.windows) {
                        std::ostringstream window_position_key_ss{};
                        window_position_key_ss << R"(")" << composition_window.window_id << "_position"
                                               << R"(")";

                        std::ostringstream window_scale_key_ss{};
                        window_scale_key_ss << R"(")" << composition_window.window_id << "_scale"
                                            << R"(")";

                        auto window_position = composition_window.position + group.position;

                        std::ostringstream window_position_ss{};
                        window_position_ss << "[" << window_position.x << "," << window_position.y << "]";

                        std::ostringstream window_scale_ss{};
                        window_scale_ss << composition_window.scaling[0];

                        replace(config_dump, window_position_key_ss.str(), window_position_ss.str());
                        replace(config_dump, window_scale_key_ss.str(), window_scale_ss.str());

                        if (gui.windows.contains(composition_window.window_id)) {
                            auto& window = gui.windows.at(composition_window.window_id);

                            if (std::holds_alternative<ConfigDumpGUICuboidWindow>(window)) {
                                auto& cuboid_window = std::get<ConfigDumpGUICuboidWindow>(window);

                                auto gen_color_str = [](std::ostringstream& dst, const glm::vec4& vec) {
                                    std::size_t r = static_cast<std::size_t>(vec.r * 255);
                                    std::size_t g = static_cast<std::size_t>(vec.g * 255);
                                    std::size_t b = static_cast<std::size_t>(vec.b * 255);
                                    std::size_t a = static_cast<std::size_t>(vec.a * 255);
                                    dst << "[ " << r << ", " << g << ", " << b << ", " << a << " ]";
                                };

                                std::vector<Material*> materials;
                                for (std::size_t i = 0; i < cuboid_window.entities.size(); i++) {
                                    auto& material = database_context.fetch_component_unchecked<Material>(
                                        cuboid_window.entities[i]);
                                    materials.push_back(&material);

                                    std::ostringstream window_color_fill_active_key_ss{};
                                    window_color_fill_active_key_ss << R"(")" << composition_window.window_id
                                                                    << "_colors_fill_active_" << i << R"(")";

                                    std::ostringstream window_color_fill_inactive_key_ss{};
                                    window_color_fill_inactive_key_ss << R"(")" << composition_window.window_id
                                                                      << "_colors_fill_inactive_" << i << R"(")";

                                    std::ostringstream window_color_border_active_key_ss{};
                                    window_color_border_active_key_ss << R"(")" << composition_window.window_id
                                                                      << "_colors_border_active_" << i << R"(")";

                                    std::ostringstream window_color_border_inactive_key_ss{};
                                    window_color_border_inactive_key_ss << R"(")" << composition_window.window_id
                                                                        << "_colors_border_inactive_" << i << R"(")";

                                    std::ostringstream window_color_oob_active_key_ss{};
                                    window_color_oob_active_key_ss << R"(")" << composition_window.window_id
                                                                   << "_colors_oob_active_" << i << R"(")";

                                    std::ostringstream window_color_oob_inactive_key_ss{};
                                    window_color_oob_inactive_key_ss << R"(")" << composition_window.window_id
                                                                     << "_colors_oob_inactive_" << i << R"(")";

                                    std::ostringstream fill_active_ss{};
                                    std::ostringstream fill_inactive_ss{};
                                    std::ostringstream border_active_ss{};
                                    std::ostringstream border_inactive_ss{};
                                    std::ostringstream oob_active_ss{};
                                    std::ostringstream oob_inactive_ss{};

                                    auto& fill_active_attribute
                                        = *material.m_passes[1].m_material_variables.getPtr<glm::vec4>(
                                            "active_fill_color", 1);
                                    auto& fill_inactive_attribute
                                        = *material.m_passes[1].m_material_variables.getPtr<glm::vec4>(
                                            "inactive_fill_color", 1);
                                    auto& border_active_attribute
                                        = *material.m_passes[0].m_material_variables.getPtr<glm::vec4>(
                                            "active_border_color", 1);
                                    auto& border_inactive_attribute
                                        = *material.m_passes[0].m_material_variables.getPtr<glm::vec4>(
                                            "inactive_border_color", 1);
                                    auto& oob_active_attribute
                                        = *material.m_passes[1].m_material_variables.getPtr<glm::vec4>(
                                            "oob_active_color", 1);
                                    auto& oob_inactive_attribute
                                        = *material.m_passes[1].m_material_variables.getPtr<glm::vec4>(
                                            "oob_inactive_color", 1);

                                    gen_color_str(fill_active_ss, fill_active_attribute);
                                    gen_color_str(fill_inactive_ss, fill_inactive_attribute);
                                    gen_color_str(border_active_ss, border_active_attribute);
                                    gen_color_str(border_inactive_ss, border_inactive_attribute);
                                    gen_color_str(oob_active_ss, oob_active_attribute);
                                    gen_color_str(oob_inactive_ss, oob_inactive_attribute);

                                    replace(config_dump, window_color_fill_active_key_ss.str(), fill_active_ss.str());
                                    replace(
                                        config_dump, window_color_fill_inactive_key_ss.str(), fill_inactive_ss.str());
                                    replace(
                                        config_dump, window_color_border_active_key_ss.str(), border_active_ss.str());
                                    replace(config_dump, window_color_border_inactive_key_ss.str(),
                                        border_inactive_ss.str());
                                    replace(config_dump, window_color_oob_active_key_ss.str(), oob_active_ss.str());
                                    replace(config_dump, window_color_oob_inactive_key_ss.str(), oob_inactive_ss.str());
                                }

                                if (cuboid_window.heatmap) {
                                    std::ostringstream heatmap_cuboid_key_ss{};
                                    std::ostringstream heatmap_colors_key_ss{};
                                    std::ostringstream heatmap_colors_start_key_ss{};

                                    heatmap_cuboid_key_ss << R"(")" << composition_window.window_id << "_heatmap_cuboid"
                                                          << R"(")";

                                    heatmap_colors_key_ss << R"(")" << composition_window.window_id << "_heatmap_colors"
                                                          << R"(")";

                                    heatmap_colors_start_key_ss << R"(")" << composition_window.window_id
                                                                << "_heatmap_colors_start"
                                                                << R"(")";

                                    std::ostringstream heatmap_cuboid_ss{};
                                    std::ostringstream heatmap_colors_ss{};
                                    std::ostringstream heatmap_colors_start_ss{};

                                    heatmap_cuboid_ss << cuboid_window.heatmap_idx;

                                    auto& heatmap_color_count_attribute
                                        = *materials.back()->m_passes[1].m_material_variables.getPtr<unsigned int>(
                                            "heatmap_color_count", 1);
                                    auto heatmap_colors_attribute
                                        = materials.back()->m_passes[1].m_material_variables.getPtr<glm::vec4>(
                                            "heatmap_fill_colors", 10);
                                    auto heatmap_colors_start_attribute
                                        = materials.back()->m_passes[1].m_material_variables.getPtr<float>(
                                            "heatmap_color_start", 10);

                                    heatmap_colors_ss << "[ ";
                                    heatmap_colors_start_ss << "[ ";

                                    for (unsigned int color = 0; color < heatmap_color_count_attribute; color++) {
                                        gen_color_str(heatmap_colors_ss, heatmap_colors_attribute[color]);
                                        heatmap_colors_start_ss << heatmap_colors_start_attribute[color];

                                        if (color != heatmap_color_count_attribute - 1) {
                                            heatmap_colors_ss << ", ";
                                            heatmap_colors_start_ss << ", ";
                                        }
                                    }

                                    heatmap_colors_ss << " ]";
                                    heatmap_colors_start_ss << " ]";

                                    replace(config_dump, heatmap_cuboid_key_ss.str(), heatmap_cuboid_ss.str());
                                    replace(config_dump, heatmap_colors_key_ss.str(), heatmap_colors_ss.str());
                                    replace(
                                        config_dump, heatmap_colors_start_key_ss.str(), heatmap_colors_start_ss.str());
                                }
                            }
                        }
                    }
                }
            }

            std::ofstream output{ "config_dump.json" };
            output.write(config_dump.c_str(), config_dump.size());
        }

        ImGui::End();

        if (!gui.active) {
            freeze(false);
        }
    }
}

}