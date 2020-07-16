#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace MDH2Vis {

template <typename T, std::size_t N> struct VecN {
    T data[N];

    T& operator[](std::size_t idx) noexcept { return data[idx]; }
    const T& operator[](std::size_t idx) const noexcept { return data[idx]; }
};

namespace Model {

    using Color = VecN<std::uint8_t, 4>;

    struct Colors {
        Color tile;
        Color memory;
        Color thread;
        Color tileOutOfBorder;
        Color threadOutOfBorder;

        static constexpr const char* tileJson{ "tile" };
        static constexpr const char* memoryJson{ "memory" };
        static constexpr const char* threadJson{ "thread" };
        static constexpr const char* tileOutOfBorderJson{ "tile_out_of_border" };
        static constexpr const char* threadOutOfBorderJson{ "thread_out_of_border" };
    };

    struct Layer {
        std::string name;
        std::string nameThreads;
        std::string nameMemory;
        Colors colors;

        static constexpr const char* nameJson{ "name" };
        static constexpr const char* nameThreadsJson{ "name_threads" };
        static constexpr const char* nameMemoryJson{ "name_memory" };
        static constexpr const char* colorsJson{ "colors" };
    };

    struct Model {
        Layer layer0;
        Layer layer1;
        Layer layer2;

        static constexpr const char* layer0Json{ "layer 0" };
        static constexpr const char* layer1Json{ "layer 1" };
        static constexpr const char* layer2Json{ "layer 2" };
    };

}

namespace MDH {

    enum class OperatorExpr { ADD, SUB, MUL, DIV };
    enum class ExpressionType { Constant, Component };

    struct Expression {
        Expression(ExpressionType type);
        virtual ~Expression() = default;

        virtual std::uint32_t apply(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3) = 0;

        ExpressionType type;
    };

    struct ConstantExpression final : public Expression {
        ConstantExpression(std::uint32_t constant);
        std::uint32_t apply(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3) final;

        std::uint32_t constant;
    };

    struct ConstantOperation {
        ConstantOperation(OperatorExpr operator_expr, std::uint32_t constant);

        std::uint32_t apply(std::uint32_t in);

        OperatorExpr operator_expr;
        std::uint32_t constant;
    };

    struct ComponentExpression final : public Expression {
        ComponentExpression(std::uint8_t component);
        ComponentExpression(std::uint8_t component, ConstantOperation constant_operation);

        std::uint32_t apply(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3) final;

        std::uint8_t component;
        std::optional<ConstantOperation> constant_operation;
    };

    using ExpressionPtr = std::shared_ptr<Expression>;
    using ExpressionTup = VecN<ExpressionPtr, 3>;
    using ExpressionVector = std::vector<ExpressionTup>;
    using ExpressionMap = std::unordered_map<std::string, ExpressionVector>;

    struct MDHIn {
        std::vector<std::string> combine_operators;

        static constexpr const char* combine_operatorsJson{ "combine operators" };
    };

    struct Views {
        ExpressionMap input;
        ExpressionMap output;

        static constexpr const char* inputJson{ "input" };
        static constexpr const char* outputJson{ "output" };
    };

    struct MDH {
        MDHIn mdh;
        Views views;

        static constexpr const char* mdhJson{ "MDH" };
        static constexpr const char* viewsJson{ "views" };
    };

}

namespace TPS {

    using MemRegionMap = std::unordered_map<std::string, std::uint8_t>;
    using SigBufferMap = std::unordered_map<std::string, VecN<std::uint8_t, 3>>;

    struct Layer {
        VecN<std::uint8_t, 3> sigArrayToOCL;
        VecN<std::uint8_t, 3> sigMDH;
        SigBufferMap sigBufferDo;
        VecN<std::uint32_t, 3> tileSize;
        VecN<std::uint32_t, 3> numThreads;
        MemRegionMap memRegionInp;
        MemRegionMap memRegionRes;
        bool cmbRes;

        static constexpr const char* sigArrayToOCLJson{ "SIG_array_to_OCL" };
        static constexpr const char* sigMDHJson{ "SIG_mdh-do" };
        static constexpr const char* sigBufferDoJson{ "SIG_buffer-do" };
        static constexpr const char* tileSizeJson{ "TILE_SIZE" };
        static constexpr const char* numThreadsJson{ "NUM_THREADS" };
        static constexpr const char* memRegionInpJson{ "MEM_REGION_INP" };
        static constexpr const char* memRegionResJson{ "MEM_REGION_RES" };
        static constexpr const char* cmbResJson{ "CMB_RES" };
    };

