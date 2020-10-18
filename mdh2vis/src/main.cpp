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

constexpr std::string_view UsageStr{ "Usage: mdh2vis --model model-path --tps tps-path [--out output-dir]" };

constexpr std::size_t screenWidth = 1200;
constexpr std::size_t screenHeight = 900;
constexpr std::size_t screenMSAASamples = 16;
constexpr bool screenFullscreen = false;

constexpr std::size_t renderResolutionMultiplier{ 2 };
constexpr std::size_t renderResolutionWidth{ screenWidth * renderResolutionMultiplier };
constexpr std::size_t renderResolutionHeight{ screenHeight * renderResolutionMultiplier };

constexpr float mainViewTextureBorderRelativeWidth{ 0.02f };
constexpr float threadViewTextureBorderRelativeWidth{ 0.05f };
constexpr float subViewTextureBorderRelativeWidth{ 0.4f };

constexpr float cameraFOV{ 70.0f };
constexpr float cameraAspect{ static_cast<float>(screenWidth) / static_cast<float>(screenHeight) };
constexpr float cameraAspectSmall{ cameraAspect / 2 };
constexpr float cameraNear{ 0.3f };
constexpr float cameraFar{ 10000.0f };

constexpr float minTransparency{ 0.1f };
constexpr float maxTransparency{ 0.95f };

constexpr auto assetsDirectory{ "external_assets" };
constexpr auto assetsTextureDirectory{ "external_assets/textures" };

constexpr auto cubeMeshAsset{ "cube_mesh" };
constexpr auto cubeTextureAsset{ "cube_texture" };
constexpr auto outputCubeTextureAsset{ "output_cube_texture" };
constexpr auto cubeShaderAsset{ "cube_shader" };
constexpr auto defaultFramebufferAsset{ "default_framebuffer" };
constexpr auto viewCompositionShaderAsset{ "view_composition_shader" };

constexpr auto cubeShaderVertexPath{ "assets/shaders/cube.vs.glsl" };
constexpr auto cubeShaderFragmentPath{ "assets/shaders/cube.fs.glsl" };

constexpr auto viewCompositionShaderVertexPath{ "assets/shaders/compositing.vs.glsl" };
constexpr auto viewCompositionShaderFragmentPath{ "assets/shaders/compositing.fs.glsl" };

struct SequentialLayer {
    MDH2Vis::Model::Layer model;
    MDH2Vis::TPS::Layer tps;
};

struct MainLayerInfo {
    std::array<float, 3> scale;
    std::array<float, 3> absoluteScale;
    std::array<std::size_t, 3> numIterations;
};

struct ThreadLayerInfo {
    std::array<float, 3> scale;
    std::array<float, 3> absoluteScale;
    std::array<std::size_t, 3> numThreads;
    std::array<std::size_t, 3> numIterations;
};

struct SubViewLayerInfo {
    std::size_t iterationRate;
    std::array<float, 3> scale;
    std::array<float, 3> absoluteScale;
    std::vector<std::array<float, 3>> positions;
};

struct OutputLayerInfo {
    std::size_t iterationRate;
    std::array<float, 3> size;
    std::array<float, 3> absoluteSize;
    std::array<std::size_t, 3> subdivisions;
    std::vector<std::size_t> iterationRates;
    std::array<std::size_t, 3> numIterations;
    std::vector<std::array<float, 3>> positions;
    std::vector<std::array<float, 3>> gridPositions;
    std::vector<std::array<float, 3>> absolutePositions;
};

struct MainViewInfo {
    std::vector<MainLayerInfo> layers;
    std::vector<ThreadLayerInfo> threads;
};

struct SubViewInfo {
    std::string name;
    std::vector<SubViewLayerInfo> layers;
};

struct OutputViewInfo {
    std::array<float, 3> size;
    std::vector<OutputLayerInfo> layers;
};

struct ProcessedConfig {
    MainViewInfo mainView;
    OutputViewInfo outputView;
    std::vector<SubViewInfo> subViews;
    std::vector<SequentialLayer> config;
};

void printConfigInfo(const ProcessedConfig& config);

ProcessedConfig processConfig(const MDH2Vis::MDHConfig& config);

Visconfig::Config generateConfig(const ProcessedConfig& config, const std::filesystem::path& workingDir);

void generateAssetsDirectory(const std::filesystem::path& workingDir);

