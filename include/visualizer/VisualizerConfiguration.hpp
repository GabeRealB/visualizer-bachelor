#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

#include <nlohmann/json.hpp>

namespace Visualizer {

/**
 * Three-dimensional Vector.
 * @tparam T
 */
template <typename T> struct Vector3 {
    T x;
    T y;
    T z;
};

/**
 * Color info.
 */
using Color = Vector3<std::uint8_t>;

/**
 * Size info.
 */
using Size = Vector3<std::uint32_t>;

/**
 * Position info.
 */
using Position = Vector3<std::int32_t>;

/**
 * Tiling info.
 */
using TilingInfo = Vector3<std::uint32_t>;

/**
 * Order in which the dimensions are traversed.
 */
enum class TraversalOrder : std::uint8_t {
    XYZ = 12,
    XZY = 21,
    YXZ = 102,
    YZX = 201,
    ZXY = 120,
    ZYX = 210
};

/**
 * An inner cube.
 */
struct InnerCube {
    Color color;
    TilingInfo tiling;
    TraversalOrder traversalOrder;
    std::vector<InnerCube> innerCubes;

    static constexpr auto colorJSONKey{ "color" };
    static constexpr auto tilingJSONKey{ "tiling" };
    static constexpr auto traversalOrderJSONKey{ "traversal_order" };
    static constexpr auto innerCubesJSONKey{ "inner_cubes" };

    InnerCube() = default;
    InnerCube(Color color, TilingInfo tiling, TraversalOrder traversalOrder,
        std::vector<InnerCube> innerCubes);
};

/**
 * An outer cube.
 */
struct OuterCube : public InnerCube {
    Position position;
    Size size;

    static constexpr auto positionJSONKey{ "position" };
    static constexpr auto sizeJSONKey{ "size" };

    OuterCube() = default;
    OuterCube(Position pos, Size size, Color color, TilingInfo tiling,
        TraversalOrder traversalOrder, std::vector<InnerCube> innerCubes);
};

/**
 * The configuration of the visualizer.
 */
struct VisualizerConfiguration {
    std::vector<OuterCube> cubes;

    static constexpr auto cubesJSONKey{ "cubes" };
};

/**
 * @brief Loads the configuration from the file path.
 *
 * @param filePath File path.
 *
 * @return Configuration if successful.
 * @return Nothing otherwise.
 */
std::optional<VisualizerConfiguration> loadConfig(const std::filesystem::path& filePath);

/**
 * @brief Saves the configuration to a file.
 *
 * @param filePath File path.
 * @param config Configuration.
 *
 * @return <code>true<code> if successful.
 * @return <code>false<code> otherwise.
 */
bool saveConfig(const std::filesystem::path& filePath, const VisualizerConfiguration& config);

/**
 * @brief Compares two Vector3 instances for equality.
 *
 * @tparam T Vector type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
template <typename T> bool operator==(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
}

/**
 * @brief Compares two Vector3 instances for inequality.
 *
 * @tparam T Vector type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
template <typename T> bool operator!=(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
    return !(lhs == rhs);
}

/**
 * @brief Compares two InnerCubes instances for equality.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
bool operator==(const InnerCube& lhs, const InnerCube& rhs);

/**
 * @brief Compares two InnerCubes instances for inequality.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
bool operator!=(const InnerCube& lhs, const InnerCube& rhs);

/**
 * @brief Compares two OuterCube instances for equality.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
bool operator==(const OuterCube& lhs, const OuterCube& rhs);

/**
 * @brief Compares two OuterCube instances for inequality.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
bool operator!=(const OuterCube& lhs, const OuterCube& rhs);

/**
 * @brief Compares two VisualizerConfiguration instances for equality.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
bool operator==(const VisualizerConfiguration& lhs, const VisualizerConfiguration& rhs);

/**
 * @brief Compares two VisualizerConfiguration instances for inequality.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
bool operator!=(const VisualizerConfiguration& lhs, const VisualizerConfiguration& rhs);

/**
 * @brief Serializes a three-dimensional vector.
 *
 * @tparam T Vector type.
 * @param json Output.
 * @param vec Input.
 */
template <typename T> void to_json(nlohmann::json& json, const Vector3<T>& vec)
{

    json[0] = vec.x;
    json[1] = vec.y;
    json[2] = vec.z;
}

/**
 * @brief Deserializes a three-dimensional vector.
 *
 * @tparam T Vector type.
 * @param json Input.
 * @param vec Output vector.
 */
template <typename T> void from_json(const nlohmann::json& json, Vector3<T>& vec)
{
    json.at(0).get_to(vec.x);
    json.at(1).get_to(vec.y);
    json.at(2).get_to(vec.z);
}

/**
 * @brief Serializes an inner cube.
 *
 * @param json Output.
 * @param cube Cube.
 */
void to_json(nlohmann::json& json, const InnerCube& cube);

/**
 * @brief Serializes an outer cube.
 *
 * @param json Output.
 * @param cube Cube.
 */
void to_json(nlohmann::json& json, const OuterCube& cube);

/**
 * @brief Serializes an configuration instance.
 *
 * @param json Output.
 * @param config Config.
 */
void to_json(nlohmann::json& json, const VisualizerConfiguration& config);

/**
 * @brief Deserializes an inner cube.
 *
 * @param json Input.
 * @param cube Cube.
 */
void from_json(const nlohmann::json& json, InnerCube& cube);

/**
 * @brief Deserializes an outer cube.
 *
 * @param json Input.
 * @param cube Cube.
 */
void from_json(const nlohmann::json& json, OuterCube& cube);

/**
 * @brief Deserializes a configuration instance.
 *
 * @param json Input.
 * @param config Config.
 */
void from_json(const nlohmann::json& json, VisualizerConfiguration& config);

}
