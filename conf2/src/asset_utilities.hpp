#pragma once

#include <cstdio>
#include <filesystem>
#include <string>

#include <visconfig/Config.hpp>

namespace Config {

Visconfig::Asset create_cuboid_mesh_asset(const std::string& asset_name);
Visconfig::Asset create_fullscreen_quad_mesh_asset(const std::string& asset_name);
Visconfig::Asset create_default_framebuffer_asset(const std::string& asset_name);
Visconfig::Asset create_shader_asset(const std::string& asset_name, const std::filesystem::path& vertex_shader,
    const std::filesystem::path& fragment_shader);
Visconfig::Asset create_texture_asset(const std::string& texture_name, const std::filesystem::path& texture_path,
    Visconfig::Assets::TextureDataType data_type, const std::vector<Visconfig::Assets::TextureAttributes>& attributes);
Visconfig::Asset create_render_texture_asset(
    const std::string& texture_name, std::size_t width, std::size_t height, Visconfig::Assets::TextureFormat format);
Visconfig::Asset create_multisample_render_texture_asset(const std::string& texture_name, std::size_t width,
    std::size_t height, std::size_t samples, Visconfig::Assets::TextureFormat format);
Visconfig::Asset create_renderbuffer_asset(const std::string& renderbuffer_name, std::size_t width, std::size_t height,
    std::size_t samples, Visconfig::Assets::RenderbufferFormat format);
Visconfig::Asset create_framebuffer_asset(const std::string& renderbuffer_name, std::size_t start_x,
    std::size_t start_y, std::size_t width, std::size_t height,
    const std::vector<std::tuple<Visconfig::Assets::FramebufferType, Visconfig::Assets::FramebufferDestination,
        std::string>>& attachments);
Visconfig::Asset create_cuboid_render_pipeline_asset(const std::string& asset_name, std::size_t samples,
    std::size_t transparency_layers, std::size_t width, std::size_t height);
}
