#include <visualizer/VisualizerConfiguration.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>

namespace Visualizer {

InnerCube::InnerCube(
    Color color, TilingInfo tiling, TraversalOrder traversalOrder, std::shared_ptr<InnerCube> innerCube)
    : color{ color }
    , tiling{ tiling }
    , traversalOrder{ traversalOrder }
    , innerCube{ std::move(innerCube) }
{
}

OuterCube::OuterCube(
    Position pos, Color color, TilingInfo tiling, TraversalOrder traversalOrder, std::shared_ptr<InnerCube> innerCube)
    : InnerCube(color, tiling, traversalOrder, std::move(innerCube))
    , position{ pos }
{
}

std::optional<VisualizerConfiguration> loadConfig(const std::filesystem::path& filePath)
{
    if (!std::filesystem::is_regular_file(filePath)) {
        return std::nullopt;
    } else {
        try {
            std::ifstream file{ filePath };
            nlohmann::json j{};
            file >> j;
            return { j.get<VisualizerConfiguration>() };
        } catch (nlohmann::json::exception& e) {
            std::cerr << e.what() << std::endl;
            return std::nullopt;
        }
    }
}

bool saveConfig(const std::filesystem::path& filePath, const VisualizerConfiguration& config)
{
    try {
        nlohmann::json j{};
        j = config;
        std::ofstream file{ filePath };
        file << std::setw(4) << j << std::endl;
        return true;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

template <typename T>
static bool deepSharedPtrEqualityCheck(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs)
{
    if (lhs == rhs) {
        return true;
    } else {
        return (!((lhs == nullptr) || (rhs == nullptr))) && (*lhs == *rhs);
    }
}

bool operator==(const InnerCube& lhs, const InnerCube& rhs)
{
    return (lhs.color == rhs.color) && (lhs.tiling == rhs.tiling) && (lhs.traversalOrder == rhs.traversalOrder)
        && (deepSharedPtrEqualityCheck(lhs.innerCube, rhs.innerCube));
}

bool operator!=(const InnerCube& lhs, const InnerCube& rhs) { return !(lhs == rhs); }

bool operator==(const OuterCube& lhs, const OuterCube& rhs)
{
    return (lhs.position == rhs.position) && static_cast<const InnerCube&>(lhs) == static_cast<const InnerCube&>(rhs);
}

bool operator!=(const OuterCube& lhs, const OuterCube& rhs) { return !(lhs == rhs); }

bool operator==(const VisualizerConfiguration& lhs, const VisualizerConfiguration& rhs)
{
    return (lhs.resolution == rhs.resolution) && (lhs.fullscreen == rhs.fullscreen) && (lhs.cubes == rhs.cubes);
}

bool operator!=(const VisualizerConfiguration& lhs, const VisualizerConfiguration& rhs) { return !(lhs == rhs); }

void to_json(nlohmann::json& json, const InnerCube& cube)
{
    json[InnerCube::colorJSONKey] = cube.color;
    json[InnerCube::tilingJSONKey] = cube.tiling;
    json[InnerCube::traversalOrderJSONKey] = cube.traversalOrder;
    if (cube.innerCube != nullptr) {
        json[InnerCube::innerCubeJSONKey] = *cube.innerCube;
    }
}

void to_json(nlohmann::json& json, const OuterCube& cube)
{
    to_json(json, static_cast<const InnerCube&>(cube));
    json[OuterCube::positionJSONKey] = cube.position;
}

void to_json(nlohmann::json& json, const VisualizerConfiguration& config)
{
    json[VisualizerConfiguration::resolutionJSONKey] = config.resolution;
    json[VisualizerConfiguration::fullscreenJSONKey] = config.fullscreen;
    json[VisualizerConfiguration::cubesJSONKey] = config.cubes;
}

void from_json(const nlohmann::json& json, InnerCube& cube)
{
    json.at(InnerCube::colorJSONKey).get_to(cube.color);
    json.at(InnerCube::tilingJSONKey).get_to(cube.tiling);

    if (auto innerCubeSearch = json.find(InnerCube::innerCubeJSONKey); innerCubeSearch != json.end()) {
        cube.innerCube = std::make_shared<InnerCube>(innerCubeSearch->get<InnerCube>());
    }

    if (auto traversalOrderSearch = json.find(InnerCube::traversalOrderJSONKey); traversalOrderSearch != json.end()) {
        traversalOrderSearch->get_to(cube.traversalOrder);
    } else {
        cube.traversalOrder = TraversalOrder::XYZ;
    }
}

void from_json(const nlohmann::json& json, OuterCube& cube)
{
    from_json(json, static_cast<InnerCube&>(cube));
    json.at(OuterCube::positionJSONKey).get_to(cube.position);
}

void from_json(const nlohmann::json& json, VisualizerConfiguration& config)
{
    json.at(VisualizerConfiguration::resolutionJSONKey).get_to(config.resolution);
    json.at(VisualizerConfiguration::fullscreenJSONKey).get_to(config.fullscreen);
    json.at(VisualizerConfiguration::cubesJSONKey).get_to(config.cubes);
}
}
