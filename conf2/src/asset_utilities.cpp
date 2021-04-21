#include "asset_utilities.hpp"

#include <cmath>
#include <numbers>

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

    /*
    constexpr std::array<float, 3> normals[]{
        { -1.0f / std::numbers::sqrt3, -1.0f / std::numbers::sqrt3, 1.0f / std::numbers::sqrt3 }, // lower-left-front
        { 1.0f / std::numbers::sqrt3, -1.0f / std::numbers::sqrt3, 1.0f / std::numbers::sqrt3 }, // lower-right-front
        { 1.0f / std::numbers::sqrt3, 1.0f / std::numbers::sqrt3, 1.0f / std::numbers::sqrt3 }, // top-right-front
        { -1.0f / std::numbers::sqrt3, 1.0f / std::numbers::sqrt3, 1.0f / std::numbers::sqrt3 }, // top-left-front

        { -1.0f / std::numbers::sqrt3, -1.0f / std::numbers::sqrt3, -1.0f / std::numbers::sqrt3 }, // lower-left-back
        { 1.0f / std::numbers::sqrt3, -1.0f / std::numbers::sqrt3, -1.0f / std::numbers::sqrt3 }, // lower-right-back
        { 1.0f / std::numbers::sqrt3, 1.0f / std::numbers::sqrt3, -1.0f / std::numbers::sqrt3 }, // top-right-back
        { -1.0f / std::numbers::sqrt3, 1.0f / std::numbers::sqrt3, -1.0f / std::numbers::sqrt3 }, // top-left-back
    };

    constexpr std::array<float, 3> flattened_normals[]{
        // front
        normals[0],
        normals[1],
        normals[2],
        normals[0],
        normals[2],
        normals[3],

        // top
        normals[3],
        normals[2],
        normals[6],
        normals[3],
        normals[6],
        normals[7],

        // right
        normals[1],
        normals[5],
        normals[6],
        normals[1],
        normals[6],
        normals[2],

        // left
        normals[4],
        normals[0],
        normals[3],
        normals[4],
        normals[3],
        normals[7],

        // bottom
        normals[4],
        normals[5],
        normals[1],
        normals[4],
        normals[1],
        normals[0],

        // back
        normals[5],
        normals[4],
        normals[7],
        normals[5],
        normals[7],
        normals[6],
    };
    */
    constexpr std::array<float, 3> normals[]{
        { 0.0f, 0.0f, 1.0f }, // front
        { 0.0f, 1.0f, 0.0f }, // top
        { 1.0f, 0.0f, 0.0f }, // right
        { -1.0f, 0.0f, 0.0f }, // left
        { 0.0f, -1.0f, 0.0f }, // bottom
        { 0.0f, 0.0f, -1.0f }, // back
    };

    constexpr std::array<float, 3> flattened_normals[]{
        // front
        normals[0],
        normals[0],
        normals[0],
        normals[0],
        normals[0],
        normals[0],

        // top
        normals[1],
        normals[1],
        normals[1],
        normals[1],
        normals[1],
        normals[1],

        // right
        normals[2],
        normals[2],
        normals[2],
        normals[2],
        normals[2],
        normals[2],

        // left
        normals[3],
        normals[3],
        normals[3],
        normals[3],
        normals[3],
        normals[3],

        // bottom
        normals[4],
        normals[4],
        normals[4],
        normals[4],
        normals[4],
        normals[4],

        // back
        normals[5],
        normals[5],
        normals[5],
        normals[5],
        normals[5],
        normals[5],
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

    constexpr std::size_t normals_buffer_size = sizeof(flattened_normals);

    std::vector<std::byte> normals_buffer(normals_buffer_size, std::byte{});
    std::memcpy(normals_buffer.data(), flattened_normals, normals_buffer_size);

    mesh_data->vertices
        = std::vector<std::array<float, 4>>{ std::begin(flattened_vertices), std::end(flattened_vertices) };
    mesh_data->indices = std::vector<std::uint32_t>{ std::begin(indices), std::end(indices) };
    mesh_data->texture_coords0
        = std::vector<std::array<float, 4>>{ std::begin(flattened_tex_coords), std::end(flattened_tex_coords) };
    mesh_data->simple_attributes.insert({ "normals",
        {
            .normalized = false,
            .size = normals_buffer_size,
            .index = 2,
            .stride = 0,
            .offset = 0,
            .usage = Visconfig::Assets::MeshAttributeUsage::StaticDraw,
            .data = std::move(normals_buffer),
            .element_size = Visconfig::Assets::MeshAttributeElementSize::Three,
            .element_type = Visconfig::Assets::MeshAttributeElementType::Float,
        } });

    return asset;
}
Visconfig::Asset create_fullscreen_quad_mesh_asset(const std::string& asset_name)
{
    auto mesh_data{ std::make_shared<Visconfig::Assets::MeshAsset>() };
    Visconfig::Asset asset{ asset_name, Visconfig::Assets::AssetType::Mesh, mesh_data };

    constexpr std::array<float, 2> vertices[]{
        { -1.0f, -1.0f }, // lower-left
        { 1.0f, -1.0f }, // lower-right
        { 1.0f, 1.0f }, // upper-right
        { -1.0f, 1.0f }, // upper-left
    };

    constexpr std::array<float, 2> uvs[]{
        { 0.0f, 0.0f }, // lower-left
        { 1.0f, 0.0f }, // lower-right
        { 1.0f, 1.0f }, // upper-right
        { 0.0f, 1.0f }, // upper-left
    };

    constexpr uint32_t indices[]{ 0, 1, 2, 0, 2, 3 };

    std::vector<std::byte> vertex_buffer(sizeof(vertices), std::byte{});
    std::memcpy(vertex_buffer.data(), vertices, sizeof(vertices));

    std::vector<std::byte> uv_buffer(sizeof(uvs), std::byte{});
    std::memcpy(uv_buffer.data(), uvs, sizeof(uvs));

    mesh_data->indices = std::vector<std::uint32_t>{ std::begin(indices), std::end(indices) };
    mesh_data->simple_attributes.insert({ "vertices",
        {
            .normalized = false,
            .size = sizeof(vertices),
            .index = 0,
            .stride = 0,
            .offset = 0,
            .usage = Visconfig::Assets::MeshAttributeUsage::StaticDraw,
            .data = std::move(vertex_buffer),
            .element_size = Visconfig::Assets::MeshAttributeElementSize::Two,
            .element_type = Visconfig::Assets::MeshAttributeElementType::Float,
        } });
    mesh_data->simple_attributes.insert({ "uv_0",
        {
            .normalized = false,
            .size = sizeof(uvs),
            .index = 1,
            .stride = 0,
            .offset = 0,
            .usage = Visconfig::Assets::MeshAttributeUsage::StaticDraw,
            .data = std::move(uv_buffer),
            .element_size = Visconfig::Assets::MeshAttributeElementSize::Two,
            .element_type = Visconfig::Assets::MeshAttributeElementType::Float,
        } });

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
    Visconfig::Assets::TextureDataType data_type, const std::vector<Visconfig::Assets::TextureAttributes>& attributes)
{
    auto texture_data{ std::make_shared<Visconfig::Assets::TextureFileAsset>() };
    Visconfig::Asset asset{ texture_name, Visconfig::Assets::AssetType::TextureFile, texture_data };
    texture_data->data_type = data_type;
    texture_data->path = texture_path;
    texture_data->attributes = attributes;

    return asset;
}

Visconfig::Asset create_buffer_texture_asset(const std::string& texture_name, std::size_t size,
    Visconfig::Assets::TextureFormat format, Visconfig::Assets::MeshAttributeUsage data_usage,
    std::vector<std::byte> fill_data)
{
    auto texture_asset{ std::make_shared<Visconfig::Assets::TextureBufferRawAsset>() };
    Visconfig::Asset asset{ texture_name, Visconfig::Assets::AssetType::TextureBufferRaw, texture_asset };
    texture_asset->size = size;
    texture_asset->format = format;
    texture_asset->data_usage = data_usage;
    texture_asset->fill_data = std::move(fill_data);

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

Visconfig::Asset create_cuboid_render_pipeline_asset(const std::string& asset_name)
{
    auto asset_data{ std::make_shared<Visconfig::Assets::CuboidRenderPipelineAsset>() };
    Visconfig::Asset asset{ asset_name, Visconfig::Assets::AssetType::CuboidRenderPipeline, asset_data };
    return asset;
}

}
