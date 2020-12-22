#pragma once

/// Standard includes
#include <array>
#include <string>
#include <vector>

/// External libraries
#include <visconfig/Config.hpp>

/// Internal includes
#include "processing.hpp"

namespace MDH2Vis {

Visconfig::Entity generate_coordinator_entity(std::size_t entity_id);

Visconfig::Entity generate_cube(std ::array<float, 3> position, std::array<float, 3> scale, std::array<float, 4> color,
    std::size_t entity_id, std::size_t parent_id, std::size_t layer_id, bool has_parent,
    const std::string& front_texture, const std::string& side_texture, const std::string& top_texture,
    const std::string& mesh_name, const std::string& shader_asset_name);
Visconfig::Entity generate_main_view_cube(const ProcessedConfig& mdh_config, const detail::MainViewInfo& view,
    std::size_t entity_id, std::size_t parent_id, std::size_t layer_number, const std::string& front_texture,
    const std::string& side_texture, const std::string& top_texture, const std::string& mesh_name,
    const std::string& shader_asset_name, float min_transparency, float max_transparency);
Visconfig::Entity generate_main_view_thread_cube(const ProcessedConfig& mdh_config, const detail::MainViewInfo& view,
    std::size_t entity_id, std::size_t parent_id, std::size_t layer_number, std::array<std::size_t, 3> block_number,
    const std::string& front_texture, const std::string& side_texture, const std::string& top_texture,
    const std::string& mesh_name, const std::string& shader_asset_name, float min_transparency, float max_transparency);
Visconfig::Entity generate_output_view_cube(Visconfig::World& world, const ProcessedConfig& mdh_config,
    const detail::OutputViewInfo& view, std::size_t& entity_id, std::size_t parent_id, std::size_t view_number,
    std::size_t layer_number, const std::string& cube_texture, const std::string& mesh_name,
    const std::string& shader_asset_name, float min_transparency, float max_transparency);
Visconfig::Entity generate_sub_view_cube(const ProcessedConfig& mdh_config, const detail::SubViewInfo& view,
    std::size_t entity_id, std::size_t parent_id, std::size_t view_number, std::size_t layer_number,
    const std::string& front_texture, const std::string& side_texture, const std::string& top_texture,
    const std::string& mesh_name, const std::string& shader_asset_name, float min_transparency, float max_transparency);

Visconfig::Entity generate_view_camera(std::size_t entity_id, std::size_t focus, std::size_t view_index,
    const std::string& framebuffer_name, float fov, float aspect, float near, float far, float distance,
    float orthographic_width, float orthographic_height, bool active, bool fixed, bool perspective);

void extend_camera_switcher(Visconfig::World& world, std::size_t camera);
void extend_copy(Visconfig::World& world, const std::string& source, const std::string& destination,
    const std::vector<Visconfig::Components::CopyOperationFlag>& flags,
    Visconfig::Components::CopyOperationFilter filter);
void extend_composition(Visconfig::World& world, std::array<float, 2> scale, std::array<float, 2> position,
    std::vector<std::string> src, const std::string& target, const std::string& shader, std::size_t id, bool draggable);

}