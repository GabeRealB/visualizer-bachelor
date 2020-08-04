#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <thread>

#include <visconfig/Config.hpp>

#include "MDHConfig.hpp"
#include "MDHOps.hpp"

constexpr std::string_view UsageStr{
    "Usage: mdh2vis --model model-path --mdh mdh-path --tps tps-path [--out output-dir]"
};

constexpr std::size_t screenWidth = 1200;
constexpr std::size_t screenHeight = 900;
constexpr bool screenFullscreen = false;

struct SequentialLayer {
    MDH2Vis::Model::Layer model;
    MDH2Vis::TPS::Layer tps;
};

struct LayerInfo {
    std::array<float, 3> scale;
    std::array<std::size_t, 3> iterationRate;
};

struct ViewInfo {
    std::string name;
    std::vector<LayerInfo> layers;
};

struct ProcessedConfig {
    ViewInfo mainView;
    std::vector<ViewInfo> subViews;
    std::vector<SequentialLayer> config;
};

void printConfigInfo(const ProcessedConfig& config);

ProcessedConfig processConfig(const MDH2Vis::MDHConfig& config);

Visconfig::Config generateConfig(const ProcessedConfig& config);

int main(int argc, char* argv[])
{
    if (argc != 7 && argc != 9) {
        std::cerr << UsageStr << std::endl;
        return 1;
    }

    auto workingDir{ std::filesystem::current_path() };

    std::filesystem::path modelPath{};
    std::filesystem::path mdhPath{};
    std::filesystem::path tpsPath{};

    bool modelSet{ false };
    bool mdhSet{ false };
    bool tpsSet{ false };

    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], "--model") == 0) {
            modelPath = argv[i + 1];
            modelSet = true;
            ++i;
        } else if (std::strcmp(argv[i], "--mdh") == 0) {
            mdhPath = argv[i + 1];
            mdhSet = true;
            ++i;
        } else if (std::strcmp(argv[i], "--tps") == 0) {
            tpsPath = argv[i + 1];
            tpsSet = true;
            ++i;
        } else if (std::strcmp(argv[i], "--out") == 0) {
            workingDir = argv[i + 1];
            ++i;
        } else {
            std::cerr << UsageStr << std::endl;
            return 1;
        }
    }

    if (!modelSet || !mdhSet || !tpsSet) {
        std::cerr << UsageStr << std::endl;
        return 1;
    }

    std::cout << "Output path: " << workingDir << std::endl << std::endl;

    if (!std::filesystem::exists(workingDir)) {
        std::cerr << "Could not find " << workingDir << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(modelPath)) {
        std::cerr << "Could not find " << modelPath << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(mdhPath)) {
        std::cerr << "Could not find " << mdhPath << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(tpsPath)) {
        std::cerr << "Could not find " << tpsPath << std::endl;
        return 1;
    }

    auto mdhConfig{ MDH2Vis::loadFromFiles(modelPath, mdhPath, tpsPath) };
    if (!mdhConfig) {
        std::cerr << "Could not load configs" << std::endl;
        return 1;
    }

    auto config{ processConfig(*mdhConfig) };
    printConfigInfo(config);

    auto visConfig{ generateConfig(config) };
    Visconfig::to_file(workingDir / "visconfig.json", visConfig);
}

void printConfigInfo(const ProcessedConfig& config)
{
    std::cout << "Config info:" << std::endl;

    std::cout << "Number of layers: " << config.config.size() << std::endl;
    std::cout << "Dimensions: " << config.config[0].tps.tileSize[0] << ", " << config.config[0].tps.tileSize[1] << ", "
              << config.config[0].tps.tileSize[2] << std::endl;
    std::cout << std::endl;
}

