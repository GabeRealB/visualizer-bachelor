#include "asset_utilities.hpp"

#include <cmath>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace Config {

Visconfig::Asset create_cuboid_mesh_asset(const std::string& asset_name)
{
    auto mesh_data{ std::make_shared<Visconfig::Assets::MeshAsset>() };
    Visconfig::Asset asset{ asset_name, Visconfig::Assets::AssetType::Mesh, mesh_data };

    constexpr std::array<float, 4> vertices[]{
        { -0.5f, -0.5f, 0.5f, 1.0f }, // lower-left-front
        { 0.5f, -0.5f, 0.5f, 1.0f }, // lower-right-front
        { 0.5f, 0.5f, 0.5f, 1.0f }, // top-right-front
        { -0.5f, 0.5f, 0.5f, 1.0f }, // top-left-front

        { -0.5f, -0.5f, -0.5f, 1.0f }, // lower-left-back
        { 0.5f, -0.5f, -0.5f, 1.0f }, // lower-right-back
        { 0.5f, 0.5f, -0.5f, 1.0f }, // top-right-back
        { -0.5f, 0.5f, -0.5f, 1.0f }, // top-left-back
    };

    constexpr std::array<float, 4> flattened_vertices[]{
        // front
        vertices[0],
        vertices[1],
        vertices[2],
        vertices[0],
        vertices[2],
        vertices[3],

        // top
        vertices[3],
        vertices[2],
        vertices[6],
        vertices[3],
        vertices[6],
        vertices[7],

        // right
        vertices[1],
        vertices[5],
        vertices[6],
        vertices[1],
        vertices[6],
        vertices[2],

        // left
        vertices[4],
        vertices[0],
        vertices[3],
        vertices[4],
        vertices[3],
        vertices[7],

        // bottom
        vertices[4],
        vertices[5],
        vertices[1],
        vertices[4],
        vertices[1],
        vertices[0],

        // back
        vertices[5],
        vertices[4],
        vertices[7],
        vertices[5],
        vertices[7],
        vertices[6],
    };

    constexpr std::array<float, 3> tex_coords[]{
        { 0.0f, 0.0f, 0.0f }, // lower-left
        { 1.0f, 0.0f, 0.0f }, // lower-right
        { 1.0f, 1.0f, 0.0f }, // top-right
        { 0.0f, 1.0f, 0.0f }, // top-left
    };

    constexpr std::array<float, 4> flattened_tex_coords[]{
        // front
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 0.0f },
        { tex_coords[1][0], tex_coords[1][1], tex_coords[1][2], 0.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 0.0f },
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 0.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 0.0f },
        { tex_coords[3][0], tex_coords[3][1], tex_coords[3][2], 0.0f },

        // top
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 1.0f },
        { tex_coords[1][0], tex_coords[1][1], tex_coords[1][2], 1.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 1.0f },
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 1.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 1.0f },
        { tex_coords[3][0], tex_coords[3][1], tex_coords[3][2], 1.0f },

        // right
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 2.0f },
        { tex_coords[1][0], tex_coords[1][1], tex_coords[1][2], 2.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 2.0f },
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 2.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 2.0f },
        { tex_coords[3][0], tex_coords[3][1], tex_coords[3][2], 2.0f },

        // left
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 2.0f },
        { tex_coords[1][0], tex_coords[1][1], tex_coords[1][2], 2.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 2.0f },
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 2.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 2.0f },
        { tex_coords[3][0], tex_coords[3][1], tex_coords[3][2], 2.0f },

        // bottom
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 1.0f },
        { tex_coords[1][0], tex_coords[1][1], tex_coords[1][2], 1.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 1.0f },
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 1.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 1.0f },
        { tex_coords[3][0], tex_coords[3][1], tex_coords[3][2], 1.0f },

        // back
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 0.0f },
        { tex_coords[1][0], tex_coords[1][1], tex_coords[1][2], 0.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 0.0f },
        { tex_coords[0][0], tex_coords[0][1], tex_coords[0][2], 0.0f },
        { tex_coords[2][0], tex_coords[2][1], tex_coords[2][2], 0.0f },
        { tex_coords[3][0], tex_coords[3][1], tex_coords[3][2], 0.0f },
    };

    constexpr uint32_t indices[]{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };

    mesh_data->vertices
        = std::vector<std::array<float, 4>>{ std::begin(flattened_vertices), std::end(flattened_vertices) };
    mesh_data->indices = std::vector<std::uint32_t>{ std::begin(indices), std::end(indices) };
    mesh_data->texture_coords0
        = std::vector<std::array<float, 4>>{ std::begin(flattened_tex_coords), std::end(flattened_tex_coords) };

    return asset;
}

