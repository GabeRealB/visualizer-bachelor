#pragma once

#include <glm/glm.hpp>
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
    Entity entity;
    std::size_t pass;
    std::string label;
    std::string description;
    std::string attribute;
};

struct LegendGUI {
    glm::vec2 size;
    glm::vec2 position;
    std::vector<std::variant<LegendGUIColor, LegendGUIImage>> entries;
};

struct CompositionGUIWindow {
    glm::vec2 scaling;
    glm::vec2 position;
    std::string window_name;
    std::weak_ptr<const Texture2D> texture;
};

struct CompositionGUIGroup {
    glm::vec2 position;
    std::string group_name;
    std::vector<CompositionGUIWindow> windows;
};

struct CompositionGUI {
    glm::vec2 size;
    glm::vec2 position;
    std::size_t selected_group;
    std::size_t selected_window;
    std::vector<CompositionGUIGroup> groups;
    std::vector<std::array<std::size_t, 2>> group_connections;
};

struct Canvas {
    std::vector<std::variant<LegendGUI, CompositionGUI>> guis;
};

}
