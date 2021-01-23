#pragma once

#include <array>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace Config {

enum class CuboidCommandType { NOOP, DRAW, DRAW_MULTIPLE, DELETE, DELETE_MULTIPLE };

struct NoopCommand {
    std::size_t counter;
};

struct DrawCommand {
    std::array<int, 3> cuboid_size;
    std::array<int, 3> start_position;
    std::array<std::size_t, 4> fill_color;
    std::array<std::size_t, 4> border_color;
};

struct DrawMultipleCommand {
    std::array<std::size_t, 4> fill_color;
    std::array<std::size_t, 4> border_color;
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
    std::vector<CuboidCommand> commands;
};

struct ViewCommandList {
    std::string view_name;
    std::vector<CuboidCommandList> cuboids;
};

struct ConfigCommandList {
    std::vector<ViewCommandList> view_commands;
};

ConfigCommandList generate_config_command_list();
void print_config_command_list(const ConfigCommandList& config);

}