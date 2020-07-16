#include <MDHConfig.hpp>

#include <charconv>
#include <fstream>
#include <iostream>

namespace MDH2Vis {

namespace MDH {

    Expression::Expression(ExpressionType type)
        : type{ type }
    {
    }

    ConstantExpression::ConstantExpression(std::uint32_t constant)
        : Expression{ ExpressionType::Constant }
        , constant{ constant }
    {
    }

    std::uint32_t ConstantExpression::apply(std::uint32_t, std::uint32_t, std::uint32_t) { return constant; }

    ConstantOperation::ConstantOperation(OperatorExpr operator_expr, std::uint32_t constant)
        : operator_expr{ operator_expr }
        , constant{ constant }
    {
    }

    std::uint32_t ConstantOperation::apply(std::uint32_t in)
    {
        switch (operator_expr) {
        case OperatorExpr::ADD:
            return in + constant;
        case OperatorExpr::SUB:
            return in - constant;
        case OperatorExpr::MUL:
            return in * constant;
        case OperatorExpr::DIV:
            return in / constant;
        default:
            return 0;
        }
    }

    ComponentExpression::ComponentExpression(std::uint8_t component)
        : Expression{ ExpressionType::Component }
        , component{ component }
    {
    }

    ComponentExpression::ComponentExpression(std::uint8_t component, ConstantOperation constant_operation)
        : Expression{ ExpressionType::Component }
        , component{ component }
        , constant_operation{ constant_operation }
    {
    }

    std::uint32_t ComponentExpression::apply(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3)
    {
        std::uint32_t input = 0;

        switch (component) {
        case 0:
            input = i1;
            break;
        case 1:
            input = i2;
            break;
        case 2:
            input = i3;
            break;
        default:
            return 0;
        }

        if (constant_operation) {
            return constant_operation->apply(input);
        } else {
            return input;
        }
    }
}

namespace Model {

    bool operator==(const Colors& lhs, const Colors& rhs) noexcept
    {
        return (lhs.tile == rhs.tile && lhs.memory == rhs.memory && lhs.thread == rhs.thread
            && lhs.tileOutOfBorder == rhs.tileOutOfBorder && lhs.threadOutOfBorder == rhs.threadOutOfBorder);
    }

    bool operator!=(const Colors& lhs, const Colors& rhs) noexcept { return !(lhs == rhs); }

    bool operator==(const Layer& lhs, const Layer& rhs) noexcept
    {
        return (lhs.name == rhs.name && lhs.nameThreads == rhs.nameThreads && lhs.nameMemory == rhs.nameMemory
            && lhs.colors == rhs.colors);
    }

    bool operator!=(const Layer& lhs, const Layer& rhs) noexcept { return !(lhs == rhs); }

    bool operator==(const Model& lhs, const Model& rhs) noexcept
    {
        return (lhs.layer0 == rhs.layer0 && lhs.layer1 == rhs.layer1 && lhs.layer2 == rhs.layer2);
    }

    bool operator!=(const Model& lhs, const Model& rhs) noexcept { return !(lhs == rhs); }

    void from_json(const nlohmann::json& j, Colors& v)
    {
        j[Colors::tileJson].get_to(v.tile);
        j[Colors::memoryJson].get_to(v.memory);
        j[Colors::threadJson].get_to(v.thread);
        j[Colors::tileOutOfBorderJson].get_to(v.tileOutOfBorder);
        j[Colors::threadOutOfBorderJson].get_to(v.threadOutOfBorder);
    }

    void from_json(const nlohmann::json& j, Layer& v)
    {
        j[Layer::nameJson].get_to(v.name);
        j[Layer::nameThreadsJson].get_to(v.nameThreads);
        j[Layer::nameMemoryJson].get_to(v.nameMemory);
        j[Layer::colorsJson].get_to(v.colors);
    }

    void from_json(const nlohmann::json& j, Model& v)
    {
        j[Model::layer0Json].get_to(v.layer0);
        j[Model::layer1Json].get_to(v.layer1);
        j[Model::layer2Json].get_to(v.layer2);
    }
}

namespace MDH {

    bool operator==(const MDHIn& lhs, const MDHIn& rhs) noexcept
    {
        return lhs.combine_operators == rhs.combine_operators;
    }

    bool operator!=(const MDHIn& lhs, const MDHIn& rhs) noexcept { return !(lhs == rhs); }

    bool operator==(const Views& lhs, const Views& rhs) noexcept
    {
        return (lhs.input == rhs.input && lhs.output == rhs.output);
    }

    bool operator!=(const Views& lhs, const Views& rhs) noexcept { return !(lhs == rhs); }

    bool operator==(const MDH& lhs, const MDH& rhs) noexcept { return (lhs.mdh == rhs.mdh && lhs.views == rhs.views); }

    bool operator!=(const MDH& lhs, const MDH& rhs) noexcept { return !(lhs == rhs); }

    void from_json(const nlohmann::json& j, ExpressionPtr& v)
    {
        if (j.is_number()) {
            v = std::make_shared<ConstantExpression>(0);
            j.get_to(static_cast<ConstantExpression&>(*v.get()));
        } else {
            v = std::make_shared<ComponentExpression>(0);
            j.get_to(static_cast<ComponentExpression&>(*v.get()));
        }
    }

