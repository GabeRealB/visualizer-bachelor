#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <thread>

#include <visconfig/Config.hpp>

#include "MDHConfig.hpp"
#include "MDHOps.hpp"

constexpr std::string_view UsageStr{ "Usage: mdh2vis --model model-path --tps tps-path [--out output-dir]" };

constexpr std::size_t screenWidth = 1200;
constexpr std::size_t screenHeight = 900;
constexpr bool screenFullscreen = false;

constexpr float minTransparency{ 0.1f };
constexpr float maxTransparency{ 0.95f };

constexpr auto cubeMeshAsset{ "cube_mesh" };
constexpr auto cubeTextureAsset{ "cube_texture" };
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

Visconfig::Config generateConfig(const ProcessedConfig& config);

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

    for (auto pos{ view.layers.begin() }; pos != view.layers.end(); pos++) {

        auto index{ std::distance(view.layers.begin(), pos) };

        const auto& currentLayer{ *pos };
        const auto& mainViewLayer{ config.mainView.layers[index] };

        pos->iterationRate
            = mainViewLayer.absoluteScale[0] * mainViewLayer.absoluteScale[1] * mainViewLayer.absoluteScale[2];

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

    auto computeRelativePositionAndSize{ Overload{ [](OutputLayerInfo& layer) {
                                                      layer.size = { 1.0f, 1.0f, 1.0f };
                                                      layer.numIterations = { 0, 0, 0 };
                                                      for (auto position : layer.absolutePositions) {
                                                          position[0] /= layer.absoluteSize[0];
                                                          position[1] /= layer.absoluteSize[1];
                                                          position[2] /= layer.absoluteSize[2];
                                                          layer.positions.push_back(position);
                                                      }
                                                  },
        [](OutputLayerInfo& layer, const OutputLayerInfo& previousLayer) {
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
        } } };

    for (const auto& layer : config.mainView.layers) {
        config.outputView.layers.push_back(generateLayer(dimensionTypes, threadDimensions, layer.absoluteScale));
    }

    for (auto pos{ config.outputView.layers.rbegin() }; pos != config.outputView.layers.rend(); pos++) {
        if (pos == config.outputView.layers.rbegin()) {
            const auto& threadLayer{ config.mainView.threads.front() };
            auto iterationRate{ static_cast<std::size_t>(threadDimensions[0])
                * static_cast<std::size_t>(threadDimensions[1]) * static_cast<std::size_t>(threadDimensions[2]) };
            computePositions.operator()(*pos, threadDimensions, threadLayer.numIterations, iterationRate);
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
            computeRelativePositionAndSize.operator()(*pos);
        } else {
            computeRelativePositionAndSize.operator()(*pos, *(pos - 1));
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

float interpolateLinear(float startValue, float endValue, std::size_t min, std::size_t max, std::size_t current)
{
    if (current == min) {
        return startValue;
    } else if (current == max) {
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

Visconfig::Asset createCubeTextureAsset()
{
    auto textureData{ std::make_shared<Visconfig::Assets::TextureFileAsset>() };
    Visconfig::Asset asset{ cubeTextureAsset, Visconfig::Assets::AssetType::TextureFile, textureData };
    textureData->path = "assets/textures/cube.png";
    textureData->attributes.push_back(Visconfig::Assets::TextureAttributes::MagnificationLinear);
    textureData->attributes.push_back(Visconfig::Assets::TextureAttributes::MinificationLinear);
    textureData->attributes.push_back(Visconfig::Assets::TextureAttributes::GenerateMipMaps);

    return asset;
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

Visconfig::Asset generateFramebufferAsset(const std::string& name,
    const std::vector<std::tuple<Visconfig::Assets::FramebufferType, Visconfig::Assets::FramebufferDestination,
        std::string>>& attachments)
{
    auto bufferData{ std::make_shared<Visconfig::Assets::FramebufferAsset>() };
    Visconfig::Asset asset{ name, Visconfig::Assets::AssetType::Framebuffer, bufferData };

    for (auto& attachment : attachments) {
        bufferData->attachments.push_back(Visconfig::Assets::FramebufferAttachment{
            std::get<0>(attachment), std::get<1>(attachment), std::get<2>(attachment) });
    }

    return asset;
}

Visconfig::Entity generateMainViewCube(const ProcessedConfig& mdhConfig, const MainViewInfo& view, std::size_t entityId,
    std::size_t parentId, std::size_t layerNumber)
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

    auto texture{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuseColor{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto gridScale{ std::make_shared<Visconfig::Components::Vec2ArrayMaterialAttribute>() };

    texture->asset = cubeTextureAsset;
    texture->slot = 0;

    diffuseColor->value[0] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[0]) / 255.0f;
    diffuseColor->value[1] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[1]) / 255.0f;
    diffuseColor->value[2] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[2]) / 255.0f;
    diffuseColor->value[3] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.tile[3]) / 255.0f;
    diffuseColor->value[3] = interpolateLinear(
        minTransparency, maxTransparency, 0, view.layers.size() + view.threads.size() - 1, layerNumber);

    if (layerNumber != view.layers.size() - 1) {
        auto currentLayerScale{ view.layers[layerNumber].absoluteScale };
        auto nextLayerScale{ view.layers[layerNumber + 1].absoluteScale };

        MDH2Vis::VecN<float, 3> scales{
            static_cast<float>(currentLayerScale[0]) / static_cast<float>(nextLayerScale[0]),
            static_cast<float>(currentLayerScale[1]) / static_cast<float>(nextLayerScale[1]),
            static_cast<float>(currentLayerScale[2]) / static_cast<float>(nextLayerScale[2]),
        };

        gridScale->value.push_back({ scales[0], scales[1] });
        gridScale->value.push_back({ scales[0], scales[2] });
        gridScale->value.push_back({ scales[2], scales[1] });
    } else {
        MDH2Vis::VecN<float, 3> scales{
            static_cast<float>(view.layers[layerNumber].absoluteScale[0]),
            static_cast<float>(view.layers[layerNumber].absoluteScale[1]),
            static_cast<float>(view.layers[layerNumber].absoluteScale[2]),
        };

        gridScale->value.push_back({ scales[0], scales[1] });
        gridScale->value.push_back({ scales[0], scales[2] });
        gridScale->value.push_back({ scales[2], scales[1] });
    }

    material.asset = cubeShaderAsset;
    material.attributes.insert_or_assign("gridTexture",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture, false });
    material.attributes.insert_or_assign("diffuseColor",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuseColor, false });
    material.attributes.insert_or_assign("gridScale",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec2, gridScale, true });

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
    }

    return entity;
}