int main(int argc, char* argv[])
{
    if (argc != 5 && argc != 7) {
        std::cerr << UsageStr << std::endl;
        return 1;
    }

    auto workingDir{ std::filesystem::current_path() };

    std::filesystem::path modelPath{};
    std::filesystem::path tpsPath{};

    bool modelSet{ false };
    bool tpsSet{ false };

    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], "--model") == 0) {
            modelPath = argv[i + 1];
            modelSet = true;
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

    if (!modelSet || !tpsSet) {
        std::cerr << UsageStr << std::endl;
        return 1;
    }

    if (!std::filesystem::exists(workingDir)) {
        std::cerr << "Could not find " << workingDir << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(modelPath)) {
        std::cerr << "Could not find " << modelPath << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(tpsPath)) {
        std::cerr << "Could not find " << tpsPath << std::endl;
        return 1;
    }

    std::cout << "Paths:" << std::endl;
    std::cout << "\tOutput:" << workingDir << std::endl;
    std::cout << "\tModel:" << modelPath << std::endl;
    std::cout << "\tTPS:" << tpsPath << std::endl;

    auto mdhConfig{ MDH2Vis::loadFromFiles(modelPath, tpsPath) };
    if (!mdhConfig) {
        std::cerr << "Could not load configs" << std::endl;
        return 1;
    }

    auto config{ processConfig(*mdhConfig) };
    printConfigInfo(config);

    generateAssetsDirectory(workingDir);
    auto visConfig{ generateConfig(config, workingDir) };
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

template <class... Fs> struct Overload : Fs... {
    template <class... Ts>
    Overload(Ts&&... ts)
        : Fs{ std::forward<Ts>(ts) }...
    {
    }

    using Fs::operator()...;
};

template <class... Ts> Overload(Ts&&...) -> Overload<std::remove_reference_t<Ts>...>;

void processMainView(const MDH2Vis::MDHConfig& mdhConfig, ProcessedConfig& config)
{
    auto adjustLayer{ Overload(
        [](auto& current, const auto& previous) {
            current.scale = {
                current.absoluteScale[0] / previous.absoluteScale[0],
                current.absoluteScale[1] / previous.absoluteScale[1],
                current.absoluteScale[2] / previous.absoluteScale[2],
            };
        },
        [](MainLayerInfo& current, const MainLayerInfo& previous) {
            current.scale = {
                current.absoluteScale[0] / previous.absoluteScale[0],
                current.absoluteScale[1] / previous.absoluteScale[1],
                current.absoluteScale[2] / previous.absoluteScale[2],
            };

            current.numIterations = {
                static_cast<std::size_t>(previous.absoluteScale[0] / current.absoluteScale[0]) - 1,
                static_cast<std::size_t>(previous.absoluteScale[1] / current.absoluteScale[1]) - 1,
                static_cast<std::size_t>(previous.absoluteScale[2] / current.absoluteScale[2]) - 1,
            };

            if (current.absoluteScale[0] == previous.absoluteScale[0]) {
                current.numIterations[0] = 0;
            }
            if (current.absoluteScale[1] == previous.absoluteScale[1]) {
                current.numIterations[1] = 0;
            }
            if (current.absoluteScale[2] == previous.absoluteScale[2]) {
                current.numIterations[2] = 0;
            }
        },
        [](ThreadLayerInfo& current, const MainLayerInfo& previous) {
            current.scale = {
                current.absoluteScale[0] / previous.absoluteScale[0],
                current.absoluteScale[1] / previous.absoluteScale[1],
                current.absoluteScale[2] / previous.absoluteScale[2],
            };

            current.numIterations = {
                static_cast<std::size_t>(previous.absoluteScale[0] / current.absoluteScale[0]) - 1,
                static_cast<std::size_t>(previous.absoluteScale[1] / current.absoluteScale[1]) - 1,
                static_cast<std::size_t>(previous.absoluteScale[2] / current.absoluteScale[2]) - 1,
            };

            if (current.absoluteScale[0] == previous.absoluteScale[0]) {
                current.numIterations[0] = 0;
            }
            if (current.absoluteScale[1] == previous.absoluteScale[1]) {
                current.numIterations[1] = 0;
            }
            if (current.absoluteScale[2] == previous.absoluteScale[2]) {
                current.numIterations[2] = 0;
            }
        }) };

    auto adjustLayerVector{ Overload(
        [&](std::vector<MainLayerInfo>& layers) {
            for (auto pos{ layers.begin() + 1 }; pos != layers.end(); pos++) {
                adjustLayer.operator()(*pos, *(pos - 1));
            }
        },
        [&](std::vector<ThreadLayerInfo>& layers) {
            for (auto pos{ layers.begin() + 1 }; pos != layers.end(); pos++) {
                adjustLayer.operator()(*pos, *(pos - 1));
            }
        }) };

    std::array<float, 3> threadScale{ 1.0f, 1.0f, 1.0f };

    for (auto& tps : mdhConfig.tps) {
        threadScale[0] *= tps.second.numThreads[0];
        threadScale[1] *= tps.second.numThreads[1];
        threadScale[2] *= tps.second.numThreads[2];

        auto absoluteScale{ threadScale };
        absoluteScale[0] *= tps.second.tileSize[0];
        absoluteScale[1] *= tps.second.tileSize[1];
        absoluteScale[2] *= tps.second.tileSize[2];

        config.mainView.layers.push_back(MainLayerInfo{ absoluteScale, absoluteScale, { 0, 0, 0 } });
    }

    for (std::size_t i{ 0 }; i < config.config.size(); i++) {
        std::array<std::size_t, 3> numThreads{
            config.config[i].tps.numThreads[0],
            config.config[i].tps.numThreads[1],
            config.config[i].tps.numThreads[2],
        };
        std::array<float, 3> absoluteScale{ 1, 1, 1 };
        for (std::size_t j{ i + 1 }; j < config.config.size(); ++j) {
            absoluteScale[0] *= config.config[j].tps.numThreads[0];
            absoluteScale[1] *= config.config[j].tps.numThreads[1];
            absoluteScale[2] *= config.config[j].tps.numThreads[2];
        }

        config.mainView.threads.push_back(ThreadLayerInfo{ absoluteScale, absoluteScale, numThreads, { 0, 0, 0 } });
    }

    adjustLayerVector.operator()(config.mainView.layers);
    adjustLayer.operator()(config.mainView.threads.front(), config.mainView.layers.back());
    adjustLayerVector.operator()(config.mainView.threads);
}

void processSubView(ProcessedConfig& config, const std::string& name, const MDH2Vis::OperationContainer& operation)
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

    auto computeIterationPositions{ [](const MDH2Vis::OperationContainer& operationContainer, std::uint32_t stepX,
                                        std::uint32_t stepY, std::uint32_t stepZ, std::uint32_t maxX,
                                        std::uint32_t maxY, std::uint32_t maxZ, float sizeX, float sizeY,
                                        float sizeZ) -> std::vector<std::array<float, 3>> {
        auto computeMin{ [](float& min, const MDH2Vis::Operation& operation, std::size_t i1, std::size_t i2,
                             std::size_t i3) {
            auto value{ static_cast<float>(operation(i1, i2, i3)) };

            if (min > value) {
                min = value;
            }
        } };

        std::vector<std::array<float, 3>> positions{};

        for (std::size_t i3 = 0; i3 < maxZ; i3 += stepZ) {
            for (std::size_t i2 = 0; i2 < maxY; i2 += stepY) {
                for (std::size_t i1 = 0; i1 < maxX; i1 += stepX) {
                    std::array<float, 3> position{};
                    position.fill(std::numeric_limits<float>::max());

                    for (std::size_t i = 0; i < operationContainer.x.size(); ++i) {
                        computeMin(position[0], operationContainer.x[i], i1, i2, i3);
                    }
                    for (std::size_t i = 0; i < operationContainer.y.size(); ++i) {
                        computeMin(position[1], operationContainer.y[i], i1, i2, i3);
                    }
                    for (std::size_t i = 0; i < operationContainer.z.size(); ++i) {
                        computeMin(position[2], operationContainer.z[i], i1, i2, i3);
                    }

                    position[0] /= sizeX;
                    position[1] /= sizeY;
                    position[2] /= sizeZ;

                    positions.push_back(position);
                }
            }
        }

        return positions;
    } };

    SubViewInfo view{};
    view.name = name;

    for (const auto& layer : config.mainView.layers) {
        auto scale{ computeBounds(operation, layer.absoluteScale[0], layer.absoluteScale[1], layer.absoluteScale[2]) };
        view.layers.push_back(SubViewLayerInfo{ 0, scale, scale, {} });
    }

    std::size_t threadLayerSize = config.mainView.threads[0].absoluteScale[0]
        * config.mainView.threads[0].absoluteScale[1] * config.mainView.threads[0].absoluteScale[2];

    for (auto pos{ view.layers.begin() }; pos != view.layers.end(); pos++) {

        auto index{ std::distance(view.layers.begin(), pos) };

        const auto& currentLayer{ *pos };
        const auto& mainViewLayer{ config.mainView.layers[index] };

        pos->iterationRate
            = mainViewLayer.absoluteScale[0] * mainViewLayer.absoluteScale[1] * mainViewLayer.absoluteScale[2];
        pos->iterationRate /= threadLayerSize;

        if (pos == view.layers.begin()) {
            pos->positions = { { 0.0f, 0.0f, 0.0f } };
        } else {
            const auto& previousLayer{ *(pos - 1) };
            const auto& previousMainViewLayer{ config.mainView.layers[index - 1] };

            pos->scale = {
                currentLayer.absoluteScale[0] / previousLayer.absoluteScale[0],
                currentLayer.absoluteScale[1] / previousLayer.absoluteScale[1],
                currentLayer.absoluteScale[2] / previousLayer.absoluteScale[2],
            };

            pos->positions = computeIterationPositions(operation, mainViewLayer.absoluteScale[0],
                mainViewLayer.absoluteScale[1], mainViewLayer.absoluteScale[2], previousMainViewLayer.absoluteScale[0],
                previousMainViewLayer.absoluteScale[1], previousMainViewLayer.absoluteScale[2],
                currentLayer.absoluteScale[0], currentLayer.absoluteScale[1], currentLayer.absoluteScale[2]);
        }
    }

    config.subViews.push_back(view);
}

