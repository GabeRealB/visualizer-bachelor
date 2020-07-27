#include <visualizer/AssetDatabase.hpp>

#include <map>

namespace Visualizer {

struct StringCmp {
    using is_transparent = void;

    bool operator()(std::string_view a, std::string_view b) const { return a < b; }
};

std::map<std::string, Asset, StringCmp> s_assetMap{};

bool AssetDatabase::hasAsset(std::string_view name) { return s_assetMap.contains(name); }

Asset AssetDatabase::getAsset(std::string_view name)
{
    if (auto pos{ s_assetMap.find(name) }; pos != s_assetMap.end()) {
        return pos->second;
    } else {
        return { getTypeId<void>(), nullptr };
    }
}

void AssetDatabase::setAsset(std::string_view name, const Asset& asset)
{
    s_assetMap.insert_or_assign(std::string{ name }, asset);
}

void AssetDatabase::popAsset(std::string_view name)
{
    if (auto pos{ s_assetMap.find(name) }; pos != s_assetMap.end()) {
        s_assetMap.erase(pos);
    }
}

}