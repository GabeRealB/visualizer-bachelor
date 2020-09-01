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

/*Enums*/

void to_json(nlohmann::json& j, const AssetType& v)
{
    switch (v) {
    case AssetType::Mesh:
        j = "mesh";
        break;
    case AssetType::TextureFile:
        j = "texture_file";
        break;
    case AssetType::TextureRaw:
        j = "texture_raw";
        break;
    case AssetType::Shader:
        j = "shader";
        break;
    case AssetType::Framebuffer:
        j = "framebuffer";
        break;
    case AssetType::DefaultFramebuffer:
        j = "default_framebuffer";
        break;
    }
}

void from_json(const nlohmann::json& j, AssetType& v)
{
    std::string type{};
    j.get_to(type);

    constexpr std::array<const char*, 6> assetTypeNames{ "mesh", "texture_file", "texture_raw", "shader", "framebuffer",
        "default_framebuffer" };
    auto predicate{ [&](const char* val) -> bool { return std::strcmp(val, type.c_str()) == 0; } };
    if (auto pos{ std::find_if(assetTypeNames.begin(), assetTypeNames.end(), predicate) };
        pos != assetTypeNames.end()) {
        auto index{ std::distance(assetTypeNames.begin(), pos) };
        switch (index) {
        case 0:
            v = AssetType::Mesh;
            return;
        case 1:
            v = AssetType::TextureFile;
            return;
        case 2:
            v = AssetType::TextureRaw;
            return;
        case 3:
            v = AssetType::Shader;
            return;
        case 4:
            v = AssetType::Framebuffer;
            return;
        case 5:
            v = AssetType::DefaultFramebuffer;
            return;
        }
    }
    std::abort();
}

std::unordered_map<TextureFormat, std::string> s_textureFormatNames{
    { TextureFormat::R, "r" },
    { TextureFormat::RG, "rg" },
    { TextureFormat::RGB, "rgb" },
    { TextureFormat::RGBA, "rgba" },
    { TextureFormat::R8, "r8" },
    { TextureFormat::RGBA16F, "rgba16" },
};

void to_json(nlohmann::json& j, const TextureFormat& v) { j = s_textureFormatNames[v]; }

void from_json(const nlohmann::json& j, TextureFormat& v)
{
    std::string type{};
    j.get_to(type);

    auto predicate{ [&](const decltype(s_textureFormatNames)::value_type& pair) -> bool {
        return pair.second == type;
    } };
    if (auto pos{ std::find_if(s_textureFormatNames.begin(), s_textureFormatNames.end(), predicate) };
        pos != s_textureFormatNames.end()) {
        v = pos->first;
    } else {
        std::abort();
    }
}

std::unordered_map<TextureAttributes, std::string> s_textureAttributesNames{
    { TextureAttributes::MagnificationLinear, "magnification_linear" },
    { TextureAttributes::MinificationLinear, "minification_linear" },
    { TextureAttributes::GenerateMipMaps, "generate_mipmaps" },
};

void to_json(nlohmann::json& j, const TextureAttributes& v) { j = s_textureAttributesNames[v]; }

void from_json(const nlohmann::json& j, TextureAttributes& v)
{
    std::string attribute{};
    j.get_to(attribute);

    auto predicate{ [&](const decltype(s_textureAttributesNames)::value_type& pair) -> bool {
        return pair.second == attribute;
    } };
    if (auto pos{ std::find_if(s_textureAttributesNames.begin(), s_textureAttributesNames.end(), predicate) };
        pos != s_textureAttributesNames.end()) {
        v = pos->first;
    } else {
        std::abort();
    }
}

std::unordered_map<FramebufferType, std::string> s_framebufferTypeNames{
    { FramebufferType::Texture, "texture" },
    { FramebufferType::Renderbuffer, "renderbuffer" },
};

void to_json(nlohmann::json& j, const FramebufferType& v) { j = s_framebufferTypeNames[v]; }

void from_json(const nlohmann::json& j, FramebufferType& v)
{
    std::string type{};
    j.get_to(type);

    auto predicate{ [&](const decltype(s_framebufferTypeNames)::value_type& pair) -> bool {
        return pair.second == type;
    } };
    if (auto pos{ std::find_if(s_framebufferTypeNames.begin(), s_framebufferTypeNames.end(), predicate) };
        pos != s_framebufferTypeNames.end()) {
        v = pos->first;
    } else {
        std::abort();
    }
}

