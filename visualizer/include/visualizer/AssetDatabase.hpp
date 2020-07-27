#pragma once

#include <memory>
#include <string_view>

#include <visualizer/TypeId.hpp>

namespace Visualizer {

struct Asset {
    TypeId type;
    std::shared_ptr<void> data;
};

class AssetDatabase final {
public:
    AssetDatabase() = delete;

    static bool hasAsset(std::string_view name);

    static Asset getAsset(std::string_view name);

    static void setAsset(std::string_view name, const Asset& asset);

    static void popAsset(std::string_view name);
};

}