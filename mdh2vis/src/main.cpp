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

struct BufferSize {
    std::size_t x;
    std::size_t y;
    std::size_t z;
};

using BufferSizeMap = std::unordered_map<std::string, BufferSize>;

struct BufferSizes {
    BufferSizeMap inputMap;
    BufferSizeMap outputMap;
};

void printConfigInfo(const MDH2Vis::MDHConfig& config);

BufferSizes computeBufferSizes(const MDH2Vis::MDHConfig& config);

Visconfig::Config generateConfig(const MDH2Vis::MDHConfig& config);

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

    printConfigInfo(*mdhConfig);
    // computeBufferSizes(*mdhConfig);

    auto config{ generateConfig(*mdhConfig) };
    Visconfig::to_file(workingDir / "visconfig.json", config);
}

void printConfigInfo(const MDH2Vis::MDHConfig& config)
{
    std::cout << "Config info:" << std::endl;

    std::cout << "Number of layers: " << 3 << std::endl;
    std::cout << "Dimensions: " << config.tps.layer0.tileSize[0] << ", " << config.tps.layer0.tileSize[1] << ", "
              << config.tps.layer0.tileSize[2] << std::endl;
    std::cout << std::endl;
}

struct BufferBounds {
    std::size_t min;
    std::size_t max;
};

constexpr BufferBounds StartBounds{ std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::min() };

struct BufferBounds3D {
    BufferBounds x;
    BufferBounds y;
    BufferBounds z;
};

constexpr BufferBounds3D StartBounds3D{ StartBounds, StartBounds, StartBounds };

using BoundsMap = std::unordered_map<std::string, BufferBounds3D>;