Visconfig::Entity generateMainViewThreadCube(const ProcessedConfig& mdhConfig, const MainViewInfo& view,
    std::size_t entityId, std::size_t parentId, std::size_t layerNumber, std::array<std::size_t, 3> blockNumber)
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

    auto texture{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuseColor{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto gridScale{ std::make_shared<Visconfig::Components::Vec2ArrayMaterialAttribute>() };

    texture->asset = cubeTextureAsset;
    texture->slot = 0;

    diffuseColor->value[0] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.thread[0]) / 255.0f;
    diffuseColor->value[1] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.thread[1]) / 255.0f;
    diffuseColor->value[2] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.thread[2]) / 255.0f;
    diffuseColor->value[3] = static_cast<float>(mdhConfig.config[layerNumber].model.colors.thread[3]) / 255.0f;
    diffuseColor->value[3] = interpolateLinear(minTransparency, maxTransparency, 0,
        view.layers.size() + view.threads.size() - 1, view.layers.size() + layerNumber);

    if (layerNumber != view.threads.size() - 1) {
        auto currentLayerScale{ view.threads[layerNumber].absoluteScale };
        auto nextLayerScale{ view.threads[layerNumber + 1].absoluteScale };

        MDH2Vis::VecN<float, 3> scales{
            static_cast<float>(currentLayerScale[0]) / static_cast<float>(nextLayerScale[0]),
            static_cast<float>(currentLayerScale[1]) / static_cast<float>(nextLayerScale[1]),
            static_cast<float>(currentLayerScale[2]) / static_cast<float>(nextLayerScale[2]),
        };

        gridScale->value.push_back({ scales[0], scales[1] });
        gridScale->value.push_back({ scales[0], scales[2] });
        gridScale->value.push_back({ scales[2], scales[1] });
    } else {
        MDH2Vis::VecN<float, 3> scales{
            static_cast<float>(view.threads[layerNumber].absoluteScale[0]),
            static_cast<float>(view.threads[layerNumber].absoluteScale[1]),
            static_cast<float>(view.threads[layerNumber].absoluteScale[2]),
        };

        gridScale->value.push_back({ scales[0], scales[1] });
        gridScale->value.push_back({ scales[0], scales[2] });
        gridScale->value.push_back({ scales[2], scales[1] });
    }

    material.asset = cubeShaderAsset;
    material.attributes.insert_or_assign("gridTexture",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture, false });
    material.attributes.insert_or_assign("diffuseColor",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuseColor, false });
    material.attributes.insert_or_assign("gridScale",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec2, gridScale, true });

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

        iteration.ticksPerIteration = view.threads[layerNumber].absoluteScale[0]
            * view.threads[layerNumber].absoluteScale[1] * view.threads[layerNumber].absoluteScale[2];
    }

    return entity;
}

