#pragma once

#include <cstdio>
#include <string>

#include <visconfig/Config.hpp>

#include "processing.hpp"

namespace Config {

Visconfig::Entity generate_coordinator_entity(std::size_t entity_id);
Visconfig::Entity generate_view_camera(std::size_t entity_id, std::size_t focus_entity_id, std::size_t view_idx,
    float fov, float aspect, float near, float far, float distance, float orthographic_width, float orthographic_height,
    const std::map<std::string, std::vector<std::string>>& targets, bool fixed, bool perspective, bool active);
Visconfig::Entity generate_cuboid(std::size_t entity_id, std::size_t view_idx, bool global,
    const CuboidCommandList& command_list, const std::string& front_texture, const std::string& side_texture,
    const std::string& top_texture, const std::string& accum_texture, const std::string& revealage_texture,
    const std::string& mesh_name, const std::string& pipeline_name, const std::vector<std::string>& shader_asset_names,
    std::array<int, 3> global_size);

void extend_camera_switcher(Visconfig::Entity& coordinator_entity, std::size_t camera_entity_id);
void extend_composition(Visconfig::Entity& coordinator_entity, std::array<float, 2> scale,
    std::array<float, 2> position, const std::vector<std::string>& src, const std::string& target,
    const std::string& shader, std::size_t id, bool draggable);

void add_color_legend(Visconfig::Entity& coordinator_entity, const std::string& label, const std::string& description,
    const std::string& attribute, std::size_t entity, std::size_t pass);
void add_image_legend(Visconfig::Entity& coordinator_entity, const std::string& image, const std::string& description,
    const std::array<float, 2>& scaling, bool absolute);
}
