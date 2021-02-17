#pragma once

#include <array>
#include <map>
#include <string>
#include <tuple>
#include <unordered_set>
#include <variant>
#include <vector>

namespace Config {

using CuboidSize = std::array<int, 3>;
using CuboidPosition = std::array<int, 3>;
using CuboidColor = std::array<std::size_t, 4>;

enum class CuboidCommandType { NOOP, DRAW, DRAW_MULTIPLE, DELETE, DELETE_MULTIPLE };

struct NoopCommand {
    std::size_t counter;
};

struct DrawCommand {
    std::size_t cuboid_idx;
    std::array<int, 3> cuboid_size;
    std::array<int, 3> start_position;
    std::array<std::size_t, 4> fill_color;
    std::array<std::size_t, 4> border_color;
};

struct DrawMultipleCommand {
    std::array<std::size_t, 4> fill_color;
    std::array<std::size_t, 4> border_color;
    std::unordered_set<std::size_t> cuboid_indices;
    std::vector<std::array<int, 3>> cuboid_sizes;
    std::vector<std::array<int, 3>> start_positions;
};

struct DeleteCommand {
    std::array<std::size_t, 4> fill_color;
    std::array<std::size_t, 4> border_color;
};

struct DeleteMultipleCommand {
    std::size_t counter;
    std::array<std::size_t, 4> fill_color;
    std::array<std::size_t, 4> border_color;
};

struct CuboidCommand {
    CuboidCommandType type;
    std::variant<NoopCommand, DrawCommand, DrawMultipleCommand, DeleteCommand, DeleteMultipleCommand> command;
};

struct CuboidCommandList {
    CuboidColor active_fill_color;
    CuboidColor inactive_fill_color;
    CuboidColor active_border_color;
    CuboidColor inactive_border_color;
    std::vector<CuboidCommand> commands;
    std::vector<std::tuple<CuboidPosition, CuboidSize>> positions;
    std::map<std::tuple<CuboidPosition, CuboidSize>, std::size_t> position_index_map;
};

struct ViewCommandList {
    float size;
    bool movable;
    std::string view_name;
    std::array<float, 2> position;
    std::vector<CuboidCommandList> cuboids;
};

struct ConfigCommandList {
    std::vector<ViewCommandList> view_commands;
};

ConfigCommandList generate_config_command_list();
void print_config_command_list(const ConfigCommandList& config);

}