    void from_json(const nlohmann::json& j, ConstantExpression& v) { j.get_to(v.constant); }

    void from_json(const nlohmann::json& j, ComponentExpression& v)
    {
        std::string op{};
        j.get_to(op);

        std::uint8_t component{ 0 };
        std::from_chars(&op[1], &op[2], component);

        v.component = component - 1;

        if (op.size() > 3) {
            OperatorExpr operator_expr{ OperatorExpr::ADD };
            switch (op[2]) {
            case '+':
                operator_expr = OperatorExpr::ADD;
                break;
            case '-':
                operator_expr = OperatorExpr::SUB;
                break;
            case '*':
                operator_expr = OperatorExpr::MUL;
                break;
            case '/':
                operator_expr = OperatorExpr::DIV;
                break;
            }

            std::uint32_t constant{ 0 };
            std::from_chars(&op[3], op.c_str() + op.size(), constant);

            v.constant_operation = ConstantOperation{ operator_expr, constant };
        }
    }

    void from_json(const nlohmann::json& j, MDHIn& v) { j[MDHIn::combine_operatorsJson].get_to(v.combine_operators); }

    void from_json(const nlohmann::json& j, Views& v)
    {
        j[Views::inputJson].get_to(v.input);
        j[Views::outputJson].get_to(v.output);
    }

    void from_json(const nlohmann::json& j, MDH& v)
    {
        j[MDH::mdhJson].get_to(v.mdh);
        j[MDH::viewsJson].get_to(v.views);
    }

}

namespace TPS {

    bool operator==(const Layer& lhs, const Layer& rhs) noexcept
    {
        return (lhs.sigArrayToOCL == rhs.sigArrayToOCL && lhs.sigMDH == rhs.sigMDH && lhs.sigBufferDo == rhs.sigBufferDo
            && lhs.tileSize == rhs.tileSize && lhs.numThreads == rhs.numThreads && lhs.memRegionInp == rhs.memRegionInp
            && lhs.memRegionRes == rhs.memRegionRes && lhs.cmbRes == rhs.cmbRes);
    }

    bool operator!=(const Layer& lhs, const Layer& rhs) noexcept { return !(lhs == rhs); }

    bool operator==(const TPS& lhs, const TPS& rhs) noexcept
    {
        return (lhs.layer0 == rhs.layer0 && lhs.layer1 == rhs.layer1 && lhs.layer2 == rhs.layer2);
    }

    bool operator!=(const TPS& lhs, const TPS& rhs) noexcept { return !(lhs == rhs); }

    void from_json(const nlohmann::json& j, Layer& v)
    {
        j[Layer::sigArrayToOCLJson].get_to(v.sigArrayToOCL);
        j[Layer::sigMDHJson].get_to(v.sigMDH);
        j[Layer::sigBufferDoJson].get_to(v.sigBufferDo);
        j[Layer::tileSizeJson].get_to(v.tileSize);
        j[Layer::numThreadsJson].get_to(v.numThreads);
        j[Layer::memRegionInpJson].get_to(v.memRegionInp);
        j[Layer::memRegionResJson].get_to(v.memRegionRes);
        j[Layer::cmbResJson].get_to(v.cmbRes);
    }

    void from_json(const nlohmann::json& j, TPS& v)
    {
        j[TPS::layer0Json].get_to(v.layer0);
        j[TPS::layer1Json].get_to(v.layer1);
        j[TPS::layer2Json].get_to(v.layer2);
    }

}

bool operator==(const MDHConfig& lhs, const MDHConfig& rhs) noexcept
{
    return (lhs.model == rhs.model && lhs.mdh == rhs.mdh && lhs.tps == rhs.tps);
}

bool operator!=(const MDHConfig& lhs, const MDHConfig& rhs) noexcept { return !(lhs == rhs); }

std::optional<MDHConfig> loadFromFiles(
    const std::filesystem::path& modelPath, const std::filesystem::path& mdhPath, const std::filesystem::path& tpsPath)
{
    if (!std::filesystem::exists(modelPath) || !std::filesystem::exists(mdhPath) || !std::filesystem::exists(tpsPath)) {
        return std::nullopt;
    } else {
        try {
            std::ifstream modelStream{ modelPath };
            std::ifstream mdhStream{ mdhPath };
            std::ifstream tpsStream{ tpsPath };

            nlohmann::json modelJson{};
            nlohmann::json mdhJson{};
            nlohmann::json tpsJson{};

            modelStream >> modelJson;
            mdhStream >> mdhJson;
            tpsStream >> tpsJson;

            Model::Model model{};
            MDH::MDH mdh{};
            TPS::TPS tps{};

            modelJson.get_to(model);
            mdhJson.get_to(mdh);
            tpsJson.get_to(tps);

            return MDHConfig{ model, mdh, tps };

        } catch (std::exception& ex) {
            std::cerr << ex.what() << std::endl;
            return std::nullopt;
        }
    }
}

}