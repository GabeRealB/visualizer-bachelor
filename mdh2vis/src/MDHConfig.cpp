#include <MDHConfig.hpp>

#include <charconv>
#include <fstream>
#include <iostream>

namespace MDH2Vis {

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

}

namespace TPS {

    bool operator==(const Layer& lhs, const Layer& rhs) noexcept
    {
        return (lhs.sigArrayToOCL == rhs.sigArrayToOCL && lhs.sigMDH == rhs.sigMDH && lhs.sigBufferDo == rhs.sigBufferDo
            && lhs.tileSize == rhs.tileSize && lhs.numThreads == rhs.numThreads && lhs.memRegionInp == rhs.memRegionInp
            && lhs.memRegionRes == rhs.memRegionRes && lhs.cmbRes == rhs.cmbRes);
    }

    bool operator!=(const Layer& lhs, const Layer& rhs) noexcept { return !(lhs == rhs); }

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

}

bool operator==(const MDHConfig& lhs, const MDHConfig& rhs) noexcept
{
    return (lhs.model == rhs.model && lhs.tps == rhs.tps);
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
            TPS::TPS tps{};

            modelJson.get_to(model);
            tpsJson.get_to(tps);

            return MDHConfig{ model, tps };

        } catch (std::exception& ex) {
            std::cerr << ex.what() << std::endl;
            return std::nullopt;
        }
    }
}

}