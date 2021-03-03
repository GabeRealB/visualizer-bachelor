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

struct BoundingBox {
    CuboidPosition start;
    CuboidPosition end;
};

struct NoopCommand {
    std::size_t counter;
};

struct DrawCommand {
    bool out_of_bounds;
    std::size_t cuboid_idx;
    BoundingBox bounding_box;
};

struct DrawMultipleCommand {
    std::vector<std::size_t> out_of_bounds;
    std::vector<BoundingBox> bounding_boxes;
    std::unordered_set<std::size_t> cuboid_indices;
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

struct CuboidInfo {
    CuboidSize size;
    CuboidPosition position;
};

struct CuboidCommandList {
    CuboidColor active_fill_color;
    CuboidColor out_of_bounds_fill_color;
    CuboidColor inactive_fill_color;
    CuboidColor active_border_color;
    CuboidColor inactive_border_color;
    std::vector<CuboidCommand> commands;
    std::vector<CuboidInfo> positions;
    std::map<std::tuple<CuboidPosition, CuboidSize>, std::size_t> position_index_map;
};

struct ViewCommandList {
    float size;
    std::string view_name;
    std::array<float, 2> position;
    std::vector<CuboidCommandList> cuboids;
};

struct ConfigCommandList {
    std::vector<ViewCommandList> view_commands;
};

bool aabb_contains(const BoundingBox& lhs, const BoundingBox& rhs);
bool aabb_intersects(const BoundingBox& lhs, const BoundingBox& rhs);
BoundingBox aabb_extend(const BoundingBox& lhs, const BoundingBox& rhs);

ConfigCommandList generate_config_command_list();
void print_config_command_list(const ConfigCommandList& config);

}