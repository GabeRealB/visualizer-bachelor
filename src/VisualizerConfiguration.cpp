#include <visualizer/VisualizerConfiguration.hpp>

#include <fstream>
#include <iomanip>

namespace Visualizer {

InnerCube::InnerCube(Color color, TilingInfo tiling, TraversalOrder traversalOrder,
    std::vector<InnerCube> innerCubes)
    : color{ color }
    , tiling{ tiling }
    , traversalOrder{ traversalOrder }
    , innerCubes{ std::move(innerCubes) }
{
}

OuterCube::OuterCube(Position pos, Size size, Color color, TilingInfo tiling,
    TraversalOrder traversalOrder, std::vector<InnerCube> innerCubes)
    : InnerCube(color, tiling, traversalOrder, std::move(innerCubes))
    , position{ pos }
    , size{ size }
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
        } catch (nlohmann::json::exception&) {
            return std::nullopt;
        }
    }
}

bool saveConfig(const std::filesystem::path& filePath, const VisualizerConfiguration& config)
{
    try {
        nlohmann::json j{ config };
        std::ofstream file{ filePath };
        file << std::setw(4) << j << std::endl;
        return true;
    } catch (std::exception&) {
        return false;
    }
}

bool operator==(const InnerCube& lhs, const InnerCube& rhs)
{
    return (lhs.color == rhs.color) && (lhs.tiling == rhs.tiling)
        && (lhs.traversalOrder == rhs.traversalOrder) && (lhs.innerCubes == rhs.innerCubes);
}

bool operator!=(const InnerCube& lhs, const InnerCube& rhs) { return !(lhs == rhs); }

bool operator==(const OuterCube& lhs, const OuterCube& rhs)
{
    return (lhs.position == rhs.position) && (lhs.size == rhs.size)
        && static_cast<const InnerCube&>(lhs) == static_cast<const InnerCube&>(rhs);
}

bool operator!=(const OuterCube& lhs, const OuterCube& rhs) { return !(lhs == rhs); }

bool operator==(const VisualizerConfiguration& lhs, const VisualizerConfiguration& rhs)
{
    return lhs.cubes == rhs.cubes;
}

bool operator!=(const VisualizerConfiguration& lhs, const VisualizerConfiguration& rhs)
{
    return !(lhs == rhs);
}

void to_json(nlohmann::json& json, const InnerCube& cube)
{
    json = nlohmann::json{ { InnerCube::colorJSONKey, cube.color },
        { InnerCube::tilingJSONKey, cube.tiling },
        { InnerCube::traversalOrderJSONKey, cube.traversalOrder },
        { InnerCube::innerCubesJSONKey, cube.innerCubes } };
}

void to_json(nlohmann::json& json, const OuterCube& cube)
{
    to_json(json, static_cast<const InnerCube&>(cube));
    json[OuterCube::positionJSONKey] = cube.position;
    json[OuterCube::sizeJSONKey] = cube.size;
}

void to_json(nlohmann::json& json, const VisualizerConfiguration& config)
{
    json = nlohmann::json{ VisualizerConfiguration::cubesJSONKey, config.cubes };
}

void from_json(const nlohmann::json& json, InnerCube& cube)
{
    json.at(InnerCube::colorJSONKey).get_to(cube.color);
    json.at(InnerCube::tilingJSONKey).get_to(cube.tiling);
    json.at(InnerCube::innerCubesJSONKey).get_to(cube.innerCubes);

    if (auto traversalOrderSearch = json.find(InnerCube::traversalOrderJSONKey);
        traversalOrderSearch != json.end()) {
        traversalOrderSearch->get_to(cube.traversalOrder);
    } else {
        cube.traversalOrder = TraversalOrder::XYZ;
    }
}

void from_json(const nlohmann::json& json, OuterCube& cube)
{
    from_json(json, static_cast<InnerCube&>(cube));
    json.at(OuterCube::positionJSONKey).get_to(cube.position);
    json.at(OuterCube::sizeJSONKey).get_to(cube.size);
}

void from_json(const nlohmann::json& json, VisualizerConfiguration& config)
{
    json.at(VisualizerConfiguration::cubesJSONKey).get_to(config.cubes);
}
}
