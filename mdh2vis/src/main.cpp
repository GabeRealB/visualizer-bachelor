#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <thread>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <visconfig/Config.hpp>

#include "MDHConfig.hpp"
#include "MDHOps.hpp"
#include "processing.hpp"

constexpr std::string_view usage_str{ "Usage: mdh2vis --model model-path --tps tps-path [--out output-dir]" };

constexpr std::size_t screen_width = 1200;
constexpr std::size_t screen_height = 900;
constexpr std::size_t screen_msaa_samples = 16;
constexpr bool screen_fullscreen = false;

constexpr std::size_t render_resolution_multiplier{ 2 };
constexpr std::size_t render_resolution_width{ screen_width * render_resolution_multiplier };
constexpr std::size_t render_resolution_height{ screen_height * render_resolution_multiplier };

constexpr float main_view_texture_border_relative_width{ 0.02f };
constexpr float thread_view_texture_border_relative_width{ 0.05f };
constexpr float sub_view_texture_border_relative_width{ 0.4f };

constexpr float camera_fov{ 70.0f };
constexpr float camera_aspect{ static_cast<float>(screen_width) / static_cast<float>(screen_height) };
constexpr float camera_aspect_small{ camera_aspect / 2 };
constexpr float camera_near{ 0.3f };
constexpr float camera_far{ 10000.0f };

constexpr float min_transparency{ 0.1f };
constexpr float max_transparency{ 0.95f };

constexpr auto assets_directory{ "external_assets" };
constexpr auto assets_texture_directory{ "external_assets/textures" };

constexpr auto cube_mesh_asset{ "cube_mesh" };
constexpr auto cube_texture_asset{ "cube_texture" };
constexpr auto output_cube_texture_asset{ "output_cube_texture" };
constexpr auto cube_shader_asset{ "cube_shader" };
constexpr auto default_framebuffer_asset{ "default_framebuffer" };
constexpr auto view_composition_shader_asset{ "view_composition_shader" };

constexpr auto cube_shader_vertex_path{ "assets/shaders/cube.vs.glsl" };
constexpr auto cube_shader_fragment_path{ "assets/shaders/cube.fs.glsl" };

constexpr auto view_composition_shader_vertex_path{ "assets/shaders/compositing.vs.glsl" };
constexpr auto view_composition_shader_fragment_path{ "assets/shaders/compositing.fs.glsl" };

void print_config_info(const MDH2Vis::ProcessedConfig& config);
void generate_assets_directory(const std::filesystem::path& workingDir);
Visconfig::Config generate_config(const MDH2Vis::ProcessedConfig& config, const std::filesystem::path& working_dir);

int main(int argc, char* argv[])
{
    if (argc != 5 && argc != 7) {
        std::cerr << usage_str << std::endl;
        return 1;
    }

    auto working_dir{ std::filesystem::current_path() };

    std::filesystem::path model_path{};
    std::filesystem::path tps_path{};

    bool model_set{ false };
    bool tps_set{ false };

    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], "--model") == 0) {
            model_path = argv[i + 1];
            model_set = true;
            ++i;
        } else if (std::strcmp(argv[i], "--tps") == 0) {
            tps_path = argv[i + 1];
            tps_set = true;
            ++i;
        } else if (std::strcmp(argv[i], "--out") == 0) {
            working_dir = argv[i + 1];
            ++i;
        } else {
            std::cerr << usage_str << std::endl;
            return 1;
        }
    }

    if (!model_set || !tps_set) {
        std::cerr << usage_str << std::endl;
        return 1;
    }

    if (!std::filesystem::exists(working_dir)) {
        std::cerr << "Could not find " << working_dir << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(model_path)) {
        std::cerr << "Could not find " << model_path << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(tps_path)) {
        std::cerr << "Could not find " << tps_path << std::endl;
        return 1;
    }

    std::cout << "Paths:" << std::endl;
    std::cout << "\tOutput:" << working_dir << std::endl;
    std::cout << "\tModel:" << model_path << std::endl;
    std::cout << "\tTPS:" << tps_path << std::endl;

    auto mdh_config{ MDH2Vis::loadFromFiles(model_path, tps_path) };
    if (!mdh_config) {
        std::cerr << "Could not load configs" << std::endl;
        return 1;
    }

    auto config{ MDH2Vis::process_config(*mdh_config) };
    print_config_info(config);

    generate_assets_directory(working_dir);
    auto vis_config{ generate_config(config, working_dir) };
    Visconfig::to_file(working_dir / "visconfig.json", vis_config);
}

void print_config_info(const MDH2Vis::ProcessedConfig& config)
{
    std::cout << "Config info:" << std::endl;

    std::cout << "Number of layers: " << config.config.size() << std::endl;
    std::cout << "Dimensions: " << config.config[0].tps.tileSize[0] << ", " << config.config[0].tps.tileSize[1] << ", "
              << config.config[0].tps.tileSize[2] << std::endl;
    std::cout << std::endl;
}

template <typename T> float interpolateLinear(float startValue, float endValue, T min, T max, T current)
{
    if (current <= min) {
        return startValue;
    } else if (current >= max) {
        return endValue;
    } else {
        auto length{ max - min };
        auto minDistance{ current - min };
        auto normalizedMinDistance{ static_cast<float>(minDistance) / static_cast<float>(length) };

        auto startInfluence{ (1 - normalizedMinDistance) * startValue };
        auto endInfluence{ normalizedMinDistance * endValue };
        return startInfluence + endInfluence;
    }
}

void generate_assets_directory(const std::filesystem::path& workingDir)
{
    auto assetPath{ workingDir / assets_directory };
    auto assetTexturePath{ workingDir / assets_texture_directory };

    if (std::filesystem::exists(assetPath)) {
        std::filesystem::remove_all(assetPath);
    }

    std::filesystem::create_directories(assetPath);
    std::filesystem::create_directory(assetTexturePath);
}

void generateTextureFile(const std::string& name, std::size_t width, std::size_t height, std::size_t subdivisionsX,
    std::size_t subdivisionsY, std::size_t lineWidth)
{
    std::vector<std::byte> textureData{};
    constexpr std::size_t textureScaling{ 8 };
    constexpr std::size_t minTextureQualityMultiplier{ 1 };
    constexpr std::size_t maxTextureQualityMultiplier{ 4 };

    auto dimensionMean{ static_cast<float>(std::pow(width * height, 0.5f)) };
    auto lineRatio = static_cast<float>(lineWidth) / dimensionMean * textureScaling;
    auto textureQualityMultiplier{ static_cast<std::size_t>(
        interpolateLinear(minTextureQualityMultiplier, maxTextureQualityMultiplier, 0.0f, 1.0f, lineRatio)) };

    std::size_t scaledWidth{ width * textureScaling * textureQualityMultiplier };
    std::size_t scaledHeight{ height * textureScaling * textureQualityMultiplier };
    std::size_t scaledLineWidth{ lineWidth * textureQualityMultiplier };

    std::size_t sectionWidth{ scaledWidth / subdivisionsX };
    std::size_t sectionHeight{ scaledHeight / subdivisionsY };
    std::size_t rowStride{ scaledWidth * 3 };

    std::byte sectionColor{ std::numeric_limits<unsigned char>::max() };
    std::byte borderColor{ std::numeric_limits<std::byte>::min() };

    textureData.reserve(scaledWidth * scaledHeight);

    for (std::size_t i{ 0 }; i < subdivisionsY; i++) {
        for (std::size_t j{ 0 }; j < sectionHeight; j++) {
            if (j >= scaledLineWidth && j < sectionHeight - scaledLineWidth) {
                for (std::size_t k{ 0 }; k < subdivisionsX; k++) {
                    for (std::size_t l{ 0 }; l < sectionWidth; l++) {
                        if (l >= scaledLineWidth && l < sectionWidth - scaledLineWidth) {
                            textureData.push_back(sectionColor);
                            textureData.push_back(sectionColor);
                            textureData.push_back(sectionColor);
                        } else {
                            textureData.push_back(borderColor);
                            textureData.push_back(borderColor);
                            textureData.push_back(borderColor);
                        }
                    }
                }
            } else {
                for (std::size_t k{ 0 }; k < scaledWidth; k++) {
                    textureData.push_back(borderColor);
                    textureData.push_back(borderColor);
                    textureData.push_back(borderColor);
                }
            }
        }
    }

    auto textureWidth{ static_cast<int>(scaledWidth) };
    auto textureHeight{ static_cast<int>(scaledHeight) };
    auto textureRowStride{ static_cast<int>(rowStride) };

    if (textureWidth == 0 || static_cast<std::size_t>(textureWidth) < scaledWidth || textureHeight == 0
        || static_cast<std::size_t>(textureHeight) < scaledHeight || textureRowStride == 0
        || static_cast<std::size_t>(textureRowStride) < rowStride) {
        return;
    }

    stbi_write_png(name.c_str(), textureWidth, textureHeight, 3, textureData.data(), textureRowStride);
}

