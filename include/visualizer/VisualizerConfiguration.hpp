#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

#include <nlohmann/json.hpp>

namespace Visualizer {

/**
 * N-dimensional Vector.
 * @tparam T Type of the vector.
 * @tparam N Dimension of the vector.
 */
template <typename T, std::size_t N> struct VectorN {
    T data[N];

    T& operator[](std::size_t idx) { return data[idx]; }
    const T& operator[](std::size_t idx) const { return data[idx]; }
};

/**
 * Color info.
 */
using Color = VectorN<std::uint8_t, 3>;

/**
 * Position info.
 */
using Position = VectorN<std::int32_t, 3>;

/**
 * Tiling info.
 */
using TilingInfo = VectorN<std::uint32_t, 3>;

/**
 * Resolution info.
 */
using Resolution = VectorN<std::uint32_t, 2>;

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
    std::shared_ptr<InnerCube> innerCube;

    static constexpr auto colorJSONKey{ "color" };
    static constexpr auto tilingJSONKey{ "tiling" };
    static constexpr auto traversalOrderJSONKey{ "traversal_order" };
    static constexpr auto innerCubeJSONKey{ "inner_cube" };

    InnerCube() = default;
    InnerCube(Color color, TilingInfo tiling, TraversalOrder traversalOrder,
        std::shared_ptr<InnerCube> innerCube);
};

/**
 * An outer cube.
 */
struct OuterCube : public InnerCube {
    Position position;

    static constexpr auto positionJSONKey{ "position" };

    OuterCube() = default;
    OuterCube(Position pos, Color color, TilingInfo tiling, TraversalOrder traversalOrder,
        std::shared_ptr<InnerCube> innerCube);
};

/**
 * The configuration of the visualizer.
 */
struct VisualizerConfiguration {
    Resolution resolution;
    bool fullscreen;
    std::vector<OuterCube> cubes;

    static constexpr auto resolutionJSONKey{ "resolution" };
    static constexpr auto fullscreenJSONKey{ "fullscreen" };
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
 * @brief Compares two VectorN instances for equality.
 *
 * @tparam T Vector type.
 * @tparam N Vector size.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
template <typename T, std::size_t N>
bool operator==(const VectorN<T, N>& lhs, const VectorN<T, N>& rhs)
{
    for (std::size_t i = 0; i < N; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Compares two VectorN instances for inequality.
 *
 * @tparam T Vector type.
 * @tparam N Vector size.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if unequal.
 * @return <code>false<code> otherwise.
 */
template <typename T, std::size_t N>
bool operator!=(const VectorN<T, N>& lhs, const VectorN<T, N>& rhs)
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
 * @brief Serializes a N-dimensional vector.
 *
 * @tparam T Vector type.
 * @tparam N Vector dimension.
 * @param json Output.
 * @param vec Input.
 */
template <typename T, std::size_t N> void to_json(nlohmann::json& json, const VectorN<T, N>& vec)
{
    json = vec.data;
}

/**
 * @brief Deserializes a N-dimensional vector.
 *
 * @tparam T Vector type.
 * @tparam N Vector dimension.
 * @param json Input.
 * @param vec Output vector.
 */
template <typename T, std::size_t N> void from_json(const nlohmann::json& json, VectorN<T, N>& vec)
{
    json.get_to(vec.data);
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
