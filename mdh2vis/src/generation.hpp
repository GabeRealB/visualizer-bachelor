#pragma once

/// Standard includes
#include <filesystem>
#include <string>

/// External libraries
#include <visconfig/Config.hpp>

/// Internal includes
#include "processing.hpp"

namespace MDH2Vis {

struct GenerationOptions {
    std::size_t screen_width;
    std::size_t screen_height;
    std::size_t screen_msaa_samples;
    bool screen_fullscreen;

    std::size_t render_resolution_multiplier;

    float main_view_texture_border_relative_width;
    float thread_view_texture_border_relative_width;
    float sub_view_texture_border_relative_width;

    float camera_fov;
    float camera_aspect;
    float camera_aspect_small;
    float camera_near;
    float camera_far;

    float min_transparency;
    float max_transparency;

    std::filesystem::path working_dir;

    std::filesystem::path assets_directory_path;
    std::filesystem::path assets_texture_directory_path;

    std::filesystem::path cube_shader_vertex_path;
    std::filesystem::path cube_shader_fragment_path;

    std::filesystem::path view_composition_shader_vertex_path;
    std::filesystem::path view_composition_shader_fragment_path;

    std::string cube_mesh_asset_name;
    std::string cube_texture_asset_name;
    std::string output_cube_texture_asset_name;
    std::string cube_shader_asset_name;
    std::string default_framebuffer_asset_name;
    std::string view_composition_shader_asset_name;
};

void generate_assets_directory(const GenerationOptions& generation_options);
Visconfig::Config generate_config(const MDH2Vis::ProcessedConfig& config, const GenerationOptions& generation_options);

}