Visconfig::Entity generateCube(std ::array<float, 3> position, std::array<float, 3> scale, std::array<float, 4> color,
    std::size_t entityId, std::size_t parentId, std::size_t layerId, bool hasParent)
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

    mesh.asset = cubeMeshAsset;

    auto texture{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuseColor{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto gridScale{ std::make_shared<Visconfig::Components::Vec2ArrayMaterialAttribute>() };

    texture->asset = cubeTextureAsset;
    texture->slot = 0;

    diffuseColor->value[0] = color[0];
    diffuseColor->value[1] = color[1];
    diffuseColor->value[2] = color[2];
    diffuseColor->value[3] = color[3];

    gridScale->value.push_back({ 1.0f, 1.0f });
    gridScale->value.push_back({ 1.0f, 1.0f });
    gridScale->value.push_back({ 1.0f, 1.0f });

    material.asset = cubeShaderAsset;
    material.attributes.insert_or_assign("gridTexture",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture, false });
    material.attributes.insert_or_assign("diffuseColor",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuseColor, false });
    material.attributes.insert_or_assign("gridScale",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec2, gridScale, true });

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
    std::size_t layerNumber)
{
    Visconfig::Entity entity{};
    entity.id = entityId;

    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::EntityActivation,
        std::make_shared<Visconfig::Components::EntityActivationComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Parent, std::make_shared<Visconfig::Components::ParentComponent>() });

    if (layerNumber > 0) {
        entity.components.push_back({ Visconfig::Components::ComponentType::ImplicitIteration,
            std::make_shared<Visconfig::Components::ImplicitIterationComponent>() });
    }

    constexpr auto transformIndex{ 0 };
    constexpr auto entityActivationIndex{ 1 };
    constexpr auto parentIndex{ 2 };
    constexpr auto iterationIndex{ 3 };

    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transformIndex].data) };
    auto& entityActivation{ *std::static_pointer_cast<Visconfig::Components::EntityActivationComponent>(
        entity.components[entityActivationIndex].data) };
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
        interpolateLinear(minTransparency, maxTransparency, 0, view.layers.size(), layerNumber + 1),
    };

    auto& threadLayer{ mdhConfig.mainView.threads.front() };
    std::array<float, 3> childScale{
        threadLayer.absoluteScale[0] / mdhConfig.outputView.layers[layerNumber].absoluteSize[0],
        threadLayer.absoluteScale[1] / mdhConfig.outputView.layers[layerNumber].absoluteSize[1],
        threadLayer.absoluteScale[2] / mdhConfig.outputView.layers[layerNumber].absoluteSize[2],
    };

    std::array<float, 3> childHalfScale{
        childScale[0] / 2.0f,
        childScale[1] / 2.0f,
        childScale[2] / 2.0f,
    };

    std::array<float, 3> childStartPos{
        -0.5f + childHalfScale[0],
        0.5f - childHalfScale[1],
        -0.5f + childHalfScale[2],
    };

    auto parentEntity{ entityId };
    std::vector<std::size_t> children{};
    for (auto pos : view.layers[layerNumber].positions) {
        auto first{ entityId == parentEntity };
        auto cubeLayer{ first ? 1llu << viewNumber : 0 };
        entityId++;

        pos[0] += childStartPos[0];
        pos[1] += childStartPos[1];
        pos[2] += childStartPos[2];

        children.push_back(entityId);
        world.entities.push_back(generateCube(pos, childScale, color, entityId, parentEntity, cubeLayer, true));
    }

    entityActivation.layer = 1llu << viewNumber;
    entityActivation.entities = children;
    entityActivation.ticksPerIteration = view.layers[layerNumber].iterationRates;

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
    std::size_t parentId, std::size_t viewNumber, std::size_t layerNumber)
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

    auto texture{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuseColor{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto gridScale{ std::make_shared<Visconfig::Components::Vec2ArrayMaterialAttribute>() };

    texture->asset = cubeTextureAsset;
    texture->slot = 0;

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
        = interpolateLinear(minTransparency, maxTransparency, 0, view.layers.size() - 1, layerNumber);

    gridScale->value.push_back({ 1, 1 });
    gridScale->value.push_back({ 1, 1 });
    gridScale->value.push_back({ 1, 1 });

    material.asset = cubeShaderAsset;
    material.attributes.insert_or_assign("gridTexture",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture, false });
    material.attributes.insert_or_assign("diffuseColor",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuseColor, false });
    material.attributes.insert_or_assign("gridScale",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec2, gridScale, true });

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

