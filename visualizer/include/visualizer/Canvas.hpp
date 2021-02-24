#pragma once

#include <glm/glm.hpp>
#include <string>
#include <variant>
#include <vector>

#include <visualizer/Entity.hpp>

namespace Visualizer {

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
    std::vector<std::variant<LegendGUIColor>> entries;
};

struct Canvas {
    std::vector<std::variant<LegendGUI>> guis;
};

}