void processView(const MDH2Vis::MDHConfig& mdhConfig, ProcessedConfig& config)
{
    for (auto& tps : mdhConfig.tps) {
        config.mainView.layers.push_back(
            LayerInfo{ { static_cast<float>(tps.second.tileSize[0]), static_cast<float>(tps.second.tileSize[1]),
                           static_cast<float>(tps.second.tileSize[2]) },
                { 0, 0, 0 } });
    }

    for (auto pos{ config.mainView.layers.begin() + 1 }; pos != config.mainView.layers.end(); pos++) {

        auto index{ std::distance(config.mainView.layers.begin(), pos) };

        const auto& currentLayer{ config.config[index] };
        const auto& previousLayer{ config.config[index - 1] };

        pos->scale = {
            static_cast<float>(currentLayer.tps.tileSize[0]) / static_cast<float>(previousLayer.tps.tileSize[0]),
            static_cast<float>(currentLayer.tps.tileSize[1]) / static_cast<float>(previousLayer.tps.tileSize[1]),
            static_cast<float>(currentLayer.tps.tileSize[2]) / static_cast<float>(previousLayer.tps.tileSize[2]),
        };

        pos->iterationRate = {
            (previousLayer.tps.tileSize[0] / currentLayer.tps.tileSize[0]) - 1,
            (previousLayer.tps.tileSize[1] / currentLayer.tps.tileSize[1]) - 1,
            (previousLayer.tps.tileSize[2] / currentLayer.tps.tileSize[2]) - 1,
        };
    }
}