Visconfig::Entity generateViewCamera(
    std::size_t entityId, std::size_t focus, std::size_t viewIndex, const std::string& framebufferName)
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
    camera->active = false;
    camera->fixed = true;
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

    auto fixedCamera{ std::static_pointer_cast<Visconfig::Components::FixedCameraComponent>(
        entity.components[3].data) };
    fixedCamera->focus = focus;
    fixedCamera->distance = 10.0f;
    fixedCamera->horizontalAngle = 0.0f;
    fixedCamera->verticalAngle = 0.0f;

    if (viewIndex == 0) {
        camera->active = true;
        camera->fixed = false;
    }

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

    return entity;
}

void extentComposition(Visconfig::World& world, std::array<float, 2> scale, std::array<float, 2> position,
    std::vector<std::string> src, const std::string& target, const std::string& shader)
{
    auto composition{ std::static_pointer_cast<Visconfig::Components::CompositionComponent>(
        world.entities[0].components[0].data) };

    composition->operations.push_back(Visconfig::Components::CompositionOperation{
        { scale[0], scale[1] }, { position[0], position[1] }, std::move(src), target, shader });
}

void extendCameraSwitcher(Visconfig::World& world, std::size_t camera)
{
    auto cameraSwitcher{ std::static_pointer_cast<Visconfig::Components::CameraSwitcherComponent>(
        world.entities[0].components[1].data) };

    cameraSwitcher->cameras.push_back(camera);
}

void generateMainViewConfig(
    Visconfig::Config& config, const ProcessedConfig& mdhConfig, Visconfig::World& world, std::size_t& numEntities)
{
    constexpr auto renderTextureName{ "render_texture_0" };
    constexpr auto framebufferName{ "framebuffer_0" };

    config.assets.push_back(generateRenderTextureAsset(
        renderTextureName, screenWidth, screenHeight, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateFramebufferAsset(framebufferName,
        { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
            renderTextureName } }));

    auto focusEntity{ numEntities };

    for (auto layer{ mdhConfig.mainView.layers.begin() }; layer != mdhConfig.mainView.layers.end(); layer++) {
        auto index{ std::distance(mdhConfig.mainView.layers.begin(), layer) };
        world.entities.push_back(
            generateMainViewCube(mdhConfig, mdhConfig.mainView, numEntities, numEntities - 1, index));
        numEntities++;
    }

    std::vector<std::size_t> threadParents{ numEntities - 1 };

    for (auto layer{ mdhConfig.mainView.threads.begin() }; layer != mdhConfig.mainView.threads.end(); layer++) {
        std::vector<std::size_t> newParents{};
        auto index{ std::distance(mdhConfig.mainView.threads.begin(), layer) };

        for (auto& parent : threadParents) {
            for (std::size_t x{ 0 }; x < layer->numThreads[0]; x++) {
                for (std::size_t y{ 0 }; y < layer->numThreads[1]; y++) {
                    for (std::size_t z{ 0 }; z < layer->numThreads[2]; z++) {
                        world.entities.push_back(generateMainViewThreadCube(
                            mdhConfig, mdhConfig.mainView, numEntities, parent, index, { x, y, z }));
                        newParents.push_back(numEntities++);
                    }
                }
            }
        }
        threadParents = std::move(newParents);
    }

    auto cameraEntity{ numEntities++ };
    world.entities.push_back(generateViewCamera(cameraEntity, focusEntity, 0, framebufferName));
    extentComposition(world, { 0.5f, 1.0f }, { -0.5f, 0.0f }, { renderTextureName }, defaultFramebufferAsset,
        viewCompositionShaderAsset);
    extendCameraSwitcher(world, cameraEntity);
}