BufferSizes computeBufferSizes(const MDH2Vis::MDHConfig& config)
{
    BoundsMap bounds_map{};

    for (auto& operation : MDH2Vis::OperationMap::operations()) {
        bounds_map.insert_or_assign(operation.first, StartBounds3D);
    }

    auto computeBounds{ [](BufferBounds3D& bounds, const MDH2Vis::OperationContainer& operationContainer,
                            std::size_t maxX, std::size_t maxY, std::size_t maxZ) {
        auto computeBounds{ [](BufferBounds3D& bounds, const MDH2Vis::OperationContainer& operationContainer,
                                std::size_t minX, std::size_t maxX, std::size_t maxY, std::size_t maxZ) {
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
                            computeBounds(bounds.x, operationContainer.x[i], i1, i2, i3);
                        }
                        for (std::size_t i = 0; i < operationContainer.y.size(); ++i) {
                            computeBounds(bounds.y, operationContainer.y[i], i1, i2, i3);
                        }
                        for (std::size_t i = 0; i < operationContainer.z.size(); ++i) {
                            computeBounds(bounds.z, operationContainer.z[i], i1, i2, i3);
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

        std::vector<BufferBounds3D> buffer{};
        buffer.resize(numThreads, StartBounds3D);

        std::vector<std::thread> threads{};
        threads.reserve(numThreads);

        auto xMinCmp{ [](const BufferBounds3D& lhs, const BufferBounds3D& rhs) { return lhs.x.min < rhs.x.min; } };
        auto xMaxCmp{ [](const BufferBounds3D& lhs, const BufferBounds3D& rhs) { return lhs.x.max < rhs.x.max; } };

        auto yMinCmp{ [](const BufferBounds3D& lhs, const BufferBounds3D& rhs) { return lhs.y.min < rhs.y.min; } };
        auto yMaxCmp{ [](const BufferBounds3D& lhs, const BufferBounds3D& rhs) { return lhs.y.max < rhs.y.max; } };

        auto zMinCmp{ [](const BufferBounds3D& lhs, const BufferBounds3D& rhs) { return lhs.z.min < rhs.z.min; } };
        auto zMaxCmp{ [](const BufferBounds3D& lhs, const BufferBounds3D& rhs) { return lhs.z.max < rhs.z.max; } };

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

        bounds.x.min = std::min(bounds.x.min, std::min_element(buffer.begin(), buffer.end(), xMinCmp)->x.min);
        bounds.x.max = std::max(bounds.x.max, std::max_element(buffer.begin(), buffer.end(), xMaxCmp)->x.max);

        bounds.y.min = std::min(bounds.y.min, std::min_element(buffer.begin(), buffer.end(), yMinCmp)->y.min);
        bounds.y.max = std::max(bounds.y.max, std::max_element(buffer.begin(), buffer.end(), yMaxCmp)->y.max);

        bounds.z.min = std::min(bounds.z.min, std::min_element(buffer.begin(), buffer.end(), zMinCmp)->z.min);
        bounds.z.max = std::max(bounds.z.max, std::max_element(buffer.begin(), buffer.end(), zMaxCmp)->z.max);
    } };

    auto maxX{ config.tps.layer0.tileSize[0] };
    auto maxY{ config.tps.layer0.tileSize[1] };
    auto maxZ{ config.tps.layer0.tileSize[2] };

    std::size_t viewNumber{ 0 };
    std::size_t numViews{ bounds_map.size() };

    std::cout << "Computing buffer sizes... " << 0 << " of " << numViews;

    for (auto& input : bounds_map) {
        const auto& operationContainer{ MDH2Vis::OperationMap::getOperations(input.first) };
        computeBounds(input.second, operationContainer, maxX, maxY, maxZ);
        viewNumber++;

        std::cout << "\33[2K\r" << std::flush;
        std::cout << "Computing buffer sizes... " << viewNumber << " of " << numViews;
    }
    std::cout << std::endl << std::endl;

    BufferSizes buffer_sizes{};

    auto computeSizes{ [](BufferSizeMap& buffer, const BoundsMap& bounds_map) {
        for (const auto& bufferSize : bounds_map) {
            BufferSize size{};
            size.x = bufferSize.second.x.max - bufferSize.second.x.min + 1;
            size.y = bufferSize.second.y.max - bufferSize.second.y.min + 1;
            size.z = bufferSize.second.z.max - bufferSize.second.z.min + 1;

            buffer.insert_or_assign(bufferSize.first, size);
            std::cout << bufferSize.first << ": "
                      << "[" << size.x << ", " << size.y << ", " << size.z << "]" << std::endl;
        }
    } };

    std::cout << "Buffers:" << std::endl;
    computeSizes(buffer_sizes.inputMap, bounds_map);
    std::cout << std::endl;

    return buffer_sizes;
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

Visconfig::Asset generateMainRenderTextureAsset()
{
    auto textureData{ std::make_shared<Visconfig::Assets::TextureRawAsset>() };
    Visconfig::Asset asset{ "render_texture_main", Visconfig::Assets::AssetType::TextureRaw, textureData };
    textureData->width = 600;
    textureData->height = 400;
    textureData->format = Visconfig::Assets::TextureFormat::RGB;

    return asset;
}

Visconfig::Asset generateMainFramebufferAsset()
{
    auto bufferData{ std::make_shared<Visconfig::Assets::FramebufferAsset>() };
    Visconfig::Asset asset{ "framebuffer_main", Visconfig::Assets::AssetType::Framebuffer, bufferData };
    bufferData->attachments.push_back(
        Visconfig::Assets::FramebufferAttachment{ Visconfig::Assets::FramebufferType::Texture,
            Visconfig::Assets::FramebufferDestination::Color0, "render_texture_main" });

    return asset;
}

Visconfig::Entity generateMainViewCube(const MDH2Vis::TPS::Layer& cubeLayer, std::size_t entityId)
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

    auto material{ std::static_pointer_cast<Visconfig::Components::MaterialComponent>(entity.components[2].data) };
    texture->asset = "cube_texture";
    texture->slot = 0;

    material->asset = "cube_shader";
    material->attributes.insert_or_assign("texture",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture, false });

    auto layer{ std::static_pointer_cast<Visconfig::Components::LayerComponent>(entity.components[3].data) };
    layer->mask = 1;

    auto transform{ std::static_pointer_cast<Visconfig::Components::TransformComponent>(entity.components[4].data) };
    transform->rotation[0] = 0;
    transform->rotation[1] = 0;
    transform->rotation[2] = 0;

    transform->position[0] = 0;
    transform->position[1] = 0;
    transform->position[2] = 0;

    transform->scale[0] = cubeLayer.tileSize[0];
    transform->scale[1] = cubeLayer.tileSize[1];
    transform->scale[2] = cubeLayer.tileSize[2];

    return entity;
}

Visconfig::Entity generateMainViewCube(const MDH2Vis::TPS::Layer& cubeLayer, const MDH2Vis::TPS::Layer& parentLayer,
    std::size_t entityId, std::size_t parentId)
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
    entity.components.push_back({ Visconfig::Components::ComponentType::ImplicitIteration,
        std::make_shared<Visconfig::Components::ImplicitIterationComponent>() });

    std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[1].data)->asset = "cube_mesh";
    auto texture{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };

    auto material{ std::static_pointer_cast<Visconfig::Components::MaterialComponent>(entity.components[2].data) };
    texture->asset = "cube_texture";
    texture->slot = 0;

    material->asset = "cube_shader";
    material->attributes.insert_or_assign("texture",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture, false });

    auto layer{ std::static_pointer_cast<Visconfig::Components::LayerComponent>(entity.components[3].data) };
    layer->mask = 1;

    std::array<float, 3> scale{
        static_cast<float>(cubeLayer.tileSize[0]) / static_cast<float>(parentLayer.tileSize[0]),
        static_cast<float>(cubeLayer.tileSize[1]) / static_cast<float>(parentLayer.tileSize[1]),
        static_cast<float>(cubeLayer.tileSize[2]) / static_cast<float>(parentLayer.tileSize[2]),
    };

    std::array<float, 3> halfScale{
        scale[0] / 2.0f,
        scale[1] / 2.0f,
        scale[2] / 2.0f,
    };

    auto transform{ std::static_pointer_cast<Visconfig::Components::TransformComponent>(entity.components[4].data) };
    transform->rotation[0] = 0;
    transform->rotation[1] = 0;
    transform->rotation[2] = 0;

    transform->position[0] = -0.5f + halfScale[0];
    transform->position[1] = 0.5f - halfScale[1];
    transform->position[2] = -0.5f + halfScale[2];

    transform->scale[0] = scale[0];
    transform->scale[1] = scale[1];
    transform->scale[2] = scale[2];

    std::static_pointer_cast<Visconfig::Components::ParentComponent>(entity.components[5].data)->id = parentId;

    auto iteration{ std::static_pointer_cast<Visconfig::Components::ImplicitIterationComponent>(
        entity.components[6].data) };
    iteration->order = Visconfig::Components::IterationOrder::XYZ;
    iteration->ticksPerIteration = cubeLayer.tileSize[0] * cubeLayer.tileSize[1] * cubeLayer.tileSize[2];
    iteration->numIterations[0] = (parentLayer.tileSize[0] / cubeLayer.tileSize[0]) - 1;
    iteration->numIterations[1] = (parentLayer.tileSize[1] / cubeLayer.tileSize[1]) - 1;
    iteration->numIterations[2] = (parentLayer.tileSize[2] / cubeLayer.tileSize[2]) - 1;

    return entity;
}

