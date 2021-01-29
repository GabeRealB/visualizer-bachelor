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
    std::size_t cuboid_idx;
};

struct DrawMultipleCommand {
    std::vector<std::size_t> cuboid_indices;
};

struct DeleteCommand {
};

struct DeleteMultipleCommand {
    std::size_t counter;
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
