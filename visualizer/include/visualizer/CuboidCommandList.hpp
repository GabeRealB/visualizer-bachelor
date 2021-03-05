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
    bool out_of_bounds;
    std::size_t cuboid_idx;
};

struct DrawMultipleCommand {
    std::vector<std::size_t> out_of_bounds;
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
    bool heat_map;
    std::size_t current_index;
    std::size_t command_counter;
    std::size_t access_stepping;
    std::vector<CuboidCommand> commands;
    std::vector<std::size_t> cuboid_accesses;
};

}