Visconfig::Entity generateMainViewCamera(std::size_t entityId)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back(
        { Visconfig::Components::ComponentType::Camera, std::make_shared<Visconfig::Components::CameraComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::FreeFlyCamera,
        std::make_shared<Visconfig::Components::FreeFlyCameraComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });

    auto camera{ std::static_pointer_cast<Visconfig::Components::CameraComponent>(entity.components[0].data) };
    camera->layerMask.set();
    camera->targets.insert_or_assign("cube", "framebuffer_main");
    camera->targets.insert_or_assign("text", "framebuffer_main");

    auto transform{ std::static_pointer_cast<Visconfig::Components::TransformComponent>(entity.components[2].data) };
    transform->rotation[0] = 0.0f;
    transform->rotation[1] = 0.0f;
    transform->rotation[2] = 0.0f;

    transform->position[0] = 0.0f;
    transform->position[1] = 0.0f;
    transform->position[2] = 0.0f;

    transform->scale[0] = 1.0f;
    transform->scale[1] = 1.0f;
    transform->scale[2] = 1.0f;

    return entity;
}

Visconfig::Entity generateCompositionEntity(std::size_t entityId)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back({ Visconfig::Components::ComponentType::Composition,
        std::make_shared<Visconfig::Components::CompositionComponent>() });

    auto composition{ std::static_pointer_cast<Visconfig::Components::CompositionComponent>(
        entity.components[0].data) };
    composition->operations.push_back(Visconfig::Components::CompositionOperation{
        { 1.0f, 1.0f }, { 0.0f, 0.0f }, "render_texture_main", "default_framebuffer" });

    return entity;
}

void generateMainViewConfig(Visconfig::Config& config, const MDH2Vis::MDHConfig& mdhConfig, std::size_t& numEntities)
{
    config.assets.push_back(generateMainRenderTextureAsset());
    config.assets.push_back(generateMainFramebufferAsset());

    Visconfig::World world{};
    world.entities.push_back(generateMainViewCube(mdhConfig.tps.layer0, numEntities++));
    world.entities.push_back(
        generateMainViewCube(mdhConfig.tps.layer1, mdhConfig.tps.layer0, numEntities, numEntities - 1));
    numEntities++;
    world.entities.push_back(
        generateMainViewCube(mdhConfig.tps.layer2, mdhConfig.tps.layer1, numEntities, numEntities - 1));
    numEntities++;

    world.entities.push_back(generateMainViewCamera(numEntities++));
    world.entities.push_back(generateCompositionEntity(numEntities++));

    config.worlds.push_back(std::move(world));
}

Visconfig::Config generateConfig(const MDH2Vis::MDHConfig& config)
{
    Visconfig::Config visconfig{};

    visconfig.options.screenHeight = 400;
    visconfig.options.screenWidth = 600;
    visconfig.options.screenFullscreen = false;

    visconfig.assets.push_back(createCubeMeshAsset());
    visconfig.assets.push_back(createCubeTextureAsset());
    visconfig.assets.push_back(createCubeShaderAsset());
    visconfig.assets.push_back(createDefaultFramebufferAsset());

    std::size_t numEntities{ 0 };
    generateMainViewConfig(visconfig, config, numEntities);

    return visconfig;
}