void generateSubViewConfig(Visconfig::Config& config, const ProcessedConfig& mdhConfig, Visconfig::World& world,
    std::size_t& numEntities, std::size_t subview, std::size_t numSubViews)
{
    auto renderTextureName{ "render_texture_" + std::to_string(subview + 2) };
    auto framebufferName{ "framebuffer_" + std::to_string(subview + 2) };

    config.assets.push_back(generateRenderTextureAsset(
        renderTextureName, screenWidth, screenHeight, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateFramebufferAsset(framebufferName,
        { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
            renderTextureName } }));

    auto focusEntity{ numEntities };

    for (auto layer{ mdhConfig.config.begin() }; layer != mdhConfig.config.end(); layer++) {
        auto index{ std::distance(mdhConfig.config.begin(), layer) };
        world.entities.push_back(
            generateSubViewCube(mdhConfig, mdhConfig.subViews[subview], numEntities, numEntities - 1, subview, index));
        numEntities++;
    }

    auto cameraEntity{ numEntities++ };
    world.entities.push_back(generateViewCamera(cameraEntity, focusEntity, subview + 2, framebufferName));
    extentComposition(world, { 0.2f, 0.2f }, { 0.7f, (-0.7f + ((numSubViews - 1) * 0.5f)) - (subview * 0.5f) },
        { renderTextureName }, defaultFramebufferAsset, viewCompositionShaderAsset);
    extendCameraSwitcher(world, cameraEntity);
}

void generateOutputViewConfig(Visconfig::Config& config, const ProcessedConfig& mdhConfig, Visconfig::World& world,
    std::size_t& numEntities, std::size_t subview)
{
    auto renderTextureName{ "render_texture_" + std::to_string(subview) };
    auto framebufferName{ "framebuffer_" + std::to_string(subview) };

    config.assets.push_back(generateRenderTextureAsset(
        renderTextureName, screenWidth, screenHeight, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generateFramebufferAsset(framebufferName,
        { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
            renderTextureName } }));

    auto focusEntity{ numEntities };
    world.entities.push_back(generateCube({ 0, 0, 0 }, mdhConfig.outputView.size, { 1.0f, 1.0f, 1.0f, minTransparency },
        numEntities++, 0, 1llu << subview, false));

    auto parent{ focusEntity };
    for (auto layer{ mdhConfig.outputView.layers.begin() }; layer != mdhConfig.outputView.layers.end(); layer++) {
        auto index{ std::distance(mdhConfig.outputView.layers.begin(), layer) };
        auto newParent{ numEntities };
        world.entities.push_back(
            generateOutputViewCube(world, mdhConfig, mdhConfig.outputView, numEntities, parent, subview, index));
        parent = newParent;
        numEntities++;
    }

    auto cameraEntity{ numEntities++ };
    world.entities.push_back(generateViewCamera(cameraEntity, focusEntity, subview, framebufferName));
    extentComposition(world, { 0.5f, 1.0f }, { 0.5f, 0.0f }, { renderTextureName }, defaultFramebufferAsset,
        viewCompositionShaderAsset);
    extendCameraSwitcher(world, cameraEntity);
}

void generateWorld(Visconfig::Config& config, const ProcessedConfig& mdhConfig)
{
    Visconfig::World world{};

    std::size_t numEntities{ 0 };
    world.entities.push_back(generateCoordinatorEntity(numEntities++));
    generateMainViewConfig(config, mdhConfig, world, numEntities);
    generateOutputViewConfig(config, mdhConfig, world, numEntities, 1);

    for (auto pos{ mdhConfig.subViews.begin() }; pos != mdhConfig.subViews.end(); pos++) {
        auto index{ std::distance(mdhConfig.subViews.begin(), pos) };
        generateSubViewConfig(config, mdhConfig, world, numEntities, index, mdhConfig.subViews.size());
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
    visconfig.assets.push_back(createShaderAsset(cubeShaderVertexPath, cubeShaderFragmentPath, cubeShaderAsset));
    visconfig.assets.push_back(createShaderAsset(
        viewCompositionShaderVertexPath, viewCompositionShaderFragmentPath, viewCompositionShaderAsset));
    visconfig.assets.push_back(createDefaultFramebufferAsset());

    generateWorld(visconfig, config);

    return visconfig;
}