void processOutputView(const MDH2Vis::MDHConfig&, ProcessedConfig& config)
{
    enum class DimensionType { CC, CB };
    auto combineOperations{ MDH2Vis::CombineOperations::operations() };

    std::array<DimensionType, 3> dimensionTypes{};

    dimensionTypes[0] = combineOperations.size() >= 1 && combineOperations[0].compare("CB") == 0 ? DimensionType::CB
                                                                                                 : DimensionType::CC;
    dimensionTypes[1] = combineOperations.size() >= 2 && combineOperations[1].compare("CB") == 0 ? DimensionType::CB
                                                                                                 : DimensionType::CC;
    dimensionTypes[2] = combineOperations.size() >= 3 && combineOperations[2].compare("CB") == 0 ? DimensionType::CB
                                                                                                 : DimensionType::CC;
    std::array<float, 3> threadDimensions{ config.mainView.threads[0].absoluteScale };

    auto generateLayer{ [](std::array<DimensionType, 3> dimensionTypes, std::array<float, 3> threadDimensions,
                            std::array<float, 3> layerDimensions) -> OutputLayerInfo {
        OutputLayerInfo layerInfo{};

        std::array<std::size_t, 3> subdivisions{ 0, 0, 0 };
        std::array<float, 3> outputDimensions{ layerDimensions };

        if (dimensionTypes[0] == DimensionType::CB) {
            subdivisions[0] = 1;
            outputDimensions[0] = threadDimensions[0];
        } else {
            subdivisions[0] = static_cast<std::size_t>(outputDimensions[0] / threadDimensions[0]);
        }
        if (dimensionTypes[1] == DimensionType::CB) {
            subdivisions[1] = 1;
            outputDimensions[1] = threadDimensions[2];
        } else {
            subdivisions[1] = static_cast<std::size_t>(outputDimensions[1] / threadDimensions[1]);
        }
        if (dimensionTypes[2] == DimensionType::CB) {
            subdivisions[2] = 1;
            outputDimensions[2] = threadDimensions[2];
        } else {
            subdivisions[2] = static_cast<std::size_t>(outputDimensions[2] / threadDimensions[2]);
        }

        layerInfo.subdivisions = subdivisions;
        layerInfo.absoluteSize = outputDimensions;

        return layerInfo;
    } };

    auto computePositions{ Overload{ [](OutputLayerInfo& layer, std::array<float, 3> threadDimensions,
                                         std::array<std::size_t, 3> iterations, std::size_t iterationRate) {
                                        iterations[0]++;
                                        iterations[1]++;
                                        iterations[2]++;

                                        for (std::size_t z{ 0 }; z < iterations[2]; z++) {
                                            for (std::size_t y{ 0 }; y < iterations[1]; y++) {
                                                for (std::size_t x{ 0 }; x < iterations[0]; x++) {
                                                    if (x >= layer.subdivisions[0] || y >= layer.subdivisions[1]
                                                        || z >= layer.subdivisions[2]) {
                                                        layer.iterationRates.back() += iterationRate;
                                                    } else {
                                                        std::array<float, 3> position{
                                                            x * threadDimensions[0],
                                                            -1.0f * y * threadDimensions[1],
                                                            z * threadDimensions[2],
                                                        };

                                                        layer.absolutePositions.push_back(position);
                                                        layer.iterationRates.push_back(iterationRate);
                                                    }
                                                }
                                            }
                                        }

                                        layer.iterationRate = 0;
                                        for (auto rate : layer.iterationRates) {
                                            layer.iterationRate += rate;
                                        }
                                    },
        [](OutputLayerInfo& layer, const OutputLayerInfo& previousLayer, std::array<std::size_t, 3> iterations) {
            iterations[0]++;
            iterations[1]++;
            iterations[2]++;

            for (std::size_t z{ 0 }; z < iterations[2]; z++) {
                for (std::size_t y{ 0 }; y < iterations[1]; y++) {
                    for (std::size_t x{ 0 }; x < iterations[0]; x++) {
                        std::array<float, 3> offset{
                            x * previousLayer.absoluteSize[0],
                            y * previousLayer.absoluteSize[1],
                            z * previousLayer.absoluteSize[2],
                        };
                        if (offset[0] >= layer.absoluteSize[0] || offset[1] >= layer.absoluteSize[1]
                            || offset[2] >= layer.absoluteSize[2]) {
                            layer.iterationRates.back() += previousLayer.iterationRate;
                            continue;
                        } else {

                            offset[1] *= -1.0f;

                            for (std::size_t i{ 0 }; i < previousLayer.absolutePositions.size(); i++) {
                                std::array<float, 3> position{
                                    previousLayer.absolutePositions[i][0] + offset[0],
                                    previousLayer.absolutePositions[i][1] + offset[1],
                                    previousLayer.absolutePositions[i][2] + offset[2],
                                };

                                layer.absolutePositions.push_back(position);
                                layer.iterationRates.push_back(previousLayer.iterationRates[i]);
                            }
                        }
                    }
                }
            }

            layer.iterationRate = 0;
            for (auto rate : layer.iterationRates) {
                layer.iterationRate += rate;
            }
        } } };

    auto computeRelativePositionAndSize{ Overload{ [](OutputLayerInfo& layer, std::array<float, 3> threadSize) {
                                                      layer.size = { 1.0f, 1.0f, 1.0f };
                                                      layer.numIterations = { 0, 0, 0 };
                                                      for (auto position : layer.absolutePositions) {
                                                          position[0] /= layer.absoluteSize[0];
                                                          position[1] /= layer.absoluteSize[1];
                                                          position[2] /= layer.absoluteSize[2];
                                                          layer.positions.push_back(position);
                                                      }
                                                      for (auto position : layer.absolutePositions) {
                                                          position[0] /= threadSize[0];
                                                          position[1] /= threadSize[1];
                                                          position[2] /= threadSize[2];
                                                          layer.gridPositions.push_back(position);
                                                      }
                                                  },
        [](OutputLayerInfo& layer, const OutputLayerInfo& previousLayer, std::array<float, 3> threadSize) {
            layer.size = {
                layer.absoluteSize[0] / previousLayer.absoluteSize[0],
                layer.absoluteSize[1] / previousLayer.absoluteSize[1],
                layer.absoluteSize[2] / previousLayer.absoluteSize[2],
            };
            layer.numIterations = {
                static_cast<std::size_t>(previousLayer.absoluteSize[0] / layer.absoluteSize[0]) - 1,
                static_cast<std::size_t>(previousLayer.absoluteSize[1] / layer.absoluteSize[1]) - 1,
                static_cast<std::size_t>(previousLayer.absoluteSize[2] / layer.absoluteSize[2]) - 1,
            };
            for (auto position : layer.absolutePositions) {
                position[0] /= layer.absoluteSize[0];
                position[1] /= layer.absoluteSize[1];
                position[2] /= layer.absoluteSize[2];
                layer.positions.push_back(position);
            }
            for (auto position : layer.absolutePositions) {
                position[0] /= threadSize[0];
                position[1] /= threadSize[1];
                position[2] /= threadSize[2];
                layer.gridPositions.push_back(position);
            }
        } } };

    for (const auto& layer : config.mainView.layers) {
        config.outputView.layers.push_back(generateLayer(dimensionTypes, threadDimensions, layer.absoluteScale));
    }

    for (auto pos{ config.outputView.layers.rbegin() }; pos != config.outputView.layers.rend(); pos++) {
        if (pos == config.outputView.layers.rbegin()) {
            const auto& threadLayer{ config.mainView.threads.front() };
            /*
            auto iterationRate{ static_cast<std::size_t>(threadDimensions[0])
                * static_cast<std::size_t>(threadDimensions[1]) * static_cast<std::size_t>(threadDimensions[2]) };
            */
            computePositions.operator()(*pos, threadDimensions, threadLayer.numIterations, 1);
        } else {
            auto& layer{ *pos };
            const auto& previousLayer{ *(pos - 1) };
            auto previousIndex{ config.outputView.layers.size()
                - std::distance(config.outputView.layers.rbegin(), pos) };

            auto& previousMainLayer{ config.mainView.layers[previousIndex] };
            computePositions.operator()(layer, previousLayer, previousMainLayer.numIterations);
        }
    }

    for (auto pos{ config.outputView.layers.begin() }; pos != config.outputView.layers.end(); pos++) {
        if (pos == config.outputView.layers.begin()) {
            computeRelativePositionAndSize.operator()(*pos, threadDimensions);
        } else {
            computeRelativePositionAndSize.operator()(*pos, *(pos - 1), threadDimensions);
        }
    }

    config.outputView.size = config.outputView.layers.front().absoluteSize;
}