void processView(ProcessedConfig& config, const std::string& name, const MDH2Vis::OperationContainer& operation)
{
    auto computeBounds{ [](const MDH2Vis::OperationContainer& operationContainer, std::size_t maxX, std::size_t maxY,
                            std::size_t maxZ) -> std::array<float, 3> {
        struct BufferBounds {
            std::size_t min;
            std::size_t max;
        };

        constexpr BufferBounds StartBounds{ std::numeric_limits<std::size_t>::max(),
            std::numeric_limits<std::size_t>::min() };

        auto computeBounds{ [](std::array<BufferBounds, 3>& bounds,
                                const MDH2Vis::OperationContainer& operationContainer, std::size_t minX,
                                std::size_t maxX, std::size_t maxY, std::size_t maxZ) {
            auto computeBounds{ [](BufferBounds& bounds, const MDH2Vis::Operation& operation, std::size_t i1,
                                    std::size_t i2, std::size_t i3) {
                auto value{ operation(i1, i2, i3) };

                if (bounds.min > value) {
                    bounds.min = value;
                }

                if (bounds.max < value) {
                    bounds.max = value;
                }
            } };

            for (std::size_t i1 = minX; i1 < maxX; ++i1) {
                for (std::size_t i2 = 0; i2 < maxY; ++i2) {
                    for (std::size_t i3 = 0; i3 < maxZ; ++i3) {
                        for (std::size_t i = 0; i < operationContainer.x.size(); ++i) {
                            computeBounds(bounds[0], operationContainer.x[i], i1, i2, i3);
                        }
                        for (std::size_t i = 0; i < operationContainer.y.size(); ++i) {
                            computeBounds(bounds[1], operationContainer.y[i], i1, i2, i3);
                        }
                        for (std::size_t i = 0; i < operationContainer.z.size(); ++i) {
                            computeBounds(bounds[2], operationContainer.z[i], i1, i2, i3);
                        }
                    }
                }
            }
        } };

        auto numThreads{ std::thread::hardware_concurrency() };

        auto xStep{ maxX };
        if (numThreads != 1) {
            xStep = maxX / numThreads;
        }

        std::vector<std::array<BufferBounds, 3>> buffer{};
        buffer.resize(numThreads, { StartBounds, StartBounds, StartBounds });

        std::vector<std::thread> threads{};
        threads.reserve(numThreads);

        auto xMinCmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[0].min < rhs[0].min;
        } };
        auto xMaxCmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[0].max < rhs[0].max;
        } };

        auto yMinCmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[1].min < rhs[1].min;
        } };
        auto yMaxCmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[1].max < rhs[1].max;
        } };

        auto zMinCmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[2].min < rhs[2].min;
        } };
        auto zMaxCmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[2].max < rhs[2].max;
        } };

        for (std::size_t i = 0; i < numThreads; i++) {
            if (i == numThreads - 1) {
                threads.push_back(std::thread(
                    computeBounds, std::ref(buffer[i]), std::ref(operationContainer), i * xStep, maxX, maxY, maxZ));
            } else {
                threads.push_back(std::thread(computeBounds, std::ref(buffer[i]), std::ref(operationContainer),
                    i * xStep, (i + 1) * xStep, maxY, maxZ));
            }
        }

        for (auto& thread : threads) {
            thread.join();
        }

        std::array<BufferBounds, 3> bounds{};

        bounds[0].min = std::min(bounds[0].min, std::min_element(buffer.begin(), buffer.end(), xMinCmp)->at(0).min);
        bounds[0].max = std::max(bounds[0].max, std::max_element(buffer.begin(), buffer.end(), xMaxCmp)->at(0).max);

        bounds[1].min = std::min(bounds[1].min, std::min_element(buffer.begin(), buffer.end(), yMinCmp)->at(1).min);
        bounds[1].max = std::max(bounds[1].max, std::max_element(buffer.begin(), buffer.end(), yMaxCmp)->at(1).max);

        bounds[2].min = std::min(bounds[2].min, std::min_element(buffer.begin(), buffer.end(), zMinCmp)->at(2).min);
        bounds[2].max = std::max(bounds[2].max, std::max_element(buffer.begin(), buffer.end(), zMaxCmp)->at(2).max);

        return {
            static_cast<float>(bounds[0].max - bounds[0].min + 1),
            static_cast<float>(bounds[1].max - bounds[1].min + 1),
            static_cast<float>(bounds[2].max - bounds[2].min + 1),
        };
    } };

    ViewInfo view{};
    view.name = name;

    std::vector<std::array<float, 3>> constantScales{};

    for (const auto& layer : config.config) {
        auto scale{ computeBounds(operation, layer.tps.tileSize[0], layer.tps.tileSize[1], layer.tps.tileSize[2]) };
        view.layers.push_back(LayerInfo{ scale, { 0, 0, 0 } });
        constantScales.push_back(scale);
    }

    for (auto pos{ view.layers.begin() + 1 }; pos != view.layers.end(); pos++) {

        auto index{ std::distance(view.layers.begin(), pos) };

        const auto& currentLayer{ config.config[index] };
        const auto& previousLayer{ config.config[index - 1] };

        pos->scale = {
            pos->scale[0] / constantScales[index - 1][0],
            pos->scale[1] / constantScales[index - 1][1],
            pos->scale[2] / constantScales[index - 1][2],
        };

        pos->iterationRate = {
            (previousLayer.tps.tileSize[0] / currentLayer.tps.tileSize[0]) - 1,
            (previousLayer.tps.tileSize[1] / currentLayer.tps.tileSize[1]) - 1,
            (previousLayer.tps.tileSize[2] / currentLayer.tps.tileSize[2]) - 1,
        };
    }

    config.subViews.push_back(view);
}

ProcessedConfig processConfig(const MDH2Vis::MDHConfig& mdhConfig)
{
    ProcessedConfig config{};

    for (auto& tps : mdhConfig.tps) {
        config.config.push_back(SequentialLayer{ mdhConfig.model.at(tps.first), tps.second });
    }

    processView(mdhConfig, config);
    for (auto& operation : MDH2Vis::OperationMap::operations()) {
        processView(config, operation.first, operation.second);
    }

    return config;
}

