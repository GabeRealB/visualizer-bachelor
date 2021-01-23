#include <visconfig/Config.hpp>

#include <fstream>
#include <iostream>

namespace Visconfig {

Config from_file(const std::filesystem::path& path)
{
    if (!std::filesystem::is_regular_file(path)) {
        std::abort();
    } else {
        try {
            auto bin_path = path;
            bin_path.replace_extension(".vbin");
            std::ifstream file{ bin_path, std::ios::in | std::ios::binary };
            file.unsetf(std::ios::skipws);

            nlohmann::json j = nlohmann::json::from_cbor(
                std::istream_iterator<std::uint8_t>(file), std::istream_iterator<std::uint8_t>());
            // file >> j;
            return { j.get<Config>() };
        } catch (nlohmann::json::exception& e) {
            std::cerr << e.what() << std::endl;
            std::abort();
        }
    }
}

void to_file(const std::filesystem::path& path, const Config& config)
{
    try {
        nlohmann::json j{};
        j = config;
        std::ofstream file{ path };
        file << std::setw(4) << j << std::endl;

        auto bin_path = path;
        bin_path.replace_extension(".vbin");
        auto binary = nlohmann::json::to_cbor(j);
        std::ofstream bin_file{ bin_path, std::ios::out | std::ios::binary };
        bin_file.write(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(std::uint8_t));
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::abort();
    }
}

void to_json(nlohmann::json& j, const Config& v)
{
    j[Config::optionsJson] = v.options;
    j[Config::assetsJson] = v.assets;
    j[Config::worldsJson] = v.worlds;
}

void from_json(const nlohmann::json& j, Config& v)
{
    j[Config::optionsJson].get_to(v.options);
    j[Config::assetsJson].get_to(v.assets);
    j[Config::worldsJson].get_to(v.worlds);
}

void to_json(nlohmann::json& j, const Options& v)
{
    j[Options::screenWidthJson] = v.screenWidth;
    j[Options::screenHeightJson] = v.screenHeight;
    j[Options::screenMSAASamplesJson] = v.screenMSAASamples;
    j[Options::screenFullscreenJson] = v.screenFullscreen;
}

void from_json(const nlohmann::json& j, Options& v)
{
    j[Options::screenWidthJson].get_to(v.screenWidth);
    j[Options::screenHeightJson].get_to(v.screenHeight);
    j[Options::screenMSAASamplesJson].get_to(v.screenMSAASamples);
    j[Options::screenFullscreenJson].get_to(v.screenFullscreen);
}

void to_json(nlohmann::json& j, const Asset& v)
{
    j[Asset::nameJson] = v.name;
    j[Asset::typeJson] = v.type;

    nlohmann::json assetJson{};
    Assets::to_json(assetJson, v.data, v.type);

    j[Asset::dataJson] = assetJson;
}

void from_json(const nlohmann::json& j, Asset& v)
{
    j[Asset::nameJson].get_to(v.name);
    j[Asset::typeJson].get_to(v.type);

    Assets::from_json(j[Asset::dataJson], v.data, v.type);
}

void to_json(nlohmann::json& j, const World& v) { j[World::entitiesJson] = v.entities; }

void from_json(const nlohmann::json& j, World& v) { j[World::entitiesJson].get_to(v.entities); }

void to_json(nlohmann::json& j, const Entity& v)
{
    j[Entity::idJson] = v.id;
    j[Entity::componentsJson] = v.components;
}

void from_json(const nlohmann::json& j, Entity& v)
{
    j[Entity::idJson].get_to(v.id);
    j[Entity::componentsJson].get_to(v.components);
}

void to_json(nlohmann::json& j, const Component& v)
{
    j[Component::typeJson] = v.type;

    nlohmann::json componentJson{};
    Components::to_json(componentJson, v.data, v.type);

    j[Component::dataJson] = componentJson;
}

void from_json(const nlohmann::json& j, Component& v)
{
    j[Component::typeJson].get_to(v.type);

    Components::from_json(j[Component::dataJson], v.data, v.type);
}

}