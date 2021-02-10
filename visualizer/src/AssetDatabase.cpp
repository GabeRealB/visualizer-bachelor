#include <visualizer/AssetDatabase.hpp>

#include <map>

namespace Visualizer {

struct StringCmp {
    using is_transparent = void;

    bool operator()(std::string_view a, std::string_view b) const { return a < b; }
};

struct AssetDatabaseImpl {
    ~AssetDatabaseImpl() noexcept;
    std::map<std::string, Asset, StringCmp> asset_map;
};

AssetDatabaseImpl s_asset_database{};

AssetDatabaseImpl::~AssetDatabaseImpl() noexcept {}

void AssetDatabase::clear()
{
    s_asset_database.asset_map.clear();
}

bool AssetDatabase::hasAsset(std::string_view name) { return s_asset_database.asset_map.contains(name); }

Asset AssetDatabase::getAsset(std::string_view name)
{
    if (auto pos{ s_asset_database.asset_map.find(name) }; pos != s_asset_database.asset_map.end()) {
        return pos->second;
    } else {
        return { getTypeId<void>(), nullptr };
    }
}

void AssetDatabase::setAsset(std::string_view name, const Asset& asset)
{
    s_asset_database.asset_map.insert_or_assign(std::string{ name }, asset);
}

void AssetDatabase::popAsset(std::string_view name)
{
    if (auto pos{ s_asset_database.asset_map.find(name) }; pos != s_asset_database.asset_map.end()) {
        s_asset_database.asset_map.erase(pos);
    }
}

}