Visconfig::Asset createCubeMeshAsset()
{
    auto meshData{ std::make_shared<Visconfig::Assets::MeshAsset>() };
    Visconfig::Asset asset{ "cube_mesh", Visconfig::Assets::AssetType::Mesh, meshData };

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
        { 1.0f, 0.0f, 0.0f }, // top-left
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

Visconfig::Asset createCubeTextureAsset()
{
    auto textureData{ std::make_shared<Visconfig::Assets::TextureFileAsset>() };
    Visconfig::Asset asset{ "cube_texture", Visconfig::Assets::AssetType::TextureFile, textureData };
    textureData->path = "assets/textures/cube.png";
    textureData->attributes.push_back(Visconfig::Assets::TextureAttributes::MagnificationLinear);
    textureData->attributes.push_back(Visconfig::Assets::TextureAttributes::MinificationLinear);
    textureData->attributes.push_back(Visconfig::Assets::TextureAttributes::GenerateMipMaps);

    return asset;
}

Visconfig::Asset createCubeShaderAsset()
{
    auto shaderData{ std::make_shared<Visconfig::Assets::ShaderAsset>() };
    Visconfig::Asset asset{ "cube_shader", Visconfig::Assets::AssetType::Shader, shaderData };
    shaderData->vertex = "assets/shaders/cube.vs.glsl";
    shaderData->fragment = "assets/shaders/cube.fs.glsl";

    return asset;
}

Visconfig::Asset createDefaultFramebufferAsset()
{
    return Visconfig::Asset{ "default_framebuffer", Visconfig::Assets::AssetType::DefaultFramebuffer,
        std::make_shared<Visconfig::Assets::DefaultFramebufferAsset>() };
}

Visconfig::Asset generateRenderTextureAsset(std::size_t index)
{
    auto textureData{ std::make_shared<Visconfig::Assets::TextureRawAsset>() };
    Visconfig::Asset asset{ "render_texture_" + std::to_string(index), Visconfig::Assets::AssetType::TextureRaw,
        textureData };
    textureData->width = screenWidth;
    textureData->height = screenHeight;
    textureData->format = Visconfig::Assets::TextureFormat::RGBA;

    return asset;
}

Visconfig::Asset generateFramebufferAsset(std::size_t index)
{
    auto bufferData{ std::make_shared<Visconfig::Assets::FramebufferAsset>() };
    Visconfig::Asset asset{ "framebuffer_" + std::to_string(index), Visconfig::Assets::AssetType::Framebuffer,
        bufferData };
    bufferData->attachments.push_back(
        Visconfig::Assets::FramebufferAttachment{ Visconfig::Assets::FramebufferType::Texture,
            Visconfig::Assets::FramebufferDestination::Color0, "render_texture_" + std::to_string(index) });

    return asset;
}

Visconfig::Entity generateCube(const ProcessedConfig& mdhConfig, const ViewInfo& view, std::size_t layerNumber,
    std::size_t viewIndex, std::size_t entityId, std::size_t parentId)
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

    std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[1].data)->asset = "cube_mesh";

    auto texture{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuseColor{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto gridScale{ std::make_shared<Visconfig::Components::Vec2ArrayMaterialAttribute>() };

    texture->asset = "cube_texture";
    texture->slot = 0;

    diffuseColor->value[0] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[0]) / 255.0f;
    diffuseColor->value[1] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[1]) / 255.0f;
    diffuseColor->value[2] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[2]) / 255.0f;
    diffuseColor->value[3] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[3]) / 255.0f;

    gridScale->value.push_back({ static_cast<float>(mdhConfig.config[layerNumber].tps.tileSize[0]),
        static_cast<float>(mdhConfig.config[layerNumber].tps.tileSize[1]) });
    gridScale->value.push_back({ static_cast<float>(mdhConfig.config[layerNumber].tps.tileSize[0]),
        static_cast<float>(mdhConfig.config[layerNumber].tps.tileSize[2]) });
    gridScale->value.push_back({ static_cast<float>(mdhConfig.config[layerNumber].tps.tileSize[1]),
        static_cast<float>(mdhConfig.config[layerNumber].tps.tileSize[2]) });

    auto material{ std::static_pointer_cast<Visconfig::Components::MaterialComponent>(entity.components[2].data) };

    material->asset = "cube_shader";
    material->attributes.insert_or_assign("gridTexture",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture, false });

    material->attributes.insert_or_assign("diffuseColor",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuseColor, false });
    material->attributes.insert_or_assign("gridScale",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec2, gridScale, true });

    auto layer{ std::static_pointer_cast<Visconfig::Components::LayerComponent>(entity.components[3].data) };
    layer->mask = 1llu << viewIndex;

    auto transform{ std::static_pointer_cast<Visconfig::Components::TransformComponent>(entity.components[4].data) };
    transform->rotation[0] = 0;
    transform->rotation[1] = 0;
    transform->rotation[2] = 0;

    transform->position[0] = 0;
    transform->position[1] = 0;
    transform->position[2] = 0;

    transform->scale[0] = view.layers[layerNumber].scale[0];
    transform->scale[1] = view.layers[layerNumber].scale[1];
    transform->scale[2] = view.layers[layerNumber].scale[2];

    if (layerNumber != 0) {
        entity.components.push_back({ Visconfig::Components::ComponentType::Parent,
            std::make_shared<Visconfig::Components::ParentComponent>() });
        entity.components.push_back({ Visconfig::Components::ComponentType::ImplicitIteration,
            std::make_shared<Visconfig::Components::ImplicitIterationComponent>() });

        std::static_pointer_cast<Visconfig::Components::ParentComponent>(entity.components[5].data)->id = parentId;

        auto iteration{ std::static_pointer_cast<Visconfig::Components::ImplicitIterationComponent>(
            entity.components[6].data) };
        iteration->order = Visconfig::Components::IterationOrder::XYZ;
        iteration->ticksPerIteration = mdhConfig.config[layerNumber].tps.tileSize[0]
            * mdhConfig.config[layerNumber].tps.tileSize[1] * mdhConfig.config[layerNumber].tps.tileSize[2];
        iteration->numIterations[0] = view.layers[layerNumber].iterationRate[0];
        iteration->numIterations[1] = view.layers[layerNumber].iterationRate[1];
        iteration->numIterations[2] = view.layers[layerNumber].iterationRate[2];

        std::array<float, 3> halfScale{
            view.layers[layerNumber].scale[0] / 2.0f,
            view.layers[layerNumber].scale[1] / 2.0f,
            view.layers[layerNumber].scale[2] / 2.0f,
        };

        transform->position[0] = -0.5f + halfScale[0];
        transform->position[1] = 0.5f - halfScale[1];
        transform->position[2] = -0.5f + halfScale[2];
    }

    return entity;
}

