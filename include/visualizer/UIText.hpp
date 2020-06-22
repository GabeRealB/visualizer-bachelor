#pragma once

#include <string>

#include <glm/glm.hpp>

namespace Visualizer {

struct UIText {
    glm::vec4 color;
    std::string message;
};

}