Visconfig::Asset createSimpleCubeMeshAsset(const std::string& name)
{
    auto meshData{ std::make_shared<Visconfig::Assets::MeshAsset>() };
    Visconfig::Asset asset{ name, Visconfig::Assets::AssetType::Mesh, meshData };

    constexpr std::array<std::array<float, 4>, 8> vertices{
        std::array<float, 4>{ 0.0f, -1.0f, 1.0f, 1.0f }, // lower-left-front
        std::array<float, 4>{ 1.0f, -1.0f, 1.0f, 1.0f }, // lower-right-front
        std::array<float, 4>{ 1.0f, 0.0f, 1.0f, 1.0f }, // top-right-front
        std::array<float, 4>{ 0.0f, 0.0f, 1.0f, 1.0f }, // top-left-front

        std::array<float, 4>{ 0.0f, -1.0f, 0.0f, 1.0f }, // lower-left-back
        std::array<float, 4>{ 1.0f, -1.0f, 0.0f, 1.0f }, // lower-right-back
        std::array<float, 4>{ 1.0f, 0.0f, 0.0f, 1.0f }, // top-right-back
        std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f }, // top-left-back
    };

    constexpr std::array<std::array<float, 4>, 8> texCoords{
        std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }, // lower-left-front
        std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }, // lower-right-front
        std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }, // top-right-front
        std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }, // top-left-front

        std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }, // lower-left-back
        std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }, // lower-right-back
        std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }, // top-right-back
        std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }, // top-left-back
    };

    constexpr std::array<std::uint32_t, 36> indices{
        0, 1, 2, 0, 2, 3, // front
        5, 4, 7, 5, 7, 6, // back
        4, 0, 3, 4, 3, 7, // left
        1, 5, 6, 1, 6, 2, // right
        3, 2, 6, 3, 6, 7, // top
        4, 5, 1, 4, 1, 0, // bottom
    };

    meshData->indices.resize(indices.size());
    meshData->vertices.resize(vertices.size());
    meshData->texture_coords0.resize(texCoords.size());

    std::copy(indices.begin(), indices.end(), meshData->indices.begin());
    std::copy(vertices.begin(), vertices.end(), meshData->vertices.begin());
    std::copy(texCoords.begin(), texCoords.end(), meshData->texture_coords0.begin());

    return asset;
}

Visconfig::Asset createCubeMeshAsset()
{
    auto meshData{ std::make_shared<Visconfig::Assets::MeshAsset>() };
    Visconfig::Asset asset{ cube_mesh_asset, Visconfig::Assets::AssetType::Mesh, meshData };

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

    constexpr std::array<float, 4> flattenedVertices[]{
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

    constexpr std::array<float, 3> texCoords[]{
        { 0.0f, 0.0f, 0.0f }, // lower-left
        { 1.0f, 0.0f, 0.0f }, // lower-right
        { 1.0f, 1.0f, 0.0f }, // top-right
        { 0.0f, 1.0f, 0.0f }, // top-left
    };

    constexpr std::array<float, 4> flattenedTexCoords[]{
        // front
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 0.0f },
        { texCoords[1][0], texCoords[1][1], texCoords[1][2], 0.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 0.0f },
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 0.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 0.0f },
        { texCoords[3][0], texCoords[3][1], texCoords[3][2], 0.0f },

        // top
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 1.0f },
        { texCoords[1][0], texCoords[1][1], texCoords[1][2], 1.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 1.0f },
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 1.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 1.0f },
        { texCoords[3][0], texCoords[3][1], texCoords[3][2], 1.0f },

        // right
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 2.0f },
        { texCoords[1][0], texCoords[1][1], texCoords[1][2], 2.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 2.0f },
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 2.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 2.0f },
        { texCoords[3][0], texCoords[3][1], texCoords[3][2], 2.0f },

        // left
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 2.0f },
        { texCoords[1][0], texCoords[1][1], texCoords[1][2], 2.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 2.0f },
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 2.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 2.0f },
        { texCoords[3][0], texCoords[3][1], texCoords[3][2], 2.0f },

        // bottom
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 1.0f },
        { texCoords[1][0], texCoords[1][1], texCoords[1][2], 1.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 1.0f },
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 1.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 1.0f },
        { texCoords[3][0], texCoords[3][1], texCoords[3][2], 1.0f },

        // back
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 0.0f },
        { texCoords[1][0], texCoords[1][1], texCoords[1][2], 0.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 0.0f },
        { texCoords[0][0], texCoords[0][1], texCoords[0][2], 0.0f },
        { texCoords[2][0], texCoords[2][1], texCoords[2][2], 0.0f },
        { texCoords[3][0], texCoords[3][1], texCoords[3][2], 0.0f },
    };

    constexpr uint32_t indices[]{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };

    meshData->vertices
        = std::vector<std::array<float, 4>>{ std::begin(flattenedVertices), std::end(flattenedVertices) };
    meshData->indices = std::vector<std::uint32_t>{ std::begin(indices), std::end(indices) };
    meshData->texture_coords0
        = std::vector<std::array<float, 4>>{ std::begin(flattenedTexCoords), std::end(flattenedTexCoords) };

    return asset;
}

Visconfig::Asset createTextureAsset(const std::string& name, const std::string& path,
    const std::vector<Visconfig::Assets::TextureAttributes>& attributes)
{
    auto textureData{ std::make_shared<Visconfig::Assets::TextureFileAsset>() };
    Visconfig::Asset asset{ name, Visconfig::Assets::AssetType::TextureFile, textureData };
    textureData->path = path;
    textureData->attributes = attributes;

    return asset;
}

Visconfig::Asset createCubeTextureAsset()
{
    return createTextureAsset(cube_texture_asset, "assets/textures/cube.png",
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps });
}

Visconfig::Asset createOutputCubeTextureAsset()
{
    return createTextureAsset(output_cube_texture_asset, "assets/textures/output_cube.png",
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps });
}

Visconfig::Asset createShaderAsset(
    const std::string& vertexPath, const std::string& fragmentPath, const std::string& name)
{
    auto shaderData{ std::make_shared<Visconfig::Assets::ShaderAsset>() };
    Visconfig::Asset asset{ name, Visconfig::Assets::AssetType::Shader, shaderData };
    shaderData->vertex = vertexPath;
    shaderData->fragment = fragmentPath;

    return asset;
}

Visconfig::Asset createDefaultFramebufferAsset()
{
    return Visconfig::Asset{ default_framebuffer_asset, Visconfig::Assets::AssetType::DefaultFramebuffer,
        std::make_shared<Visconfig::Assets::DefaultFramebufferAsset>() };
}

Visconfig::Asset generateRenderTextureAsset(
    const std::string& name, std::size_t width, std::size_t height, Visconfig::Assets::TextureFormat format)
{
    auto textureData{ std::make_shared<Visconfig::Assets::TextureRawAsset>() };
    Visconfig::Asset asset{ name, Visconfig::Assets::AssetType::TextureRaw, textureData };
    textureData->width = width;
    textureData->height = height;
    textureData->format = format;

    return asset;
}

Visconfig::Asset generateMultisampleRenderTextureAsset(const std::string& name, std::size_t width, std::size_t height,
    std::size_t samples, Visconfig::Assets::TextureFormat format)
{
    auto textureData{ std::make_shared<Visconfig::Assets::TextureMultisampleRawAsset>() };
    Visconfig::Asset asset{ name, Visconfig::Assets::AssetType::TextureMultisampleRaw, textureData };
    textureData->width = width;
    textureData->height = height;
    textureData->samples = samples;
    textureData->format = format;

    return asset;
}