Visconfig::Asset create_default_framebuffer_asset(const std::string& asset_name)
{
    return Visconfig::Asset{ asset_name, Visconfig::Assets::AssetType::DefaultFramebuffer,
        std::make_shared<Visconfig::Assets::DefaultFramebufferAsset>() };
}

Visconfig::Asset create_shader_asset(const std::string& asset_name, const std::filesystem::path& vertex_shader,
    const std::filesystem::path& fragment_shader)
{
    auto shader_data{ std::make_shared<Visconfig::Assets::ShaderAsset>() };
    Visconfig::Asset asset{ asset_name, Visconfig::Assets::AssetType::Shader, shader_data };
    shader_data->vertex = vertex_shader;
    shader_data->fragment = fragment_shader;

    return asset;
}

Visconfig::Asset create_texture_asset(const std::string& texture_name, const std::filesystem::path& texture_path,
    const std::vector<Visconfig::Assets::TextureAttributes>& attributes)
{
    auto texture_data{ std::make_shared<Visconfig::Assets::TextureFileAsset>() };
    Visconfig::Asset asset{ texture_name, Visconfig::Assets::AssetType::TextureFile, texture_data };
    texture_data->path = texture_path;
    texture_data->attributes = attributes;

    return asset;
}

Visconfig::Asset create_render_texture_asset(
    const std::string& texture_name, std::size_t width, std::size_t height, Visconfig::Assets::TextureFormat format)
{
    auto texture_data{ std::make_shared<Visconfig::Assets::TextureRawAsset>() };
    Visconfig::Asset asset{ texture_name, Visconfig::Assets::AssetType::TextureRaw, texture_data };
    texture_data->width = width;
    texture_data->height = height;
    texture_data->format = format;

    return asset;
}

Visconfig::Asset create_multisample_render_texture_asset(const std::string& texture_name, std::size_t width,
    std::size_t height, std::size_t samples, Visconfig::Assets::TextureFormat format)
{
    auto texture_data{ std::make_shared<Visconfig::Assets::TextureMultisampleRawAsset>() };
    Visconfig::Asset asset{ texture_name, Visconfig::Assets::AssetType::TextureMultisampleRaw, texture_data };
    texture_data->width = width;
    texture_data->height = height;
    texture_data->samples = samples;
    texture_data->format = format;

    return asset;
}

Visconfig::Asset create_renderbuffer_asset(const std::string& renderbuffer_name, std::size_t width, std::size_t height,
    std::size_t samples, Visconfig::Assets::RenderbufferFormat format)
{
    auto renderbuffer_asset{ std::make_shared<Visconfig::Assets::RenderbufferAsset>() };
    Visconfig::Asset asset{ renderbuffer_name, Visconfig::Assets::AssetType::Renderbuffer, renderbuffer_asset };
    renderbuffer_asset->width = width;
    renderbuffer_asset->height = height;
    renderbuffer_asset->samples = samples;
    renderbuffer_asset->format = format;

    return asset;
}