Visconfig::Entity generateViewCamera(std::size_t entityId, std::size_t viewIndex, const std::string& framebufferName)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back(
        { Visconfig::Components::ComponentType::Camera, std::make_shared<Visconfig::Components::CameraComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });

    auto camera{ std::static_pointer_cast<Visconfig::Components::CameraComponent>(entity.components[0].data) };
    camera->layerMask = 1ull << viewIndex;
    camera->targets.insert_or_assign("cube", framebufferName);
    camera->targets.insert_or_assign("text", framebufferName);

    auto transform{ std::static_pointer_cast<Visconfig::Components::TransformComponent>(entity.components[1].data) };
    transform->rotation[0] = 0.0f;
    transform->rotation[1] = 0.0f;
    transform->rotation[2] = 0.0f;

    transform->position[0] = 0.0f;
    transform->position[1] = 0.0f;
    transform->position[2] = 0.0f;

    transform->scale[0] = 1.0f;
    transform->scale[1] = 1.0f;
    transform->scale[2] = 1.0f;

    if (viewIndex == 0) {
        entity.components.push_back({ Visconfig::Components::ComponentType::FreeFlyCamera,
            std::make_shared<Visconfig::Components::FreeFlyCameraComponent>() });
    }

    return entity;
}

Visconfig::Entity generateCoordinatorEntity(std::size_t entityId)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back({ Visconfig::Components::ComponentType::Composition,
        std::make_shared<Visconfig::Components::CompositionComponent>() });

    return entity;
}

