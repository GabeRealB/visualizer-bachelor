#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <thread>

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
    computeBufferSizes(*mdhConfig);
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
    BoundsMap in_bounds_map{};
    BoundsMap out_bounds_map{};

    for (auto& input : config.mdh.views.input) {
        in_bounds_map.insert_or_assign(input.first, StartBounds3D);
    }

    for (auto& output : config.mdh.views.output) {
        out_bounds_map.insert_or_assign(output.first, StartBounds3D);
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
    std::size_t numViews{ in_bounds_map.size() + out_bounds_map.size() };

    std::cout << "Computing buffer sizes... " << 0 << " of " << numViews;

    for (auto& input : in_bounds_map) {
        const auto& operationContainer{ MDH2Vis::OperationMap::getOperations(input.first) };
        computeBounds(input.second, operationContainer, maxX, maxY, maxZ);
        viewNumber++;

        // std::cout << "\r\e[K" << std::flush;
        std::cout << "\33[2K\r" << std::flush;
        std::cout << "Computing buffer sizes... " << viewNumber << " of " << numViews;
    }

    for (auto& output : out_bounds_map) {
        const auto& operationContainer{ MDH2Vis::OperationMap::getOperations(output.first) };
        computeBounds(output.second, operationContainer, maxX, maxY, maxZ);
        viewNumber++;

        // std::cout << "\r\e[K" << std::flush;
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

    std::cout << "Input:" << std::endl;
    computeSizes(buffer_sizes.inputMap, in_bounds_map);
    std::cout << std::endl;
    std::cout << "Output:" << std::endl;
    computeSizes(buffer_sizes.outputMap, out_bounds_map);
    std::cout << std::endl;

    return buffer_sizes;
}