Visconfig::Asset generateRenderbufferAsset(const std::string& name, std::size_t width, std::size_t height,
    std::size_t samples, Visconfig::Assets::RenderbufferFormat format)
{
    auto renderbufferAsset{ std::make_shared<Visconfig::Assets::RenderbufferAsset>() };
    Visconfig::Asset asset{ name, Visconfig::Assets::AssetType::Renderbuffer, renderbufferAsset };
    renderbufferAsset->width = width;
    renderbufferAsset->height = height;
    renderbufferAsset->samples = samples;
    renderbufferAsset->format = format;

    return asset;
}

Visconfig::Asset generateFramebufferAsset(const std::string& name, std::size_t startX, std::size_t startY,
    std::size_t width, std::size_t height,
    const std::vector<std::tuple<Visconfig::Assets::FramebufferType, Visconfig::Assets::FramebufferDestination,
        std::string>>& attachments)
{
    auto bufferData{ std::make_shared<Visconfig::Assets::FramebufferAsset>() };
    Visconfig::Asset asset{ name, Visconfig::Assets::AssetType::Framebuffer, bufferData };

    for (auto& attachment : attachments) {
        bufferData->attachments.push_back(Visconfig::Assets::FramebufferAttachment{
            std::get<0>(attachment), std::get<1>(attachment), std::get<2>(attachment) });
    }
    bufferData->viewportWidth = width;
    bufferData->viewportHeight = height;
    bufferData->viewportStartX = startX;
    bufferData->viewportStartY = startY;

    return asset;
}

Visconfig::Entity generateMainViewCube(const MDH2Vis::ProcessedConfig& mdhConfig,
    const MDH2Vis::detail::MainViewInfo& view, std::size_t entityId, std::size_t parentId, std::size_t layerNumber,
    const std::string& frontTexture, const std::string& sideTexture, const std::string& topTexture)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back(
        { Visconfig::Components::ComponentType::Cube, std::make_shared<Visconfig::Components::CubeComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Mesh, std::make_shared<Visconfig::Components::MeshComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Material,
        std::make_shared<Visconfig::Components::MaterialComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Layer, std::make_shared<Visconfig::Components::LayerComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });

    if (layerNumber > 0) {
        entity.components.push_back({ Visconfig::Components::ComponentType::Parent,
            std::make_shared<Visconfig::Components::ParentComponent>() });
        entity.components.push_back({ Visconfig::Components::ComponentType::ImplicitIteration,
            std::make_shared<Visconfig::Components::ImplicitIterationComponent>() });
    }

    [[maybe_unused]] constexpr auto cubeIndex{ 0 };
    constexpr auto meshIndex{ 1 };
    constexpr auto materialIndex{ 2 };
    constexpr auto layerIndex{ 3 };
    constexpr auto transformIndex{ 4 };
    constexpr auto parentIndex{ 5 };
    constexpr auto iterationIndex{ 6 };

    auto& mesh{ *std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[meshIndex].data) };
    auto& material{ *std::static_pointer_cast<Visconfig::Components::MaterialComponent>(
        entity.components[materialIndex].data) };
    auto& layer{ *std::static_pointer_cast<Visconfig::Components::LayerComponent>(entity.components[layerIndex].data) };
    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transformIndex].data) };

    mesh.asset = cube_mesh_asset;

    auto textureFront{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto textureSide{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto textureTop{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuseColor{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    textureFront->asset = frontTexture;
    textureFront->slot = 0;

    textureSide->asset = sideTexture;
    textureSide->slot = 1;

    textureTop->asset = topTexture;
    textureTop->slot = 2;

    diffuseColor->value[0] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[0]) / 255.0f;
    diffuseColor->value[1] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[1]) / 255.0f;
    diffuseColor->value[2] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[2]) / 255.0f;
    diffuseColor->value[3] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[3]) / 255.0f;
    diffuseColor->value[3] = interpolateLinear<std::size_t>(
        min_transparency, max_transparency, 0ull, view.layers.size() + view.threads.size() - 1, layerNumber);

    material.asset = cube_shader_asset;
    material.attributes.insert_or_assign("gridTextureFront",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureFront, false });
    material.attributes.insert_or_assign("gridTextureSide",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureSide, false });
    material.attributes.insert_or_assign("gridTextureTop",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureTop, false });
    material.attributes.insert_or_assign("diffuseColor",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuseColor, false });

    layer.mask = 1llu;

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = view.layers[layerNumber].scale[0];
    transform.scale[1] = view.layers[layerNumber].scale[1];
    transform.scale[2] = view.layers[layerNumber].scale[2];

    if (layerNumber != 0) {
        std::array<float, 3> halfScale{
            view.layers[layerNumber].scale[0] / 2.0f,
            view.layers[layerNumber].scale[1] / 2.0f,
            view.layers[layerNumber].scale[2] / 2.0f,
        };

        transform.position[0] = -0.5f + halfScale[0];
        transform.position[1] = 0.5f - halfScale[1];
        transform.position[2] = -0.5f + halfScale[2];
    } else {
        transform.position[0] = 0;
        transform.position[1] = 0;
        transform.position[2] = 0;
    }

    if (layerNumber != 0) {
        auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
            entity.components[parentIndex].data) };
        auto& iteration{ *std::static_pointer_cast<Visconfig::Components::ImplicitIterationComponent>(
            entity.components[iterationIndex].data) };

        parent.id = parentId;

        iteration.order = Visconfig::Components::IterationOrder::XYZ;
        iteration.numIterations[0] = view.layers[layerNumber].num_iterations[0];
        iteration.numIterations[1] = view.layers[layerNumber].num_iterations[1];
        iteration.numIterations[2] = view.layers[layerNumber].num_iterations[2];

        iteration.ticksPerIteration = view.layers[layerNumber].absolute_scale[0]
            * view.layers[layerNumber].absolute_scale[1] * view.layers[layerNumber].absolute_scale[2];
        iteration.ticksPerIteration
            /= view.threads[0].absolute_scale[0] * view.threads[0].absolute_scale[1] * view.threads[0].absolute_scale[2];
    }

    return entity;
}

Visconfig::Entity generateMainViewThreadCube(const MDH2Vis::ProcessedConfig& mdhConfig,
    const MDH2Vis::detail::MainViewInfo& view, std::size_t entityId, std::size_t parentId, std::size_t layerNumber,
    std::array<std::size_t, 3> blockNumber, const std::string& frontTexture, const std::string& sideTexture,
    const std::string& topTexture)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back(
        { Visconfig::Components::ComponentType::Cube, std::make_shared<Visconfig::Components::CubeComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Mesh, std::make_shared<Visconfig::Components::MeshComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Material,
        std::make_shared<Visconfig::Components::MaterialComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Layer, std::make_shared<Visconfig::Components::LayerComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Parent, std::make_shared<Visconfig::Components::ParentComponent>() });

    if (layerNumber == 0) {
        entity.components.push_back({ Visconfig::Components::ComponentType::ImplicitIteration,
            std::make_shared<Visconfig::Components::ImplicitIterationComponent>() });
    }

    [[maybe_unused]] constexpr auto cubeIndex{ 0 };
    constexpr auto meshIndex{ 1 };
    constexpr auto materialIndex{ 2 };
    constexpr auto layerIndex{ 3 };
    constexpr auto transformIndex{ 4 };
    constexpr auto parentIndex{ 5 };
    constexpr auto iterationIndex{ 6 };

    auto& mesh{ *std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[meshIndex].data) };
    auto& material{ *std::static_pointer_cast<Visconfig::Components::MaterialComponent>(
        entity.components[materialIndex].data) };
    auto& layer{ *std::static_pointer_cast<Visconfig::Components::LayerComponent>(entity.components[layerIndex].data) };
    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transformIndex].data) };
    auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
        entity.components[parentIndex].data) };

    mesh.asset = cube_mesh_asset;

    auto textureFront{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto textureSide{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto textureTop{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuseColor{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    textureFront->asset = frontTexture;
    textureFront->slot = 0;

    textureSide->asset = sideTexture;
    textureSide->slot = 1;

    textureTop->asset = topTexture;
    textureTop->slot = 2;

    diffuseColor->value[0] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.thread[0]) / 255.0f;
    diffuseColor->value[1] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.thread[1]) / 255.0f;
    diffuseColor->value[2] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.thread[2]) / 255.0f;
    diffuseColor->value[3] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.thread[3]) / 255.0f;
    diffuseColor->value[3] = interpolateLinear<std::size_t>(min_transparency, max_transparency, 0ull,
        view.layers.size() + view.threads.size() - 1, view.layers.size() + layerNumber);

    material.asset = cube_shader_asset;
    material.attributes.insert_or_assign("gridTextureFront",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureFront, false });
    material.attributes.insert_or_assign("gridTextureSide",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureSide, false });
    material.attributes.insert_or_assign("gridTextureTop",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureTop, false });
    material.attributes.insert_or_assign("diffuseColor",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuseColor, false });

    layer.mask = 1llu;

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = view.threads[layerNumber].scale[0];
    transform.scale[1] = view.threads[layerNumber].scale[1];
    transform.scale[2] = view.threads[layerNumber].scale[2];

    std::array<float, 3> halfScale{
        view.threads[layerNumber].scale[0] / 2.0f,
        view.threads[layerNumber].scale[1] / 2.0f,
        view.threads[layerNumber].scale[2] / 2.0f,
    };

    transform.position[0] = -0.5f + halfScale[0] + (blockNumber[0] * transform.scale[0]);
    transform.position[1] = 0.5f - halfScale[1] - (blockNumber[1] * transform.scale[1]);
    transform.position[2] = -0.5f + halfScale[2] + (blockNumber[2] * transform.scale[2]);

    parent.id = parentId;

    if (layerNumber == 0) {
        auto& iteration{ *std::static_pointer_cast<Visconfig::Components::ImplicitIterationComponent>(
            entity.components[iterationIndex].data) };

        iteration.order = Visconfig::Components::IterationOrder::XYZ;
        iteration.numIterations[0] = view.threads[layerNumber].num_iterations[0];
        iteration.numIterations[1] = view.threads[layerNumber].num_iterations[1];
        iteration.numIterations[2] = view.threads[layerNumber].num_iterations[2];

        /*
        iteration.ticksPerIteration = view.threads[layerNumber].absolute_scale[0]
            * view.threads[layerNumber].absolute_scale[1] * view.threads[layerNumber].absolute_scale[2];
        */
        iteration.ticksPerIteration = 1;
    }

    return entity;
}

