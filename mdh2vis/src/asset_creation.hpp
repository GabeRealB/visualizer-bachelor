#pragma once

/// Standard includes
#include <filesystem>
#include <string>

/// External libraries
#include <visconfig/Config.hpp>

namespace MDH2Vis {

Visconfig::Asset create_cube_mesh_asset(const std::string& asset_name);
Visconfig::Asset create_cube_texture_asset(const std::string& asset_name);
Visconfig::Asset create_output_cube_texture_asset(const std::string& asset_name);
Visconfig::Asset create_default_framebuffer_asset(const std::string& asset_name);
Visconfig::Asset create_simple_cube_mesh_asset(const std::string& asset_name);
Visconfig::Asset create_shader_asset(const std::filesystem::path& vertex_path,
    const std::filesystem::path& fragment_path, const std::string& asset_name);
Visconfig::Asset generate_render_texture_asset(
    const std::string& asset_name, std::size_t width, std::size_t height, Visconfig::Assets::TextureFormat format);
Visconfig::Asset generate_renderbuffer_asset(const std::string& asset_name, std::size_t width, std::size_t height,
    std::size_t samples, Visconfig::Assets::RenderbufferFormat format);
Visconfig::Asset generate_framebuffer_asset(const std::string& asset_name, std::size_t start_x, std::size_t start_y,
    std::size_t width, std::size_t height,
    const std::vector<std::tuple<Visconfig::Assets::FramebufferType, Visconfig::Assets::FramebufferDestination,
        std::string>>& attachments);
Visconfig::Asset generate_multisample_render_texture_asset(const std::string& asset_name, std::size_t width,
    std::size_t height, std::size_t samples, Visconfig::Assets::TextureFormat format);
Visconfig::Asset create_texture_asset(const std::string& asset_name, const std::string& path,
    const std::vector<Visconfig::Assets::TextureAttributes>& attributes);

void generate_texture_file(const std::string& name, std::size_t width, std::size_t height, std::size_t subdivisions_x,
    std::size_t subdivisions_y, std::size_t line_width);

}