void extentComposition(Visconfig::World& world, std::array<float, 2> scale, std::array<float, 2> position,
    const std::string& src, const std::string& target)
{
    auto composition{ std::static_pointer_cast<Visconfig::Components::CompositionComponent>(
        world.entities[0].components[0].data) };

    composition->operations.push_back(Visconfig::Components::CompositionOperation{
        { scale[0], scale[1] }, { position[0], position[1] }, src, target });
}

void generateMainViewConfig(
    Visconfig::Config& config, const ProcessedConfig& mdhConfig, Visconfig::World& world, std::size_t& numEntities)
{
    config.assets.push_back(generateRenderTextureAsset(0));
    config.assets.push_back(generateFramebufferAsset(0));

    for (auto layer{ mdhConfig.mainView.layers.begin() }; layer != mdhConfig.mainView.layers.end(); layer++) {
        auto index{ std::distance(mdhConfig.mainView.layers.begin(), layer) };
        world.entities.push_back(generateCube(mdhConfig, mdhConfig.mainView, index, 0, numEntities, numEntities - 1));
        numEntities++;
    }

    world.entities.push_back(generateViewCamera(numEntities++, 0, "framebuffer_0"));
    extentComposition(world, { 1.0f, 1.0f }, { 0.0f, 0.0f }, "render_texture_0", "default_framebuffer");
}

void generateSubViewConfig(Visconfig::Config& config, const ProcessedConfig& mdhConfig, Visconfig::World& world,
    std::size_t& numEntities, std::size_t subview)
{
    config.assets.push_back(generateRenderTextureAsset(subview + 1));
    config.assets.push_back(generateFramebufferAsset(subview + 1));

    for (auto layer{ mdhConfig.mainView.layers.begin() }; layer != mdhConfig.mainView.layers.end(); layer++) {
        auto index{ std::distance(mdhConfig.mainView.layers.begin(), layer) };
        world.entities.push_back(
            generateCube(mdhConfig, mdhConfig.subViews[subview], index, subview + 1, numEntities, numEntities - 1));
        numEntities++;
    }

    world.entities.push_back(
        generateViewCamera(numEntities++, subview + 1, "framebuffer_" + std::to_string(subview + 1)));
    extentComposition(world, { 0.2f, 0.2f }, { 1.0f, 1.0f - (subview * 0.2f) },
        "render_texture_" + std::to_string(subview + 1), "default_framebuffer");
}

void generateWorld(Visconfig::Config& config, const ProcessedConfig& mdhConfig)
{
    Visconfig::World world{};

    std::size_t numEntities{ 0 };
    world.entities.push_back(generateCoordinatorEntity(numEntities++));
    generateMainViewConfig(config, mdhConfig, world, numEntities);

    for (auto pos{ mdhConfig.subViews.begin() }; pos != mdhConfig.subViews.end(); pos++) {
        auto index{ std::distance(mdhConfig.subViews.begin(), pos) };
        generateSubViewConfig(config, mdhConfig, world, numEntities, index);
    }

    config.worlds.push_back(world);
}

Visconfig::Config generateConfig(const ProcessedConfig& config)
{
    Visconfig::Config visconfig{};

    visconfig.options.screenHeight = screenHeight;
    visconfig.options.screenWidth = screenWidth;
    visconfig.options.screenFullscreen = screenFullscreen;

    visconfig.assets.push_back(createCubeMeshAsset());
    visconfig.assets.push_back(createCubeTextureAsset());
    visconfig.assets.push_back(createCubeShaderAsset());
    visconfig.assets.push_back(createDefaultFramebufferAsset());

    generateWorld(visconfig, config);

    return visconfig;
}