Visconfig::Entity generateCube(std ::array<float, 3> position, std::array<float, 3> scale, std::array<float, 4> color,
    std::size_t entityId, std::size_t parentId, std::size_t layerId, bool hasParent, const std::string& frontTexture,
    const std::string& sideTexture, const std::string& topTexture, const std::string& meshName)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back(
        { Visconfig::Components::ComponentType::Cube, std::make_shared<Visconfig::Components::CubeComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Mesh, std::make_shared<Visconfig::Components::MeshComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Material,
        std::make_shared<Visconfig::Components::MaterialComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Layer, std::make_shared<Visconfig::Components::LayerComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });

    if (hasParent) {
        entity.components.push_back({ Visconfig::Components::ComponentType::Parent,
            std::make_shared<Visconfig::Components::ParentComponent>() });
    }

    [[maybe_unused]] constexpr auto cubeIndex{ 0 };
    constexpr auto meshIndex{ 1 };
    constexpr auto materialIndex{ 2 };
    constexpr auto layerIndex{ 3 };
    constexpr auto transformIndex{ 4 };
    constexpr auto parentIndex{ 5 };

    auto& mesh{ *std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[meshIndex].data) };
    auto& material{ *std::static_pointer_cast<Visconfig::Components::MaterialComponent>(
        entity.components[materialIndex].data) };
    auto& layer{ *std::static_pointer_cast<Visconfig::Components::LayerComponent>(entity.components[layerIndex].data) };
    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transformIndex].data) };

    mesh.asset = meshName;

    auto textureFront{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto textureSide{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto textureTop{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuseColor{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    textureFront->asset = frontTexture;
    textureFront->slot = 0;

    textureSide->asset = sideTexture;
    textureSide->slot = 1;

    textureTop->asset = topTexture;
    textureTop->slot = 2;

    diffuseColor->value[0] = color[0];
    diffuseColor->value[1] = color[1];
    diffuseColor->value[2] = color[2];
    diffuseColor->value[3] = color[3];

    material.asset = cube_shader_asset;
    material.attributes.insert_or_assign("gridTextureFront",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureFront, false });
    material.attributes.insert_or_assign("gridTextureSide",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureSide, false });
    material.attributes.insert_or_assign("gridTextureTop",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureTop, false });
    material.attributes.insert_or_assign("diffuseColor",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuseColor, false });

    layer.mask = layerId;

    transform.position[0] = position[0];
    transform.position[1] = position[1];
    transform.position[2] = position[2];

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = scale[0];
    transform.scale[1] = scale[1];
    transform.scale[2] = scale[2];

    if (hasParent) {
        auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
            entity.components[parentIndex].data) };
        parent.id = parentId;
    }

    return entity;
}

Visconfig::Entity generateOutputViewCube(Visconfig::World& world, const MDH2Vis::ProcessedConfig& mdhConfig,
    const MDH2Vis::detail::OutputViewInfo& view, std::size_t& entityId, std::size_t parentId, std::size_t viewNumber,
    std::size_t layerNumber, const std::string& cubeTexture, const std::string& meshName)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Parent, std::make_shared<Visconfig::Components::ParentComponent>() });

    if (layerNumber > 0) {
        entity.components.push_back({ Visconfig::Components::ComponentType::ImplicitIteration,
            std::make_shared<Visconfig::Components::ImplicitIterationComponent>() });
    }

    constexpr auto transformIndex{ 0 };
    constexpr auto parentIndex{ 1 };
    constexpr auto iterationIndex{ 2 };

    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transformIndex].data) };
    auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
        entity.components[parentIndex].data) };

    std::array<float, 3> halfScale{
        view.layers[layerNumber].size[0] / 2.0f,
        view.layers[layerNumber].size[1] / 2.0f,
        view.layers[layerNumber].size[2] / 2.0f,
    };

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = view.layers[layerNumber].size[0];
    transform.scale[1] = view.layers[layerNumber].size[1];
    transform.scale[2] = view.layers[layerNumber].size[2];

    transform.position[0] = -0.5f + halfScale[0];
    transform.position[1] = 0.5f - halfScale[1];
    transform.position[2] = -0.5f + halfScale[2];

    std::array<float, 4> color{
        static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[0]) / 255.0f,
        static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[1]) / 255.0f,
        static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[2]) / 255.0f,
        interpolateLinear<std::size_t>(min_transparency, max_transparency, 0ull,
            mdhConfig.main_view.layers.size() + mdhConfig.main_view.threads.size() - 1, layerNumber),
    };

    auto& threadLayer{ mdhConfig.main_view.threads.front() };
    std::array<float, 3> childScale{
        threadLayer.absolute_scale[0] / mdhConfig.output_view.layers[layerNumber].absolute_size[0],
        threadLayer.absolute_scale[1] / mdhConfig.output_view.layers[layerNumber].absolute_size[1],
        threadLayer.absolute_scale[2] / mdhConfig.output_view.layers[layerNumber].absolute_size[2],
    };

    std::array<float, 3> childStartPos{
        -0.5f,
        0.5f,
        -0.5f,
    };

    auto parentEntity{ entityId };

    auto child{ generateCube(childStartPos, childScale, color, ++entityId, parentEntity, 1llu << viewNumber, true,
        cubeTexture, cubeTexture, cubeTexture, meshName) };
    child.components.push_back({ Visconfig::Components::ComponentType::MeshIteration,
        std::make_shared<Visconfig::Components::MeshIterationComponent>() });

    auto& childMeshIteration{ *std::static_pointer_cast<Visconfig::Components::MeshIterationComponent>(
        child.components.back().data) };
    childMeshIteration.dimensions = {
        static_cast<std::size_t>(
            std::abs(view.layers[layerNumber].absolute_size[0] / mdhConfig.main_view.threads[0].absolute_scale[0])),
        static_cast<std::size_t>(
            std::abs(view.layers[layerNumber].absolute_size[1] / mdhConfig.main_view.threads[0].absolute_scale[1])),
        static_cast<std::size_t>(
            std::abs(view.layers[layerNumber].absolute_size[2] / mdhConfig.main_view.threads[0].absolute_scale[2])),
    };
    childMeshIteration.positions = view.layers[layerNumber].grid_positions;
    childMeshIteration.ticksPerIteration = view.layers[layerNumber].iteration_rates;

    world.entities.push_back(child);

    parent.id = parentId;

    if (layerNumber > 0) {
        auto& iteration{ *std::static_pointer_cast<Visconfig::Components::ImplicitIterationComponent>(
            entity.components[iterationIndex].data) };

        iteration.order = Visconfig::Components::IterationOrder::XYZ;
        iteration.numIterations[0] = view.layers[layerNumber].num_iterations[0];
        iteration.numIterations[1] = view.layers[layerNumber].num_iterations[1];
        iteration.numIterations[2] = view.layers[layerNumber].num_iterations[2];
        iteration.ticksPerIteration = view.layers[layerNumber].iteration_rate;
    }

    return entity;
}