std::unordered_map<FramebufferDestination, std::string> s_framebufferDestinationNames{
    { FramebufferDestination::Color0, "color0" },
    { FramebufferDestination::Color1, "color1" },
    { FramebufferDestination::Color2, "color2" },
    { FramebufferDestination::Color3, "color3" },
    { FramebufferDestination::Depth, "depth" },
    { FramebufferDestination::Stencil, "stencil" },
    { FramebufferDestination::DepthStencil, "depth_stencil" },
};

void to_json(nlohmann::json& j, const FramebufferDestination& v) { j = s_framebufferDestinationNames[v]; }

void from_json(const nlohmann::json& j, FramebufferDestination& v)
{
    std::string destination{};
    j.get_to(destination);

    auto predicate{ [&](const decltype(s_framebufferDestinationNames)::value_type& pair) -> bool {
        return pair.second == destination;
    } };
    if (auto pos{ std::find_if(s_framebufferDestinationNames.begin(), s_framebufferDestinationNames.end(), predicate) };
        pos != s_framebufferDestinationNames.end()) {
        v = pos->first;
    } else {
        std::abort();
    }
}

/*Assets*/

void to_json(nlohmann::json& j, const MeshAsset& v)
{
    j[MeshAsset::verticesJson] = v.vertices;
    j[MeshAsset::indicesJson] = v.indices;
    j[MeshAsset::texture_coords0Json] = v.texture_coords0;
}

void from_json(const nlohmann::json& j, MeshAsset& v)
{
    j[MeshAsset::verticesJson].get_to(v.vertices);
    j[MeshAsset::indicesJson].get_to(v.indices);
    j[MeshAsset::texture_coords0Json].get_to(v.texture_coords0);
}

void to_json(nlohmann::json& j, const TextureFileAsset& v)
{
    j[TextureFileAsset::pathJson] = v.path.string();
    j[TextureFileAsset::attributesJson] = v.attributes;
}

void from_json(const nlohmann::json& j, TextureFileAsset& v)
{
    std::string path{};
    j[TextureFileAsset::pathJson].get_to(path);
    j[TextureFileAsset::attributesJson].get_to(v.attributes);

    v.path = path;
}

void to_json(nlohmann::json& j, const TextureRawAsset& v)
{
    j[TextureRawAsset::widthJson] = v.width;
    j[TextureRawAsset::heightJson] = v.height;
    j[TextureRawAsset::formatJson] = v.format;
    j[TextureRawAsset::attributesJson] = v.attributes;
}

void from_json(const nlohmann::json& j, TextureRawAsset& v)
{
    j[TextureRawAsset::widthJson].get_to(v.width);
    j[TextureRawAsset::heightJson].get_to(v.height);
    j[TextureRawAsset::formatJson].get_to(v.format);
    j[TextureRawAsset::attributesJson].get_to(v.attributes);
}

void to_json(nlohmann::json& j, const ShaderAsset& v)
{
    j[ShaderAsset::vertexJson] = v.vertex.string();
    j[ShaderAsset::fragmentJson] = v.fragment.string();
}

void from_json(const nlohmann::json& j, ShaderAsset& v)
{
    std::string vertex{};
    std::string fragment{};
    j[ShaderAsset::vertexJson].get_to(vertex);
    j[ShaderAsset::fragmentJson].get_to(fragment);

    v.vertex = vertex;
    v.fragment = fragment;
}

void to_json(nlohmann::json& j, const FramebufferAsset& v) { j[FramebufferAsset::attachmentsJson] = v.attachments; }

void from_json(const nlohmann::json& j, FramebufferAsset& v)
{
    j[FramebufferAsset::attachmentsJson].get_to(v.attachments);
}

void to_json(nlohmann::json&, const DefaultFramebufferAsset&) {}

void from_json(const nlohmann::json&, DefaultFramebufferAsset&) {}

void to_json(nlohmann::json& j, const FramebufferAttachment& v)
{
    j[FramebufferAttachment::typeJson] = v.type;
    j[FramebufferAttachment::destinationJson] = v.destination;
    j[FramebufferAttachment::assetJson] = v.asset;
}

void from_json(const nlohmann::json& j, FramebufferAttachment& v)
{
    j[FramebufferAttachment::typeJson].get_to(v.type);
    j[FramebufferAttachment::destinationJson].get_to(v.destination);
    j[FramebufferAttachment::assetJson].get_to(v.asset);
}

}