ProcessedConfig processConfig(const MDH2Vis::MDHConfig& mdhConfig)
{
    ProcessedConfig config{};

    for (auto& tps : mdhConfig.tps) {
        config.config.push_back(SequentialLayer{ mdhConfig.model.at(tps.first), tps.second });
    }

    processMainView(mdhConfig, config);
    processOutputView(mdhConfig, config);

    for (auto& operation : MDH2Vis::OperationMap::operations()) {
        processSubView(config, operation.first, operation.second);
    }

    return config;
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

void generateAssetsDirectory(const std::filesystem::path& workingDir)
{
    auto assetPath{ workingDir / assetsDirectory };
    auto assetTexturePath{ workingDir / assetsTextureDirectory };

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
    Visconfig::Asset asset{ cubeMeshAsset, Visconfig::Assets::AssetType::Mesh, meshData };

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
    return createTextureAsset(cubeTextureAsset, "assets/textures/cube.png",
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps });
}

Visconfig::Asset createOutputCubeTextureAsset()
{
    return createTextureAsset(outputCubeTextureAsset, "assets/textures/output_cube.png",
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
    return Visconfig::Asset{ defaultFramebufferAsset, Visconfig::Assets::AssetType::DefaultFramebuffer,
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

Visconfig::Entity generateMainViewCube(const ProcessedConfig& mdhConfig, const MainViewInfo& view, std::size_t entityId,
    std::size_t parentId, std::size_t layerNumber, const std::string& frontTexture, const std::string& sideTexture,
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

    mesh.asset = cubeMeshAsset;

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
        minTransparency, maxTransparency, 0ull, view.layers.size() + view.threads.size() - 1, layerNumber);

    material.asset = cubeShaderAsset;
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
        iteration.numIterations[0] = view.layers[layerNumber].numIterations[0];
        iteration.numIterations[1] = view.layers[layerNumber].numIterations[1];
        iteration.numIterations[2] = view.layers[layerNumber].numIterations[2];

        iteration.ticksPerIteration = view.layers[layerNumber].absoluteScale[0]
            * view.layers[layerNumber].absoluteScale[1] * view.layers[layerNumber].absoluteScale[2];
        iteration.ticksPerIteration
            /= view.threads[0].absoluteScale[0] * view.threads[0].absoluteScale[1] * view.threads[0].absoluteScale[2];
    }

    return entity;
}

Visconfig::Entity generateMainViewThreadCube(const ProcessedConfig& mdhConfig, const MainViewInfo& view,
    std::size_t entityId, std::size_t parentId, std::size_t layerNumber, std::array<std::size_t, 3> blockNumber,
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

    mesh.asset = cubeMeshAsset;

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
    diffuseColor->value[3] = interpolateLinear<std::size_t>(minTransparency, maxTransparency, 0ull,
        view.layers.size() + view.threads.size() - 1, view.layers.size() + layerNumber);

    material.asset = cubeShaderAsset;
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
        iteration.numIterations[0] = view.threads[layerNumber].numIterations[0];
        iteration.numIterations[1] = view.threads[layerNumber].numIterations[1];
        iteration.numIterations[2] = view.threads[layerNumber].numIterations[2];

        /*
        iteration.ticksPerIteration = view.threads[layerNumber].absoluteScale[0]
            * view.threads[layerNumber].absoluteScale[1] * view.threads[layerNumber].absoluteScale[2];
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

    material.asset = cubeShaderAsset;
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

Visconfig::Entity generateOutputViewCube(Visconfig::World& world, const ProcessedConfig& mdhConfig,
    const OutputViewInfo& view, std::size_t& entityId, std::size_t parentId, std::size_t viewNumber,
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
        interpolateLinear<std::size_t>(minTransparency, maxTransparency, 0ull,
            mdhConfig.mainView.layers.size() + mdhConfig.mainView.threads.size() - 1, layerNumber),
    };

    auto& threadLayer{ mdhConfig.mainView.threads.front() };
    std::array<float, 3> childScale{
        threadLayer.absoluteScale[0] / mdhConfig.outputView.layers[layerNumber].absoluteSize[0],
        threadLayer.absoluteScale[1] / mdhConfig.outputView.layers[layerNumber].absoluteSize[1],
        threadLayer.absoluteScale[2] / mdhConfig.outputView.layers[layerNumber].absoluteSize[2],
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
            std::abs(view.layers[layerNumber].absoluteSize[0] / mdhConfig.mainView.threads[0].absoluteScale[0])),
        static_cast<std::size_t>(
            std::abs(view.layers[layerNumber].absoluteSize[1] / mdhConfig.mainView.threads[0].absoluteScale[1])),
        static_cast<std::size_t>(
            std::abs(view.layers[layerNumber].absoluteSize[2] / mdhConfig.mainView.threads[0].absoluteScale[2])),
    };
    childMeshIteration.positions = view.layers[layerNumber].gridPositions;
    childMeshIteration.ticksPerIteration = view.layers[layerNumber].iterationRates;

    world.entities.push_back(child);

    parent.id = parentId;

    if (layerNumber > 0) {
        auto& iteration{ *std::static_pointer_cast<Visconfig::Components::ImplicitIterationComponent>(
            entity.components[iterationIndex].data) };

        iteration.order = Visconfig::Components::IterationOrder::XYZ;
        iteration.numIterations[0] = view.layers[layerNumber].numIterations[0];
        iteration.numIterations[1] = view.layers[layerNumber].numIterations[1];
        iteration.numIterations[2] = view.layers[layerNumber].numIterations[2];
        iteration.ticksPerIteration = view.layers[layerNumber].iterationRate;
    }

    return entity;
}

Visconfig::Entity generateSubViewCube(const ProcessedConfig& mdhConfig, const SubViewInfo& view, std::size_t entityId,
    std::size_t parentId, std::size_t viewNumber, std::size_t layerNumber, const std::string& frontTexture,
    const std::string& sideTexture, const std::string& topTexture)
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

    mesh.asset = cubeMeshAsset;

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
        = interpolateLinear<std::size_t>(minTransparency, maxTransparency, 0ull, view.layers.size() - 1, layerNumber);
    diffuseColor->value[3] = interpolateLinear<std::size_t>(minTransparency, maxTransparency, 0ull,
        mdhConfig.mainView.layers.size() + mdhConfig.mainView.threads.size() - 1, layerNumber);

    material.asset = cubeShaderAsset;
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
    iteration.ticksPerIteration = view.layers[layerNumber].iterationRate;

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

void generateMainViewConfig(Visconfig::Config& config, const ProcessedConfig& mdhConfig, Visconfig::World& world,
    std::size_t& numEntities, const std::filesystem::path& workingDir)
{
    constexpr auto renderTextureName{ "render_texture_0" };
    constexpr auto depthBufferName{ "renderbuffer_depth_0" };
    constexpr auto framebufferName{ "framebuffer_0" };

    config.assets.push_back(generateRenderTextureAsset(
        renderTextureName, renderResolutionWidth / 2, renderResolutionHeight, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferName, renderResolutionWidth / 2,
        renderResolutionHeight, 0, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferName, 0, 0, renderResolutionWidth / 2, renderResolutionHeight,
            { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
                  renderTextureName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferName } }));

    constexpr auto renderTextureMultisampleName{ "render_texture_0_multisample" };
    constexpr auto depthBufferMultisampleName{ "renderbuffer_depth_0_multisample" };
    constexpr auto framebufferMultisampleName{ "framebuffer_0_multisample" };

    config.assets.push_back(generateMultisampleRenderTextureAsset(renderTextureMultisampleName,
        renderResolutionWidth / 2, renderResolutionHeight, screenMSAASamples, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferMultisampleName, renderResolutionWidth / 2,
        renderResolutionHeight, screenMSAASamples, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferMultisampleName, 0, 0, renderResolutionWidth / 2, renderResolutionHeight,
            { { Visconfig::Assets::FramebufferType::TextureMultisample,
                  Visconfig::Assets::FramebufferDestination::Color0, renderTextureMultisampleName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferMultisampleName } }));

    auto focusEntity{ numEntities };

    auto threadTextureBorderWidth = static_cast<std::size_t>(threadViewTextureBorderRelativeWidth
        * std::pow(mdhConfig.mainView.threads.front().absoluteScale[0]
                * mdhConfig.mainView.threads.front().absoluteScale[1]
                * mdhConfig.mainView.threads.front().absoluteScale[2],
            1.0f / 3.0f));

    threadTextureBorderWidth = threadTextureBorderWidth == 0 ? 1 : threadTextureBorderWidth;

    for (auto layer{ mdhConfig.mainView.layers.begin() }; layer != mdhConfig.mainView.layers.end(); layer++) {
        auto index{ std::distance(mdhConfig.mainView.layers.begin(), layer) };

        auto mainTextureBorderWidth = static_cast<std::size_t>(mainViewTextureBorderRelativeWidth
            * std::pow(layer->absoluteScale[0] * layer->absoluteScale[1] * layer->absoluteScale[2], 1.0f / 3.0f));

        mainTextureBorderWidth = mainTextureBorderWidth == 0 ? 1 : mainTextureBorderWidth;

        auto textureFrontName{ "view_0_cube_texture_" + std::to_string(index) + "_front" };
        auto textureSideName{ "view_0_cube_texture_" + std::to_string(index) + "_side" };
        auto textureTopName{ "view_0_cube_texture_" + std::to_string(index) + "_top" };

        auto textureFrontPath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/" + textureFrontName
            + ".png" };
        auto textureSidePath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/" + textureSideName
            + ".png" };
        auto textureTopPath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/" + textureTopName
            + ".png" };

        if (static_cast<std::size_t>(index) == mdhConfig.mainView.layers.size() - 1) {
            generateTextureFile(textureFrontPath, static_cast<std::size_t>(layer->absoluteScale[0]),
                static_cast<std::size_t>(layer->absoluteScale[1]), 1, 1, mainTextureBorderWidth);
            generateTextureFile(textureSidePath, static_cast<std::size_t>(layer->absoluteScale[2]),
                static_cast<std::size_t>(layer->absoluteScale[1]), 1, 1, mainTextureBorderWidth);
            generateTextureFile(textureTopPath, static_cast<std::size_t>(layer->absoluteScale[0]),
                static_cast<std::size_t>(layer->absoluteScale[2]), 1, 1, mainTextureBorderWidth);
        } else {
            auto currentLayerScale{ layer->absoluteScale };
            auto nextLayerScale{ (layer + 1)->absoluteScale };

            std::array<std::size_t, 3> subdivisions{
                static_cast<std::size_t>(currentLayerScale[0] / nextLayerScale[0]),
                static_cast<std::size_t>(currentLayerScale[1] / nextLayerScale[1]),
                static_cast<std::size_t>(currentLayerScale[2] / nextLayerScale[2]),
            };

            generateTextureFile(textureFrontPath, static_cast<std::size_t>(layer->absoluteScale[0]),
                static_cast<std::size_t>(layer->absoluteScale[1]), subdivisions[0], subdivisions[1],
                mainTextureBorderWidth);
            generateTextureFile(textureSidePath, static_cast<std::size_t>(layer->absoluteScale[2]),
                static_cast<std::size_t>(layer->absoluteScale[1]), subdivisions[2], subdivisions[1],
                mainTextureBorderWidth);
            generateTextureFile(textureTopPath, static_cast<std::size_t>(layer->absoluteScale[0]),
                static_cast<std::size_t>(layer->absoluteScale[2]), subdivisions[0], subdivisions[2],
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

        world.entities.push_back(generateMainViewCube(mdhConfig, mdhConfig.mainView, numEntities, numEntities - 1,
            index, textureFrontName, textureSideName, textureTopName));
        numEntities++;
    }

    std::vector<std::size_t> threadParents{ numEntities - 1 };

    for (auto layer{ mdhConfig.mainView.threads.begin() }; layer != mdhConfig.mainView.threads.end(); layer++) {
        std::vector<std::size_t> newParents{};
        auto index{ std::distance(mdhConfig.mainView.threads.begin(), layer) };

        auto textureFrontName{ "view_0_cube_texture_" + std::to_string(index + mdhConfig.mainView.layers.size())
            + "_front" };
        auto textureSideName{ "view_0_cube_texture_" + std::to_string(index + mdhConfig.mainView.layers.size())
            + "_side" };
        auto textureTopName{ "view_0_cube_texture_" + std::to_string(index + mdhConfig.mainView.layers.size())
            + "_top" };

        auto textureFrontPath{ std::string{ assetsTextureDirectory } + "/" + textureFrontName + ".png" };
        auto textureSidePath{ std::string{ assetsTextureDirectory } + "/" + textureSideName + ".png" };
        auto textureTopPath{ std::string{ assetsTextureDirectory } + "/" + textureTopName + ".png" };

        if (static_cast<std::size_t>(index) == mdhConfig.mainView.threads.size() - 1) {
            generateTextureFile(textureFrontPath, static_cast<std::size_t>(layer->absoluteScale[0]),
                static_cast<std::size_t>(layer->absoluteScale[1]), 1, 1, threadTextureBorderWidth);
            generateTextureFile(textureSidePath, static_cast<std::size_t>(layer->absoluteScale[2]),
                static_cast<std::size_t>(layer->absoluteScale[1]), 1, 1, threadTextureBorderWidth);
            generateTextureFile(textureTopPath, static_cast<std::size_t>(layer->absoluteScale[0]),
                static_cast<std::size_t>(layer->absoluteScale[2]), 1, 1, threadTextureBorderWidth);
        } else {
            auto currentLayerScale{ layer->absoluteScale };
            auto nextLayerScale{ (layer + 1)->absoluteScale };

            std::array<std::size_t, 3> subdivisions{
                static_cast<std::size_t>(currentLayerScale[0] / nextLayerScale[0]),
                static_cast<std::size_t>(currentLayerScale[1] / nextLayerScale[1]),
                static_cast<std::size_t>(currentLayerScale[2] / nextLayerScale[2]),
            };

            generateTextureFile(textureFrontPath, static_cast<std::size_t>(layer->absoluteScale[0]),
                static_cast<std::size_t>(layer->absoluteScale[1]), subdivisions[0], subdivisions[1],
                threadTextureBorderWidth);
            generateTextureFile(textureSidePath, static_cast<std::size_t>(layer->absoluteScale[2]),
                static_cast<std::size_t>(layer->absoluteScale[1]), subdivisions[2], subdivisions[1],
                threadTextureBorderWidth);
            generateTextureFile(textureTopPath, static_cast<std::size_t>(layer->absoluteScale[0]),
                static_cast<std::size_t>(layer->absoluteScale[2]), subdivisions[0], subdivisions[2],
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
            for (std::size_t x{ 0 }; x < layer->numThreads[0]; x++) {
                for (std::size_t y{ 0 }; y < layer->numThreads[1]; y++) {
                    for (std::size_t z{ 0 }; z < layer->numThreads[2]; z++) {
                        world.entities.push_back(generateMainViewThreadCube(mdhConfig, mdhConfig.mainView, numEntities,
                            parent, index, { x, y, z }, textureFrontName, textureSideName, textureTopName));
                        newParents.push_back(numEntities++);
                    }
                }
            }
        }
        threadParents = std::move(newParents);
    }

    auto cameraDistance{ 2.0f
        * std::max({ mdhConfig.mainView.layers[0].absoluteScale[0], mdhConfig.mainView.layers[0].absoluteScale[1],
            mdhConfig.mainView.layers[0].absoluteScale[2] }) };

    auto cameraWidth{ 1.2f
        * std::max({ mdhConfig.mainView.layers[0].absoluteScale[0], mdhConfig.mainView.layers[0].absoluteScale[1] }) };
    auto cameraHeight{ cameraWidth / cameraAspectSmall };

    auto cameraEntity{ numEntities++ };
    world.entities.push_back(generateViewCamera(cameraEntity, focusEntity, 0, framebufferMultisampleName, cameraFOV,
        cameraAspectSmall, cameraNear, cameraFar, cameraDistance, cameraWidth, cameraHeight, true, false,
        mdhConfig.mainView.layers[0].absoluteScale[2] != 1.0f));
    extendCopy(world, framebufferMultisampleName, framebufferName,
        { Visconfig::Components::CopyOperationFlag::Color, Visconfig::Components::CopyOperationFlag::Depth },
        Visconfig::Components::CopyOperationFilter::Nearest);
    extentComposition(world, { 0.5f, 1.0f }, { -0.5f, 0.0f }, { renderTextureName }, defaultFramebufferAsset,
        viewCompositionShaderAsset, 0, false);
    extendCameraSwitcher(world, cameraEntity);
}

void generateSubViewConfig(Visconfig::Config& config, const ProcessedConfig& mdhConfig, Visconfig::World& world,
    std::size_t& numEntities, std::size_t subview, std::size_t numSubViews, const std::filesystem::path& workingDir)
{
    auto renderTextureName{ "render_texture_" + std::to_string(subview + 2) };
    auto depthBufferName{ "renderbuffer_depth_" + std::to_string(subview + 2) };
    auto framebufferName{ "framebuffer_" + std::to_string(subview + 2) };

    config.assets.push_back(generateRenderTextureAsset(
        renderTextureName, renderResolutionWidth, renderResolutionHeight, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferName, renderResolutionWidth, renderResolutionHeight, 0,
        Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferName, 0, 0, renderResolutionWidth, renderResolutionHeight,
            { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
                  renderTextureName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferName } }));

    auto renderTextureMultisampleName{ renderTextureName + "_multisample" };
    auto depthBufferMultisampleName{ depthBufferName + "_multisample" };
    auto framebufferMultisampleName{ framebufferName + "_multisample" };

    config.assets.push_back(generateMultisampleRenderTextureAsset(renderTextureMultisampleName, renderResolutionWidth,
        renderResolutionHeight, screenMSAASamples, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferMultisampleName, renderResolutionWidth,
        renderResolutionHeight, screenMSAASamples, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferMultisampleName, 0, 0, renderResolutionWidth, renderResolutionHeight,
            { { Visconfig::Assets::FramebufferType::TextureMultisample,
                  Visconfig::Assets::FramebufferDestination::Color0, renderTextureMultisampleName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferMultisampleName } }));

    auto textureBorderWidth = static_cast<std::size_t>(subViewTextureBorderRelativeWidth
        * std::pow(mdhConfig.subViews[subview].layers.front().absoluteScale[0]
                * mdhConfig.subViews[subview].layers.front().absoluteScale[1]
                * mdhConfig.subViews[subview].layers.front().absoluteScale[2],
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

        auto textureFrontPath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/" + textureFrontName
            + ".png" };
        auto textureSidePath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/" + textureSideName
            + ".png" };
        auto textureTopPath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/" + textureTopName
            + ".png" };

        generateTextureFile(textureFrontPath,
            static_cast<std::size_t>(mdhConfig.subViews[subview].layers[index].absoluteScale[0]),
            static_cast<std::size_t>(mdhConfig.subViews[subview].layers[index].absoluteScale[1]), 1, 1,
            textureBorderWidth);
        generateTextureFile(textureSidePath,
            static_cast<std::size_t>(mdhConfig.subViews[subview].layers[index].absoluteScale[2]),
            static_cast<std::size_t>(mdhConfig.subViews[subview].layers[index].absoluteScale[1]), 1, 1,
            textureBorderWidth);
        generateTextureFile(textureTopPath,
            static_cast<std::size_t>(mdhConfig.subViews[subview].layers[index].absoluteScale[0]),
            static_cast<std::size_t>(mdhConfig.subViews[subview].layers[index].absoluteScale[2]), 1, 1,
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

        world.entities.push_back(generateSubViewCube(mdhConfig, mdhConfig.subViews[subview], numEntities,
            numEntities - 1, subview, index, textureFrontName, textureSideName, textureTopName));
        numEntities++;
    }

    auto cameraDistance{ 1.5f
        * std::max({ mdhConfig.subViews[subview].layers[0].absoluteScale[0],
            mdhConfig.subViews[subview].layers[0].absoluteScale[1],
            mdhConfig.subViews[subview].layers[0].absoluteScale[2] }) };

    auto cameraWidth{ 1.2f
        * std::max({ mdhConfig.subViews[subview].layers[0].absoluteScale[0],
            mdhConfig.subViews[subview].layers[0].absoluteScale[1] }) };
    auto cameraHeight{ cameraWidth / cameraAspect };

    auto cameraEntity{ numEntities++ };
    world.entities.push_back(generateViewCamera(cameraEntity, focusEntity, subview + 2, framebufferMultisampleName,
        cameraFOV, cameraAspect, cameraNear, cameraFar, cameraDistance, cameraWidth, cameraHeight, false, true,
        mdhConfig.subViews[subview].layers[0].absoluteScale[2] != 1.0f));
    extendCopy(world, framebufferMultisampleName, framebufferName,
        { Visconfig::Components::CopyOperationFlag::Color, Visconfig::Components::CopyOperationFlag::Depth },
        Visconfig::Components::CopyOperationFilter::Nearest);
    extentComposition(world, { 0.2f, 0.2f }, { 0.7f, (-0.7f + ((numSubViews - 1) * 0.5f)) - (subview * 0.5f) },
        { renderTextureName }, defaultFramebufferAsset, viewCompositionShaderAsset, subview + 2, true);
    extendCameraSwitcher(world, cameraEntity);
}

void generateOutputViewConfig(Visconfig::Config& config, const ProcessedConfig& mdhConfig, Visconfig::World& world,
    std::size_t& numEntities, std::size_t subview, const std::filesystem::path& workingDir)
{
    auto renderTextureName{ "render_texture_" + std::to_string(subview) };
    auto depthBufferName{ "renderbuffer_depth_" + std::to_string(subview) };
    auto framebufferName{ "framebuffer_" + std::to_string(subview) };

    config.assets.push_back(generateRenderTextureAsset(
        renderTextureName, renderResolutionWidth / 2, renderResolutionHeight, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferName, renderResolutionWidth / 2,
        renderResolutionHeight, 0, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferName, 0, 0, renderResolutionWidth / 2, renderResolutionHeight,
            { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
                  renderTextureName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferName } }));

    auto renderTextureMultisampleName{ renderTextureName + "_multisample" };
    auto depthBufferMultisampleName{ depthBufferName + "_multisample" };
    auto framebufferMultisampleName{ framebufferName + "_multisample" };

    config.assets.push_back(generateMultisampleRenderTextureAsset(renderTextureMultisampleName,
        renderResolutionWidth / 2, renderResolutionHeight, screenMSAASamples, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateRenderbufferAsset(depthBufferMultisampleName, renderResolutionWidth / 2,
        renderResolutionHeight, screenMSAASamples, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generateFramebufferAsset(framebufferMultisampleName, 0, 0, renderResolutionWidth / 2, renderResolutionHeight,
            { { Visconfig::Assets::FramebufferType::TextureMultisample,
                  Visconfig::Assets::FramebufferDestination::Color0, renderTextureMultisampleName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferMultisampleName } }));

    auto textureBorderWidth = static_cast<std::size_t>(mainViewTextureBorderRelativeWidth
        * std::pow(mdhConfig.mainView.layers[0].absoluteScale[0] * mdhConfig.mainView.layers[0].absoluteScale[1]
                * mdhConfig.mainView.layers[0].absoluteScale[2],
            1.0f / 3.0f));

    textureBorderWidth = textureBorderWidth == 0 ? 1 : textureBorderWidth;

    auto textureFrontName{ "view_" + std::to_string(subview) + "_cube_texture_0_front" };
    auto textureSideName{ "view_" + std::to_string(subview) + "_cube_texture_0_side" };
    auto textureTopName{ "view_" + std::to_string(subview) + "_cube_texture_0_top" };
    auto innerLayerTextureName{ "view_" + std::to_string(subview) + "_cube_texture_1" };

    auto textureFrontPath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/" + textureFrontName
        + ".png" };
    auto textureSidePath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/" + textureSideName
        + ".png" };
    auto textureTopPath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/" + textureTopName + ".png" };
    auto innerLayerTexturePath{ workingDir.string() + std::string{ assetsTextureDirectory } + "/"
        + innerLayerTextureName + ".png" };

    generateTextureFile(textureFrontPath, static_cast<std::size_t>(mdhConfig.outputView.size[0]),
        static_cast<std::size_t>(mdhConfig.outputView.size[1]), 1, 1, textureBorderWidth);
    generateTextureFile(textureSidePath, static_cast<std::size_t>(mdhConfig.outputView.size[2]),
        static_cast<std::size_t>(mdhConfig.outputView.size[1]), 1, 1, textureBorderWidth);
    generateTextureFile(textureTopPath, static_cast<std::size_t>(mdhConfig.outputView.size[0]),
        static_cast<std::size_t>(mdhConfig.outputView.size[2]), 1, 1, textureBorderWidth);
    generateTextureFile(innerLayerTexturePath, static_cast<std::size_t>(mdhConfig.outputView.size[0]),
        static_cast<std::size_t>(mdhConfig.outputView.size[1]), 1, 1, 0);

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
    world.entities.push_back(generateCube({ 0, 0, 0 }, mdhConfig.outputView.size, { 0.0f, 0.0f, 0.0f, 0.0f },
        numEntities++, 0, 1llu << subview, false, textureFrontName, textureSideName, textureTopName, cubeMeshAsset));

    auto parent{ focusEntity };
    for (auto layer{ mdhConfig.outputView.layers.begin() }; layer != mdhConfig.outputView.layers.end(); layer++) {
        auto index{ std::distance(mdhConfig.outputView.layers.begin(), layer) };

        auto meshName{ "view_" + std::to_string(subview) + "_mesh_" + std::to_string(index) };
        config.assets.push_back(createSimpleCubeMeshAsset(meshName));

        auto newParent{ numEntities };
        world.entities.push_back(generateOutputViewCube(world, mdhConfig, mdhConfig.outputView, numEntities, parent,
            subview, index, innerLayerTextureName, meshName));
        parent = newParent;
        numEntities++;
    }

    auto cameraDistance{ 2.0f
        * std::max({ mdhConfig.outputView.size[0], mdhConfig.outputView.size[1], mdhConfig.outputView.size[2] }) };

    auto cameraWidth{ 1.2f * std::max({ mdhConfig.outputView.size[0], mdhConfig.outputView.size[1] }) };
    auto cameraHeight{ cameraWidth / cameraAspectSmall };

    auto cameraEntity{ numEntities++ };
    world.entities.push_back(generateViewCamera(cameraEntity, focusEntity, subview, framebufferMultisampleName,
        cameraFOV, cameraAspectSmall, cameraNear, cameraFar, cameraDistance, cameraWidth, cameraHeight, false, false,
        mdhConfig.outputView.size[2] != 1.0f));
    extendCopy(world, framebufferMultisampleName, framebufferName,
        { Visconfig::Components::CopyOperationFlag::Color, Visconfig::Components::CopyOperationFlag::Depth },
        Visconfig::Components::CopyOperationFilter::Nearest);
    extentComposition(world, { 0.5f, 1.0f }, { 0.5f, 0.0f }, { renderTextureName }, defaultFramebufferAsset,
        viewCompositionShaderAsset, subview, false);
    extendCameraSwitcher(world, cameraEntity);
}

void generateWorld(Visconfig::Config& config, const ProcessedConfig& mdhConfig, const std::filesystem::path& workingDir)
{
    Visconfig::World world{};

    std::size_t numEntities{ 0 };
    world.entities.push_back(generateCoordinatorEntity(numEntities++));
    generateMainViewConfig(config, mdhConfig, world, numEntities, workingDir);
    generateOutputViewConfig(config, mdhConfig, world, numEntities, 1, workingDir);

    for (auto pos{ mdhConfig.subViews.begin() }; pos != mdhConfig.subViews.end(); pos++) {
        auto index{ std::distance(mdhConfig.subViews.begin(), pos) };
        generateSubViewConfig(config, mdhConfig, world, numEntities, index, mdhConfig.subViews.size(), workingDir);
    }
    config.worlds.push_back(world);
}

Visconfig::Config generateConfig(const ProcessedConfig& config, const std::filesystem::path& workingDir)
{
    Visconfig::Config visconfig{};

    visconfig.options.screenHeight = screenHeight;
    visconfig.options.screenWidth = screenWidth;
    visconfig.options.screenMSAASamples = screenMSAASamples;
    visconfig.options.screenFullscreen = screenFullscreen;

    visconfig.assets.push_back(createCubeMeshAsset());
    visconfig.assets.push_back(createCubeTextureAsset());
    visconfig.assets.push_back(createOutputCubeTextureAsset());
    visconfig.assets.push_back(createShaderAsset(cubeShaderVertexPath, cubeShaderFragmentPath, cubeShaderAsset));
    visconfig.assets.push_back(createShaderAsset(
        viewCompositionShaderVertexPath, viewCompositionShaderFragmentPath, viewCompositionShaderAsset));
    visconfig.assets.push_back(createDefaultFramebufferAsset());

    generateWorld(visconfig, config, workingDir);

    return visconfig;
}