Visconfig::Entity generateSubViewCube(const MDH2Vis::ProcessedConfig& mdhConfig,
    const MDH2Vis::detail::SubViewInfo& view, std::size_t entityId, std::size_t parentId, std::size_t viewNumber,
    std::size_t layerNumber, const std::string& frontTexture, const std::string& sideTexture,
    const std::string& topTexture)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back(
        { Visconfig::Components::ComponentType::Cube, std::make_shared<Visconfig::Components::CubeComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Mesh, std::make_shared<Visconfig::Components::MeshComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Material,
        std::make_shared<Visconfig::Components::MaterialComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Layer, std::make_shared<Visconfig::Components::LayerComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Parent, std::make_shared<Visconfig::Components::ParentComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::ExplicitIteration,
        std::make_shared<Visconfig::Components::ExplicitIterationComponent>() });

    [[maybe_unused]] constexpr auto cubeIndex{ 0 };
    constexpr auto meshIndex{ 1 };
    constexpr auto materialIndex{ 2 };
    constexpr auto layerIndex{ 3 };
    constexpr auto transformIndex{ 4 };
    constexpr auto parentIndex{ 5 };
    constexpr auto iterationIndex{ 6 };

    auto& mesh{ *std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[meshIndex].data) };
    auto& material{ *std::static_pointer_cast<Visconfig::Components::MaterialComponent>(
        entity.components[materialIndex].data) };
    auto& layer{ *std::static_pointer_cast<Visconfig::Components::LayerComponent>(entity.components[layerIndex].data) };
    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transformIndex].data) };
    auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
        entity.components[parentIndex].data) };
    auto& iteration{ *std::static_pointer_cast<Visconfig::Components::ExplicitIterationComponent>(
        entity.components[iterationIndex].data) };

    mesh.asset = cube_mesh_asset;

    auto textureFront{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto textureSide{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto textureTop{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuseColor{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    textureFront->asset = frontTexture;
    textureFront->slot = 0;

    textureSide->asset = sideTexture;
    textureSide->slot = 1;

    textureTop->asset = topTexture;
    textureTop->slot = 2;

    std::size_t colorLayer{ 0 };

    if (mdhConfig.config[layerNumber].tps.memRegionInp.contains(view.name)) {
        colorLayer = mdhConfig.config[layerNumber].tps.memRegionInp.at(view.name);
    } else {
        colorLayer = mdhConfig.config[layerNumber].tps.memRegionRes.at(view.name);
    }

    diffuseColor->value[0] = static_cast<float>(mdhConfig.config[colorLayer].model.colors.memory[0]) / 255.0f;
    diffuseColor->value[1] = static_cast<float>(mdhConfig.config[colorLayer].model.colors.memory[1]) / 255.0f;
    diffuseColor->value[2] = static_cast<float>(mdhConfig.config[colorLayer].model.colors.memory[2]) / 255.0f;
    diffuseColor->value[3] = static_cast<float>(mdhConfig.config[colorLayer].model.colors.memory[3]) / 255.0f;
    diffuseColor->value[3]
        = interpolateLinear<std::size_t>(min_transparency, max_transparency, 0ull, view.layers.size() - 1, layerNumber);
    diffuseColor->value[3] = interpolateLinear<std::size_t>(min_transparency, max_transparency, 0ull,
        mdhConfig.main_view.layers.size() + mdhConfig.main_view.threads.size() - 1, layerNumber);

    material.asset = cube_shader_asset;
    material.attributes.insert_or_assign("gridTextureFront",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureFront, false });
    material.attributes.insert_or_assign("gridTextureSide",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureSide, false });
    material.attributes.insert_or_assign("gridTextureTop",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, textureTop, false });
    material.attributes.insert_or_assign("diffuseColor",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuseColor, false });

    layer.mask = 1llu << (viewNumber + 2);

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = view.layers[layerNumber].scale[0];
    transform.scale[1] = view.layers[layerNumber].scale[1];
    transform.scale[2] = view.layers[layerNumber].scale[2];

    std::array<float, 3> halfScale{
        view.layers[layerNumber].scale[0] / 2.0f,
        view.layers[layerNumber].scale[1] / 2.0f,
        view.layers[layerNumber].scale[2] / 2.0f,
    };

    if (layerNumber > 0) {
        transform.position[0] = -0.5f + halfScale[0];
        transform.position[1] = 0.5f - halfScale[1];
        transform.position[2] = -0.5f + halfScale[2];
    } else {
        transform.position[0] = 0.0f;
        transform.position[1] = 0.0f;
        transform.position[2] = 0.0f;
    }

    parent.id = parentId;

    iteration.positions = view.layers[layerNumber].positions;
    iteration.ticksPerIteration = view.layers[layerNumber].iteration_rate;

    return entity;
}

Visconfig::Entity generateViewCamera(std::size_t entityId, std::size_t focus, std::size_t viewIndex,
    const std::string& framebufferName, float fov, float aspect, float near, float far, float distance,
    float orthographicWidth, float orthographicHeight, bool active, bool fixed, bool perspective)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back(
        { Visconfig::Components::ComponentType::Camera, std::make_shared<Visconfig::Components::CameraComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::FreeFlyCamera,
        std::make_shared<Visconfig::Components::FreeFlyCameraComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::FixedCamera,
        std::make_shared<Visconfig::Components::FixedCameraComponent>() });

    auto camera{ std::static_pointer_cast<Visconfig::Components::CameraComponent>(entity.components[0].data) };
    camera->active = active;
    camera->fixed = fixed;
    camera->perspective = perspective;
    camera->fov = fov;
    camera->aspect = aspect;
    camera->near = near;
    camera->far = far;
    camera->orthographicWidth = orthographicWidth;
    camera->orthographicHeight = orthographicHeight;
    camera->layerMask = 1ull << viewIndex;
    camera->targets.insert_or_assign("cube", framebufferName);
    camera->targets.insert_or_assign("text", framebufferName);

    auto transform{ std::static_pointer_cast<Visconfig::Components::TransformComponent>(entity.components[1].data) };
    transform->rotation[0] = 0.0f;
    transform->rotation[1] = 0.0f;
    transform->rotation[2] = 0.0f;

    transform->position[0] = 0.0f;
    transform->position[1] = 0.0f;
    transform->position[2] = distance;

    transform->scale[0] = 1.0f;
    transform->scale[1] = 1.0f;
    transform->scale[2] = 1.0f;

    auto fixedCamera{ std::static_pointer_cast<Visconfig::Components::FixedCameraComponent>(
        entity.components[3].data) };
    fixedCamera->focus = focus;
    fixedCamera->distance = distance;
    fixedCamera->horizontalAngle = 0.0f;
    fixedCamera->verticalAngle = 0.0f;

    return entity;
}

Visconfig::Entity generateCoordinatorEntity(std::size_t entityId)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back({ Visconfig::Components::ComponentType::Composition,
        std::make_shared<Visconfig::Components::CompositionComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::CameraSwitcher,
        std::make_shared<Visconfig::Components::CameraSwitcherComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Copy, std::make_shared<Visconfig::Components::CopyComponent>() });

    return entity;
}

void extentComposition(Visconfig::World& world, std::array<float, 2> scale, std::array<float, 2> position,
    std::vector<std::string> src, const std::string& target, const std::string& shader, std::size_t id, bool draggable)
{
    auto composition{ std::static_pointer_cast<Visconfig::Components::CompositionComponent>(
        world.entities[0].components[0].data) };

    composition->operations.push_back(Visconfig::Components::CompositionOperation{
        { scale[0], scale[1] }, { position[0], position[1] }, std::move(src), target, shader, id, draggable });
}

