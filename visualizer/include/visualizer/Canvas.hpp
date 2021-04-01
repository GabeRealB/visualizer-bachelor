#pragma once

#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <visualizer/Entity.hpp>
#include <visualizer/Texture.hpp>

namespace Visualizer {

struct LegendGUIImage {
    bool absolute;
    glm::vec2 scaling;
    std::string description;
    std::weak_ptr<const Texture> texture;
};

struct LegendGUIColor {
    glm::vec4 color;
    std::string label;
    std::string caption;
    glm::vec4 caption_color;
};

struct LegendGUI {
    glm::vec2 size;
    glm::vec2 position;
    std::vector<std::variant<LegendGUIColor, LegendGUIImage>> entries;
};

struct CompositionGUIWindow {
    glm::vec2 scaling;
    glm::vec2 position;
    bool flip_vertically;
    std::string window_id;
    glm::vec4 caption_color;
    std::string window_name;
    std::weak_ptr<const Texture2D> texture;
};

struct CompositionGUIGroup {
    bool transparent;
    glm::vec2 position;
    std::string group_id;
    std::string group_name;
    glm::vec4 caption_color;
    std::vector<CompositionGUIWindow> windows;
};

enum class GroupConnectionPoint {
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
};

struct GroupConnection {
    glm::vec4 color;
    float head_size;
    float line_width;
    std::size_t start_idx;
    GroupConnectionPoint start_point;
    std::size_t end_idx;
    GroupConnectionPoint end_point;
};

struct CompositionGUI {
    glm::vec2 size;
    glm::vec2 position;
    glm::vec4 background_color;
    std::size_t selected_group;
    std::size_t selected_window;
    std::vector<CompositionGUIGroup> groups;
    std::vector<GroupConnection> group_connections;
};

struct ConfigDumpGUITextureWindow {
};

struct ConfigDumpGUICuboidWindow {
    bool heatmap;
    std::size_t heatmap_idx;
    std::vector<Entity> entities;
};

using ConfigDumpGUIWindow = std::variant<ConfigDumpGUITextureWindow, ConfigDumpGUICuboidWindow>;

struct ConfigDumpGUI {
    bool active;
    int n_key_state;
    std::string config_template;
    std::map<std::string, ConfigDumpGUIWindow> windows;
};

struct Canvas {
    std::vector<std::variant<LegendGUI, CompositionGUI, ConfigDumpGUI>> guis;
};

}
