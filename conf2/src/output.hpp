#pragma once

#include <filesystem>
#include <string>

#include <visconfig/Config.hpp>

#include "processing.hpp"

namespace Config {

struct GenerationOptions {
    std::size_t screen_width;
    std::size_t screen_height;
    std::size_t screen_msaa_samples;
    bool screen_fullscreen;

    std::size_t render_resolution_multiplier;

    float cuboid_border_width;

    float camera_fov;
    float camera_aspect;
    float camera_near;
    float camera_far;

    float min_transparency;
    float max_transparency;

    std::filesystem::path working_directory;
    std::filesystem::path resource_directory;

    std::filesystem::path assets_directory_path;
    std::filesystem::path assets_texture_directory_path;

    std::filesystem::path cuboid_diffuse_shader_vertex_path;
    std::filesystem::path cuboid_diffuse_shader_fragment_path;

    std::filesystem::path cuboid_oit_shader_vertex_path;
    std::filesystem::path cuboid_oit_shader_fragment_path;

    std::filesystem::path cuboid_oit_blend_shader_vertex_path;
    std::filesystem::path cuboid_oit_blend_shader_fragment_path;

    std::filesystem::path view_composition_shader_vertex_path;
    std::filesystem::path view_composition_shader_fragment_path;

    std::string cuboid_pipeline_name;
    std::string cuboid_mesh_asset_name;
    std::string cuboid_oit_shader_asset_name;
    std::string cuboid_diffuse_shader_asset_name;
    std::string default_framebuffer_asset_name;
    std::string fullscreen_quad_mesh_asset_name;
    std::string cuboid_oit_blend_shader_asset_name;
    std::string view_composition_shader_asset_name;
};

Visconfig::Config generate_config(
    const ConfigCommandList& config_command_list, const GenerationOptions& generation_options);

}