void extendCameraSwitcher(Visconfig::World& world, std::size_t camera)
{
    auto cameraSwitcher{ std::static_pointer_cast<Visconfig::Components::CameraSwitcherComponent>(
        world.entities[0].components[1].data) };

    cameraSwitcher->cameras.push_back(camera);
}

void extendCopy(Visconfig::World& world, const std::string& source, const std::string& destination,
    const std::vector<Visconfig::Components::CopyOperationFlag>& flags,
    Visconfig::Components::CopyOperationFilter filter)
{
    auto copyComponent{ std::static_pointer_cast<Visconfig::Components::CopyComponent>(
        world.entities[0].components[2].data) };

    copyComponent->operations.push_back(Visconfig::Components::CopyOperation{ source, destination, flags, filter });
}

void generateMainViewConfig(Visconfig::Config& config, const MDH2Vis::ProcessedConfig& mdhConfig,
    Visconfig::World& world, std::size_t& numEntities, const std::filesystem::path& workingDir)
{
    constexpr auto renderTextureName{ "render_texture_0" };
    constexpr auto depthBufferName{ "renderbuffer_depth_0" };
    constexpr auto framebufferName{ "framebuffer_0" };

    config.assets.push_back(generateRenderTextureAsset(
        renderTextureName, render_resolution_width / 2,
        render_resolution_height, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferName, render_resolution_width / 2,
        render_resolution_height, 0, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferName, 0, 0, render_resolution_width / 2, render_resolution_height,
            { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
                  renderTextureName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferName } }));

    constexpr auto renderTextureMultisampleName{ "render_texture_0_multisample" };
    constexpr auto depthBufferMultisampleName{ "renderbuffer_depth_0_multisample" };
    constexpr auto framebufferMultisampleName{ "framebuffer_0_multisample" };

    config.assets.push_back(generateMultisampleRenderTextureAsset(renderTextureMultisampleName, render_resolution_width / 2,
            render_resolution_height, screen_msaa_samples, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferMultisampleName, render_resolution_width / 2,
        render_resolution_height, screen_msaa_samples, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferMultisampleName, 0, 0, render_resolution_width / 2,
        render_resolution_height,
            { { Visconfig::Assets::FramebufferType::TextureMultisample,
                  Visconfig::Assets::FramebufferDestination::Color0, renderTextureMultisampleName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferMultisampleName } }));

    auto focusEntity{ numEntities };

    auto threadTextureBorderWidth = static_cast<std::size_t>(thread_view_texture_border_relative_width
        * std::pow(mdhConfig.main_view.threads.front().absolute_scale[0]
                * mdhConfig.main_view.threads.front().absolute_scale[1]
                * mdhConfig.main_view.threads.front().absolute_scale[2],
            1.0f / 3.0f));

    threadTextureBorderWidth = threadTextureBorderWidth == 0 ? 1 : threadTextureBorderWidth;

    for (auto layer{ mdhConfig.main_view.layers.begin() }; layer != mdhConfig.main_view.layers.end(); layer++) {
        auto index{ std::distance(mdhConfig.main_view.layers.begin(), layer) };

        auto mainTextureBorderWidth = static_cast<std::size_t>(main_view_texture_border_relative_width
            * std::pow(layer->absolute_scale[0] * layer->absolute_scale[1] * layer->absolute_scale[2], 1.0f / 3.0f));

        mainTextureBorderWidth = mainTextureBorderWidth == 0 ? 1 : mainTextureBorderWidth;

        auto textureFrontName{ "view_0_cube_texture_" + std::to_string(index) + "_front" };
        auto textureSideName{ "view_0_cube_texture_" + std::to_string(index) + "_side" };
        auto textureTopName{ "view_0_cube_texture_" + std::to_string(index) + "_top" };

        auto textureFrontPath{ (workingDir
            / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureFrontName + ".png" })
                                   .string() };
        auto textureSidePath{ (workingDir
            / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureSideName + ".png" })
                                  .string() };
        auto textureTopPath{ (
            workingDir / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureTopName + ".png" })
                                 .string() };

        if (static_cast<std::size_t>(index) == mdhConfig.main_view.layers.size() - 1) {
            generateTextureFile(textureFrontPath, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[1]), 1, 1, mainTextureBorderWidth);
            generateTextureFile(textureSidePath, static_cast<std::size_t>(layer->absolute_scale[2]),
                static_cast<std::size_t>(layer->absolute_scale[1]), 1, 1, mainTextureBorderWidth);
            generateTextureFile(textureTopPath, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[2]), 1, 1, mainTextureBorderWidth);
        } else {
            auto currentLayerScale{ layer->absolute_scale };
            auto nextLayerScale{ (layer + 1)->absolute_scale };

            std::array<std::size_t, 3> subdivisions{
                static_cast<std::size_t>(currentLayerScale[0] / nextLayerScale[0]),
                static_cast<std::size_t>(currentLayerScale[1] / nextLayerScale[1]),
                static_cast<std::size_t>(currentLayerScale[2] / nextLayerScale[2]),
            };

            generateTextureFile(textureFrontPath, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[1]), subdivisions[0], subdivisions[1],
                mainTextureBorderWidth);
            generateTextureFile(textureSidePath, static_cast<std::size_t>(layer->absolute_scale[2]),
                static_cast<std::size_t>(layer->absolute_scale[1]), subdivisions[2], subdivisions[1],
                mainTextureBorderWidth);
            generateTextureFile(textureTopPath, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[2]), subdivisions[0], subdivisions[2],
                mainTextureBorderWidth);
        }

        config.assets.push_back(createTextureAsset(textureFrontName, textureFrontPath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(createTextureAsset(textureSideName, textureSidePath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(createTextureAsset(textureTopName, textureTopPath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));

        world.entities.push_back(generateMainViewCube(mdhConfig, mdhConfig.main_view, numEntities, numEntities - 1,
            index, textureFrontName, textureSideName, textureTopName));
        numEntities++;
    }

    std::vector<std::size_t> threadParents{ numEntities - 1 };

    for (auto layer{ mdhConfig.main_view.threads.begin() }; layer != mdhConfig.main_view.threads.end(); layer++) {
        std::vector<std::size_t> newParents{};
        auto index{ std::distance(mdhConfig.main_view.threads.begin(), layer) };

        auto textureFrontName{ "view_0_cube_texture_" + std::to_string(index + mdhConfig.main_view.layers.size())
            + "_front" };
        auto textureSideName{ "view_0_cube_texture_" + std::to_string(index + mdhConfig.main_view.layers.size())
            + "_side" };
        auto textureTopName{ "view_0_cube_texture_" + std::to_string(index + mdhConfig.main_view.layers.size())
            + "_top" };

        auto textureFrontPath{ (workingDir
            / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureFrontName + ".png" })
                                   .string() };
        auto textureSidePath{ (workingDir
            / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureSideName + ".png" })
                                  .string() };
        auto textureTopPath{ (
            workingDir / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureTopName + ".png" })
                                 .string() };

        if (static_cast<std::size_t>(index) == mdhConfig.main_view.threads.size() - 1) {
            generateTextureFile(textureFrontPath, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[1]), 1, 1, threadTextureBorderWidth);
            generateTextureFile(textureSidePath, static_cast<std::size_t>(layer->absolute_scale[2]),
                static_cast<std::size_t>(layer->absolute_scale[1]), 1, 1, threadTextureBorderWidth);
            generateTextureFile(textureTopPath, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[2]), 1, 1, threadTextureBorderWidth);
        } else {
            auto currentLayerScale{ layer->absolute_scale };
            auto nextLayerScale{ (layer + 1)->absolute_scale };

            std::array<std::size_t, 3> subdivisions{
                static_cast<std::size_t>(currentLayerScale[0] / nextLayerScale[0]),
                static_cast<std::size_t>(currentLayerScale[1] / nextLayerScale[1]),
                static_cast<std::size_t>(currentLayerScale[2] / nextLayerScale[2]),
            };

            generateTextureFile(textureFrontPath, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[1]), subdivisions[0], subdivisions[1],
                threadTextureBorderWidth);
            generateTextureFile(textureSidePath, static_cast<std::size_t>(layer->absolute_scale[2]),
                static_cast<std::size_t>(layer->absolute_scale[1]), subdivisions[2], subdivisions[1],
                threadTextureBorderWidth);
            generateTextureFile(textureTopPath, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[2]), subdivisions[0], subdivisions[2],
                threadTextureBorderWidth);
        }

        config.assets.push_back(createTextureAsset(textureFrontName, textureFrontPath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(createTextureAsset(textureSideName, textureSidePath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(createTextureAsset(textureTopName, textureTopPath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));

        for (auto& parent : threadParents) {
            for (std::size_t x{ 0 }; x < layer->num_threads[0]; x++) {
                for (std::size_t y{ 0 }; y < layer->num_threads[1]; y++) {
                    for (std::size_t z{ 0 }; z < layer->num_threads[2]; z++) {
                        world.entities.push_back(generateMainViewThreadCube(mdhConfig, mdhConfig.main_view, numEntities,
                            parent, index, { x, y, z }, textureFrontName, textureSideName, textureTopName));
                        newParents.push_back(numEntities++);
                    }
                }
            }
        }
        threadParents = std::move(newParents);
    }

    auto cameraDistance{ 2.0f
        * std::max({ mdhConfig.main_view.layers[0].absolute_scale[0], mdhConfig.main_view.layers[0].absolute_scale[1],
            mdhConfig.main_view.layers[0].absolute_scale[2] }) };

    auto cameraWidth{ 1.2f
        * std::max({ mdhConfig.main_view.layers[0].absolute_scale[0], mdhConfig.main_view.layers[0].absolute_scale[1] }) };
    auto cameraHeight{ cameraWidth / camera_aspect_small };

    auto cameraEntity{ numEntities++ };
    world.entities.push_back(generateViewCamera(cameraEntity, focusEntity, 0, framebufferMultisampleName, camera_fov,
        camera_aspect_small, camera_near, camera_far, cameraDistance, cameraWidth, cameraHeight, true, false,
        mdhConfig.main_view.layers[0].absolute_scale[2] != 1.0f));
    extendCopy(world, framebufferMultisampleName, framebufferName,
        { Visconfig::Components::CopyOperationFlag::Color, Visconfig::Components::CopyOperationFlag::Depth },
        Visconfig::Components::CopyOperationFilter::Nearest);
    extentComposition(world, { 0.5f, 1.0f }, { -0.5f, 0.0f }, { renderTextureName }, default_framebuffer_asset,
        view_composition_shader_asset, 0, false);
    extendCameraSwitcher(world, cameraEntity);
}

void generateSubViewConfig(Visconfig::Config& config, const MDH2Vis::ProcessedConfig& mdhConfig,
    Visconfig::World& world, std::size_t& numEntities, std::size_t subview, std::size_t numSubViews,
    const std::filesystem::path& workingDir)
{
    auto renderTextureName{ "render_texture_" + std::to_string(subview + 2) };
    auto depthBufferName{ "renderbuffer_depth_" + std::to_string(subview + 2) };
    auto framebufferName{ "framebuffer_" + std::to_string(subview + 2) };

    config.assets.push_back(generateRenderTextureAsset(
        renderTextureName, render_resolution_width, render_resolution_height, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferName, render_resolution_width,
        render_resolution_height, 0,
        Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferName, 0, 0, render_resolution_width, render_resolution_height,
            { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
                  renderTextureName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferName } }));

    auto renderTextureMultisampleName{ renderTextureName + "_multisample" };
    auto depthBufferMultisampleName{ depthBufferName + "_multisample" };
    auto framebufferMultisampleName{ framebufferName + "_multisample" };

    config.assets.push_back(generateMultisampleRenderTextureAsset(renderTextureMultisampleName, render_resolution_width,
        render_resolution_height, screen_msaa_samples, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferMultisampleName, render_resolution_width,
        render_resolution_height, screen_msaa_samples, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferMultisampleName, 0, 0, render_resolution_width, render_resolution_height,
            { { Visconfig::Assets::FramebufferType::TextureMultisample,
                  Visconfig::Assets::FramebufferDestination::Color0, renderTextureMultisampleName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferMultisampleName } }));

    auto textureBorderWidth = static_cast<std::size_t>(sub_view_texture_border_relative_width
        * std::pow(mdhConfig.sub_views[subview].layers.front().absolute_scale[0]
                * mdhConfig.sub_views[subview].layers.front().absolute_scale[1]
                * mdhConfig.sub_views[subview].layers.front().absolute_scale[2],
            1.0f / 3.0f));

    textureBorderWidth = textureBorderWidth == 0 ? 1 : textureBorderWidth;

    auto focusEntity{ numEntities };
    for (auto layer{ mdhConfig.config.begin() }; layer != mdhConfig.config.end(); layer++) {
        auto index{ std::distance(mdhConfig.config.begin(), layer) };

        auto textureFrontName{ "view_" + std::to_string(subview + 2) + "_cube_texture_" + std::to_string(index)
            + "_front" };
        auto textureSideName{ "view_" + std::to_string(subview + 2) + "_cube_texture_" + std::to_string(index)
            + "_side" };
        auto textureTopName{ "view_" + std::to_string(subview + 2) + "_cube_texture_" + std::to_string(index)
            + "_top" };

        auto textureFrontPath{ (workingDir
            / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureFrontName + ".png" })
                                   .string() };
        auto textureSidePath{ (workingDir
            / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureSideName + ".png" })
                                  .string() };
        auto textureTopPath{ (
            workingDir / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureTopName + ".png" })
                                 .string() };

        generateTextureFile(textureFrontPath,
            static_cast<std::size_t>(mdhConfig.sub_views[subview].layers[index].absolute_scale[0]),
            static_cast<std::size_t>(mdhConfig.sub_views[subview].layers[index].absolute_scale[1]), 1, 1,
            textureBorderWidth);
        generateTextureFile(textureSidePath,
            static_cast<std::size_t>(mdhConfig.sub_views[subview].layers[index].absolute_scale[2]),
            static_cast<std::size_t>(mdhConfig.sub_views[subview].layers[index].absolute_scale[1]), 1, 1,
            textureBorderWidth);
        generateTextureFile(textureTopPath,
            static_cast<std::size_t>(mdhConfig.sub_views[subview].layers[index].absolute_scale[0]),
            static_cast<std::size_t>(mdhConfig.sub_views[subview].layers[index].absolute_scale[2]), 1, 1,
            textureBorderWidth);

        config.assets.push_back(createTextureAsset(textureFrontName, textureFrontPath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(createTextureAsset(textureSideName, textureSidePath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(createTextureAsset(textureTopName, textureTopPath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));

        world.entities.push_back(generateSubViewCube(mdhConfig, mdhConfig.sub_views[subview], numEntities,
            numEntities - 1, subview, index, textureFrontName, textureSideName, textureTopName));
        numEntities++;
    }

    auto cameraDistance{ 1.5f
        * std::max({ mdhConfig.sub_views[subview].layers[0].absolute_scale[0],
            mdhConfig.sub_views[subview].layers[0].absolute_scale[1],
            mdhConfig.sub_views[subview].layers[0].absolute_scale[2] }) };

    auto cameraWidth{ 1.2f
        * std::max({ mdhConfig.sub_views[subview].layers[0].absolute_scale[0],
            mdhConfig.sub_views[subview].layers[0].absolute_scale[1] }) };
    auto cameraHeight{ cameraWidth / camera_aspect };

    auto cameraEntity{ numEntities++ };
    world.entities.push_back(generateViewCamera(cameraEntity, focusEntity, subview + 2, framebufferMultisampleName,
        camera_fov, camera_aspect, camera_near, camera_far, cameraDistance, cameraWidth, cameraHeight, false, true,
        mdhConfig.sub_views[subview].layers[0].absolute_scale[2] != 1.0f));
    extendCopy(world, framebufferMultisampleName, framebufferName,
        { Visconfig::Components::CopyOperationFlag::Color, Visconfig::Components::CopyOperationFlag::Depth },
        Visconfig::Components::CopyOperationFilter::Nearest);
    extentComposition(world, { 0.2f, 0.2f }, { 0.7f, (-0.7f + ((numSubViews - 1) * 0.5f)) - (subview * 0.5f) },
        { renderTextureName }, default_framebuffer_asset, view_composition_shader_asset, subview + 2, true);
    extendCameraSwitcher(world, cameraEntity);
}

void generateOutputViewConfig(Visconfig::Config& config, const MDH2Vis::ProcessedConfig& mdhConfig,
    Visconfig::World& world, std::size_t& numEntities, std::size_t subview, const std::filesystem::path& workingDir)
{
    auto renderTextureName{ "render_texture_" + std::to_string(subview) };
    auto depthBufferName{ "renderbuffer_depth_" + std::to_string(subview) };
    auto framebufferName{ "framebuffer_" + std::to_string(subview) };

    config.assets.push_back(generateRenderTextureAsset(
        renderTextureName, render_resolution_width / 2,
        render_resolution_height, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferName, render_resolution_width / 2,
        render_resolution_height, 0, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferName, 0, 0, render_resolution_width / 2, render_resolution_height,
            { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
                  renderTextureName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferName } }));

    auto renderTextureMultisampleName{ renderTextureName + "_multisample" };
    auto depthBufferMultisampleName{ depthBufferName + "_multisample" };
    auto framebufferMultisampleName{ framebufferName + "_multisample" };

    config.assets.push_back(generateMultisampleRenderTextureAsset(renderTextureMultisampleName, render_resolution_width / 2,
            render_resolution_height, screen_msaa_samples, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferMultisampleName, render_resolution_width / 2,
        render_resolution_height, screen_msaa_samples, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferMultisampleName, 0, 0, render_resolution_width / 2,
        render_resolution_height,
            { { Visconfig::Assets::FramebufferType::TextureMultisample,
                  Visconfig::Assets::FramebufferDestination::Color0, renderTextureMultisampleName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferMultisampleName } }));

    auto textureBorderWidth = static_cast<std::size_t>(main_view_texture_border_relative_width
        * std::pow(mdhConfig.main_view.layers[0].absolute_scale[0] * mdhConfig.main_view.layers[0].absolute_scale[1]
                * mdhConfig.main_view.layers[0].absolute_scale[2],
            1.0f / 3.0f));

    textureBorderWidth = textureBorderWidth == 0 ? 1 : textureBorderWidth;

    auto textureFrontName{ "view_" + std::to_string(subview) + "_cube_texture_0_front" };
    auto textureSideName{ "view_" + std::to_string(subview) + "_cube_texture_0_side" };
    auto textureTopName{ "view_" + std::to_string(subview) + "_cube_texture_0_top" };
    auto innerLayerTextureName{ "view_" + std::to_string(subview) + "_cube_texture_1" };

    auto textureFrontPath{ (
        workingDir / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureFrontName + ".png" })
                               .string() };
    auto textureSidePath{ (
        workingDir / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureSideName + ".png" })
                              .string() };
    auto textureTopPath{ (
        workingDir / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + textureTopName + ".png" })
                             .string() };
    auto innerLayerTexturePath{ (workingDir
        / std::filesystem::path{ std::string{ assets_texture_directory } + "/" + innerLayerTextureName + ".png" })
                                    .string() };

    generateTextureFile(textureFrontPath, static_cast<std::size_t>(mdhConfig.output_view.size[0]),
        static_cast<std::size_t>(mdhConfig.output_view.size[1]), 1, 1, textureBorderWidth);
    generateTextureFile(textureSidePath, static_cast<std::size_t>(mdhConfig.output_view.size[2]),
        static_cast<std::size_t>(mdhConfig.output_view.size[1]), 1, 1, textureBorderWidth);
    generateTextureFile(textureTopPath, static_cast<std::size_t>(mdhConfig.output_view.size[0]),
        static_cast<std::size_t>(mdhConfig.output_view.size[2]), 1, 1, textureBorderWidth);
    generateTextureFile(innerLayerTexturePath, static_cast<std::size_t>(mdhConfig.output_view.size[0]),
        static_cast<std::size_t>(mdhConfig.output_view.size[1]), 1, 1, 0);

    config.assets.push_back(createTextureAsset(textureFrontName, textureFrontPath,
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
    config.assets.push_back(createTextureAsset(textureSideName, textureSidePath,
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
    config.assets.push_back(createTextureAsset(textureTopName, textureTopPath,
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
    config.assets.push_back(createTextureAsset(innerLayerTextureName, innerLayerTexturePath,
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps }));

    auto focusEntity{ numEntities };
    world.entities.push_back(generateCube({ 0, 0, 0 }, mdhConfig.output_view.size, { 0.0f, 0.0f, 0.0f, 0.0f },
        numEntities++, 0, 1llu << subview, false, textureFrontName, textureSideName, textureTopName, cube_mesh_asset));

    auto parent{ focusEntity };
    for (auto layer{ mdhConfig.output_view.layers.begin() }; layer != mdhConfig.output_view.layers.end(); layer++) {
        auto index{ std::distance(mdhConfig.output_view.layers.begin(), layer) };

        auto meshName{ "view_" + std::to_string(subview) + "_mesh_" + std::to_string(index) };
        config.assets.push_back(createSimpleCubeMeshAsset(meshName));

        auto newParent{ numEntities };
        world.entities.push_back(generateOutputViewCube(world, mdhConfig, mdhConfig.output_view, numEntities, parent,
            subview, index, innerLayerTextureName, meshName));
        parent = newParent;
        numEntities++;
    }

    auto cameraDistance{ 2.0f
        * std::max({ mdhConfig.output_view.size[0], mdhConfig.output_view.size[1], mdhConfig.output_view.size[2] }) };

    auto cameraWidth{ 1.2f * std::max({ mdhConfig.output_view.size[0], mdhConfig.output_view.size[1] }) };
    auto cameraHeight{ cameraWidth / camera_aspect_small };

    auto cameraEntity{ numEntities++ };
    world.entities.push_back(generateViewCamera(cameraEntity, focusEntity, subview, framebufferMultisampleName,
        camera_fov, camera_aspect_small, camera_near, camera_far, cameraDistance, cameraWidth, cameraHeight, false, false,
        mdhConfig.output_view.size[2] != 1.0f));
    extendCopy(world, framebufferMultisampleName, framebufferName,
        { Visconfig::Components::CopyOperationFlag::Color, Visconfig::Components::CopyOperationFlag::Depth },
        Visconfig::Components::CopyOperationFilter::Nearest);
    extentComposition(world, { 0.5f, 1.0f }, { 0.5f, 0.0f }, { renderTextureName }, default_framebuffer_asset,
        view_composition_shader_asset, subview, false);
    extendCameraSwitcher(world, cameraEntity);
}

void generateWorld(
    Visconfig::Config& config, const MDH2Vis::ProcessedConfig& mdhConfig, const std::filesystem::path& workingDir)
{
    Visconfig::World world{};

    std::size_t numEntities{ 0 };
    world.entities.push_back(generateCoordinatorEntity(numEntities++));
    generateMainViewConfig(config, mdhConfig, world, numEntities, workingDir);
    generateOutputViewConfig(config, mdhConfig, world, numEntities, 1, workingDir);

    for (auto pos{ mdhConfig.sub_views.begin() }; pos != mdhConfig.sub_views.end(); pos++) {
        auto index{ std::distance(mdhConfig.sub_views.begin(), pos) };
        generateSubViewConfig(config, mdhConfig, world, numEntities, index, mdhConfig.sub_views.size(), workingDir);
    }
    config.worlds.push_back(world);
}

Visconfig::Config generate_config(const MDH2Vis::ProcessedConfig& config, const std::filesystem::path& working_dir)
{
    Visconfig::Config visconfig{};

    visconfig.options.screenHeight = screen_height;
    visconfig.options.screenWidth = screen_width;
    visconfig.options.screenMSAASamples = screen_msaa_samples;
    visconfig.options.screenFullscreen = screen_fullscreen;

    visconfig.assets.push_back(createCubeMeshAsset());
    visconfig.assets.push_back(createCubeTextureAsset());
    visconfig.assets.push_back(createOutputCubeTextureAsset());
    visconfig.assets.push_back(createShaderAsset(cube_shader_vertex_path, cube_shader_fragment_path, cube_shader_asset));
    visconfig.assets.push_back(createShaderAsset(
        view_composition_shader_vertex_path, view_composition_shader_fragment_path, view_composition_shader_asset));
    visconfig.assets.push_back(createDefaultFramebufferAsset());

    generateWorld(visconfig, config, working_dir);

    return visconfig;
}