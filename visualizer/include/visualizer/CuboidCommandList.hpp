#pragma once

#include <glm/glm.hpp>
#include <variant>
#include <vector>

namespace Visualizer {

enum class CuboidCommandType { NOOP, DRAW, DRAW_MULTIPLE, DELETE, DELETE_MULTIPLE };

struct NoopCommand {
    std::size_t counter;
};

struct DrawCommand {
    glm::vec3 cuboid_size;
    glm::vec3 start_position;
    glm::vec4 fill_color;
    glm::vec4 border_color;
};

struct DrawMultipleCommand {
    glm::vec4 fill_color;
    glm::vec4 border_color;
    std::vector<glm::vec3> cuboid_sizes;
    std::vector<glm::vec3> start_positions;
};

struct DeleteCommand {
    glm::vec4 fill_color;
    glm::vec4 border_color;
};

struct DeleteMultipleCommand {
    std::size_t counter;
    glm::vec4 fill_color;
    glm::vec4 border_color;
};

struct CuboidCommand {
    CuboidCommandType type;
    std::variant<NoopCommand, DrawCommand, DrawMultipleCommand, DeleteCommand, DeleteMultipleCommand> command;
};

struct CuboidCommandList {
    std::size_t current_index;
    std::size_t command_counter;
    std::vector<CuboidCommand> commands;
};

}