Visconfig::Asset create_framebuffer_asset(const std::string& renderbuffer_name, std::size_t start_x,
    std::size_t start_y, std::size_t width, std::size_t height,
    const std::vector<std::tuple<Visconfig::Assets::FramebufferType, Visconfig::Assets::FramebufferDestination,
        std::string>>& attachments)
{
    auto buffer_data{ std::make_shared<Visconfig::Assets::FramebufferAsset>() };
    Visconfig::Asset asset{ renderbuffer_name, Visconfig::Assets::AssetType::Framebuffer, buffer_data };

    for (auto& attachment : attachments) {
        buffer_data->attachments.push_back(Visconfig::Assets::FramebufferAttachment{
            std::get<0>(attachment), std::get<1>(attachment), std::get<2>(attachment) });
    }
    buffer_data->viewportWidth = width;
    buffer_data->viewportHeight = height;
    buffer_data->viewportStartX = start_x;
    buffer_data->viewportStartY = start_y;

    return asset;
}

void generate_texture_file(const std::filesystem::path& destination, std::size_t width, std::size_t height,
    std::size_t subdivisions_x, std::size_t subdivisions_y, std::size_t line_width)
{
    std::vector<std::byte> texture_data{};
    constexpr std::size_t texture_scaling{ 8 };
    constexpr std::size_t min_texture_quality_multiplier{ 1 };
    constexpr std::size_t max_texture_quality_multiplier{ 4 };

    auto dimension_mean{ static_cast<float>(std::pow(width * height, 0.5f)) };
    auto line_ratio = static_cast<float>(line_width) / dimension_mean * texture_scaling;
    auto texture_quality_multiplier = [&]() -> auto
    {
        if (line_ratio <= min_texture_quality_multiplier) {
            return min_texture_quality_multiplier;
        } else if (line_ratio >= max_texture_quality_multiplier) {
            return max_texture_quality_multiplier;
        } else {
            return static_cast<std::size_t>(std::lerp(0.0f, 1.0f, line_ratio));
        }
    }
    ();

    std::size_t scaled_width{ width * texture_scaling * texture_quality_multiplier };
    std::size_t scaled_height{ height * texture_scaling * texture_quality_multiplier };
    std::size_t scaled_line_width{ line_width * texture_quality_multiplier };

    std::size_t section_width{ scaled_width / subdivisions_x };
    std::size_t section_height{ scaled_height / subdivisions_y };
    std::size_t row_stride{ scaled_width * 3 };

    std::byte section_color{ std::numeric_limits<unsigned char>::max() };
    std::byte border_color{ std::numeric_limits<std::byte>::min() };

    texture_data.reserve(scaled_width * scaled_height);

    for (std::size_t i{ 0 }; i < subdivisions_y; i++) {
        for (std::size_t j{ 0 }; j < section_height; j++) {
            if (j >= scaled_line_width && j < section_height - scaled_line_width) {
                for (std::size_t k{ 0 }; k < subdivisions_x; k++) {
                    for (std::size_t l{ 0 }; l < section_width; l++) {
                        if (l >= scaled_line_width && l < section_width - scaled_line_width) {
                            texture_data.push_back(section_color);
                            texture_data.push_back(section_color);
                            texture_data.push_back(section_color);
                        } else {
                            texture_data.push_back(border_color);
                            texture_data.push_back(border_color);
                            texture_data.push_back(border_color);
                        }
                    }
                }
            } else {
                for (std::size_t k{ 0 }; k < scaled_width; k++) {
                    texture_data.push_back(border_color);
                    texture_data.push_back(border_color);
                    texture_data.push_back(border_color);
                }
            }
        }
    }

    auto texture_width{ static_cast<int>(scaled_width) };
    auto texture_height{ static_cast<int>(scaled_height) };
    auto texture_row_stride{ static_cast<int>(row_stride) };

    if (texture_width == 0 || static_cast<std::size_t>(texture_width) < scaled_width || texture_height == 0
        || static_cast<std::size_t>(texture_height) < scaled_height || texture_row_stride == 0
        || static_cast<std::size_t>(texture_row_stride) < row_stride) {
        return;
    }

    stbi_write_png(
        destination.string().c_str(), texture_width, texture_height, 3, texture_data.data(), texture_row_stride);
}

}