    struct TPS {
        Layer layer0;
        Layer layer1;
        Layer layer2;

        static constexpr const char* layer0Json{ "layer 0" };
        static constexpr const char* layer1Json{ "layer 1" };
        static constexpr const char* layer2Json{ "layer 2" };
    };

}

struct MDHConfig {
    Model::Model model;
    MDH::MDH mdh;
    TPS::TPS tps;
};

/**
 * @brief Compares two VecN instances for equality.
 *
 * @tparam T Vector type.
 * @tparam N Vector size.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if equal.
 * @return <code>false<code> otherwise.
 */
template <typename T, std::size_t N> bool operator==(const VecN<T, N>& lhs, const VecN<T, N>& rhs)
{
    for (std::size_t i = 0; i < N; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Compares two VecN instances for inequality.
 *
 * @tparam T Vector type.
 * @tparam N Vector size.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 *
 * @return <code>true<code> if unequal.
 * @return <code>false<code> otherwise.
 */
template <typename T, std::size_t N> bool operator!=(const VecN<T, N>& lhs, const VecN<T, N>& rhs)
{
    return !(lhs == rhs);
}

/**
 * @brief Serializes a N-dimensional vector.
 *
 * @tparam T Vector type.
 * @tparam N Vector dimension.
 * @param j Output.
 * @param v Input.
 */
template <typename T, std::size_t N> void to_json(nlohmann::json& j, const VecN<T, N>& v) { j = v.data; }

/**
 * @brief Deserializes a N-dimensional vector.
 *
 * @tparam T Vector type.
 * @tparam N Vector dimension.
 * @param j Input.
 * @param v Output vector.
 */
template <typename T, std::size_t N> void from_json(const nlohmann::json& j, VecN<T, N>& v) { j.get_to(v.data); }

namespace Model {

    bool operator==(const Colors& lhs, const Colors& rhs) noexcept;
    bool operator!=(const Colors& lhs, const Colors& rhs) noexcept;

    bool operator==(const Layer& lhs, const Layer& rhs) noexcept;
    bool operator!=(const Layer& lhs, const Layer& rhs) noexcept;

    bool operator==(const Model& lhs, const Model& rhs) noexcept;
    bool operator!=(const Model& lhs, const Model& rhs) noexcept;

    void from_json(const nlohmann::json& j, Colors& v);
    void from_json(const nlohmann::json& j, Layer& v);
    void from_json(const nlohmann::json& j, Model& v);

}

namespace MDH {

    bool operator==(const MDHIn& lhs, const MDHIn& rhs) noexcept;
    bool operator!=(const MDHIn& lhs, const MDHIn& rhs) noexcept;

    bool operator==(const Views& lhs, const Views& rhs) noexcept;
    bool operator!=(const Views& lhs, const Views& rhs) noexcept;

    bool operator==(const MDH& lhs, const MDH& rhs) noexcept;
    bool operator!=(const MDH& lhs, const MDH& rhs) noexcept;

    void from_json(const nlohmann::json& j, ExpressionPtr& v);
    void from_json(const nlohmann::json& j, ConstantExpression& v);
    void from_json(const nlohmann::json& j, ComponentExpression& v);
    void from_json(const nlohmann::json& j, MDHIn& v);
    void from_json(const nlohmann::json& j, Views& v);
    void from_json(const nlohmann::json& j, MDH& v);

}

namespace TPS {

    bool operator==(const Layer& lhs, const Layer& rhs) noexcept;
    bool operator!=(const Layer& lhs, const Layer& rhs) noexcept;

    bool operator==(const TPS& lhs, const TPS& rhs) noexcept;
    bool operator!=(const TPS& lhs, const TPS& rhs) noexcept;

    void from_json(const nlohmann::json& j, Layer& v);
    void from_json(const nlohmann::json& j, TPS& v);

}

bool operator==(const MDHConfig& lhs, const MDHConfig& rhs) noexcept;
bool operator!=(const MDHConfig& lhs, const MDHConfig& rhs) noexcept;

std::optional<MDHConfig> loadFromFiles(
    const std::filesystem::path& modelPath, const std::filesystem::path& mdhPath, const std::filesystem::path& tpsPath);

}