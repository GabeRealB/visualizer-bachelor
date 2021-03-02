#include <visconfig/Assets.hpp>

#include <algorithm>
#include <cstdlib>

namespace Visconfig::Assets {

void to_json(nlohmann::json& j, const std::shared_ptr<AssetData>& v, AssetType type)
{
    switch (type) {
    case AssetType::Mesh:
        to_json(j, *std::static_pointer_cast<MeshAsset>(v));
        break;
    case AssetType::TextureFile:
        to_json(j, *std::static_pointer_cast<TextureFileAsset>(v));
        break;
    case AssetType::TextureRaw:
        to_json(j, *std::static_pointer_cast<TextureRawAsset>(v));
        break;
    case AssetType::TextureMultisampleRaw:
        to_json(j, *std::static_pointer_cast<TextureMultisampleRawAsset>(v));
        break;
    case AssetType::Renderbuffer:
        to_json(j, *std::static_pointer_cast<RenderbufferAsset>(v));
        break;
    case AssetType::Shader:
        to_json(j, *std::static_pointer_cast<ShaderAsset>(v));
        break;
    case AssetType::Framebuffer:
        to_json(j, *std::static_pointer_cast<FramebufferAsset>(v));
        break;
    case AssetType::DefaultFramebuffer:
        to_json(j, *std::static_pointer_cast<DefaultFramebufferAsset>(v));
        break;
    }
}

void from_json(const nlohmann::json& j, std::shared_ptr<AssetData>& v, AssetType type)
{
    switch (type) {
    case AssetType::Mesh: {
        auto ptr{ std::make_shared<MeshAsset>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<AssetData>(ptr);
    } break;
    case AssetType::TextureFile: {
        auto ptr{ std::make_shared<TextureFileAsset>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<AssetData>(ptr);
    } break;
    case AssetType::TextureRaw: {
        auto ptr{ std::make_shared<TextureRawAsset>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<AssetData>(ptr);
    } break;
    case AssetType::TextureMultisampleRaw: {
        auto ptr{ std::make_shared<TextureMultisampleRawAsset>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<AssetData>(ptr);
    } break;
    case AssetType::Renderbuffer: {
        auto ptr{ std::make_shared<RenderbufferAsset>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<AssetData>(ptr);
    } break;
    case AssetType::Shader: {
        auto ptr{ std::make_shared<ShaderAsset>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<AssetData>(ptr);
    } break;
    case AssetType::Framebuffer: {
        auto ptr{ std::make_shared<FramebufferAsset>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<AssetData>(ptr);
    } break;
    case AssetType::DefaultFramebuffer: {
        auto ptr{ std::make_shared<DefaultFramebufferAsset>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<AssetData>(ptr);
    } break;
    }
}

}