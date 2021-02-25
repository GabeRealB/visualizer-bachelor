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

struct Canvas {
    std::vector<std::variant<LegendGUI>> guis;
};

}
