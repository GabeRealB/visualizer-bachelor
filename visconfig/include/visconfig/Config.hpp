#pragma once

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include <visconfig/Assets.hpp>
#include <visconfig/Components.hpp>

namespace Visconfig {

struct Options {
    std::size_t screenWidth;
    std::size_t screenHeight;
    std::size_t screenMSAASamples;
    bool screenFullscreen;

    static constexpr const char* screenWidthJson{ "screen_width" };
    static constexpr const char* screenHeightJson{ "screen_height" };
    static constexpr const char* screenMSAASamplesJson{ "screen_msaa_samples" };
    static constexpr const char* screenFullscreenJson{ "screen_fullscreen" };
};

struct Asset {
    std::string name;
    Assets::AssetType type;
    std::shared_ptr<Assets::AssetData> data;

    static constexpr const char* nameJson{ "name" };
    static constexpr const char* typeJson{ "type" };
    static constexpr const char* dataJson{ "data" };
};

struct Component {
    Components::ComponentType type;
    std::shared_ptr<Components::ComponentData> data;

    static constexpr const char* typeJson{ "type" };
    static constexpr const char* dataJson{ "data" };
};

struct Entity {
    std::size_t id;
    std::vector<Component> components;

    static constexpr const char* idJson{ "id" };
    static constexpr const char* componentsJson{ "components" };
};

struct World {
    std::vector<Entity> entities;

    static constexpr const char* entitiesJson{ "entities" };
};

struct Config {
    Options options;
    std::vector<Asset> assets;
    std::vector<World> worlds;

    static constexpr const char* optionsJson{ "options" };
    static constexpr const char* assetsJson{ "assets" };
    static constexpr const char* worldsJson{ "worlds" };
};

Config from_file(const std::filesystem::path& path);
void to_file(const std::filesystem::path& path, const Config& config);

void to_json(nlohmann::json& j, const Config& v);
void from_json(const nlohmann::json& j, Config& v);

void to_json(nlohmann::json& j, const Options& v);
void from_json(const nlohmann::json& j, Options& v);

void to_json(nlohmann::json& j, const Asset& v);
void from_json(const nlohmann::json& j, Asset& v);

void to_json(nlohmann::json& j, const World& v);
void from_json(const nlohmann::json& j, World& v);

void to_json(nlohmann::json& j, const Entity& v);
void from_json(const nlohmann::json& j, Entity& v);

void to_json(nlohmann::json& j, const Component& v);
void from_json(const nlohmann::json& j, Component& v);

}