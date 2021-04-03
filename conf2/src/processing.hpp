#pragma once

#include <array>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
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
    std::unordered_map<std::size_t, std::size_t> cuboid_accesses;
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

struct HeatmapData {
    std::vector<float> colors_start;
    std::vector<CuboidColor> colors;
};

struct CameraData {
    bool fixed;
    bool active;
    bool perspective;
    float fov;
    float aspect;
    float near;
    float far;
    float distance;
    float orthographic_width;
    float orthographic_height;
    float horizontal_angle;
    float vertical_angle;
    std::array<float, 3> position;
    std::array<float, 3> rotation;
};

struct CuboidCommandList {
    float line_width;
    std::size_t max_accesses;
    CuboidColor active_fill_color;
    CuboidColor inactive_fill_color;
    CuboidColor active_border_color;
    CuboidColor inactive_border_color;
    CuboidColor oob_active_color;
    CuboidColor oob_inactive_color;
    std::vector<CuboidCommand> commands;
    std::vector<CuboidInfo> positions;
    std::vector<std::size_t> access_counters;
    std::optional<HeatmapData> heatmap;
    std::map<std::tuple<CuboidPosition, CuboidSize>, std::size_t> position_index_map;
};

struct ViewCommandList {
    float size;
    float border_width;
    bool heatmap;
    bool invert_x;
    bool invert_y;
    bool invert_z;
    std::string id;
    std::string view_name;
    std::size_t heatmap_idx;
    std::array<float, 2> position;
    std::optional<CameraData> camera;
    std::vector<CuboidCommandList> cuboids;
    std::array<std::size_t, 4> border_color;
    std::array<std::size_t, 4> caption_color;
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