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
            break;
        case 1:
            v = AssetType::TextureFile;
            break;
        case 2:
            v = AssetType::TextureRaw;
            break;
        case 3:
            v = AssetType::Shader;
            break;
        case 4:
            v = AssetType::Framebuffer;
            break;
        case 5:
            v = AssetType::DefaultFramebuffer;
            break;
        }
    }
    std::abort();
}

void to_json(nlohmann::json& j, const TextureFormat& v)
{
    switch (v) {
    case TextureFormat::RGB:
        j = "rgb";
        break;
    }
}

void from_json(const nlohmann::json& j, TextureFormat& v)
{
    std::string type{};
    j.get_to(type);

    constexpr std::array<const char*, 1> formatNames{ "rgb" };
    auto predicate{ [&](const char* val) -> bool { return std::strcmp(val, type.c_str()) == 0; } };
    if (auto pos{ std::find_if(formatNames.begin(), formatNames.end(), predicate) }; pos != formatNames.end()) {
        auto index{ std::distance(formatNames.begin(), pos) };
        switch (index) {
        case 0:
            v = TextureFormat::RGB;
            break;
        }
    }
    std::abort();
}

void to_json(nlohmann::json& j, const TextureAttributes& v)
{
    switch (v) {
    case TextureAttributes::MagnificationLinear:
        j = "magnification_linear";
        break;
    case TextureAttributes::MinificationLinear:
        j = "minification_linear";
        break;
    case TextureAttributes::GenerateMipMaps:
        j = "generate_mipmaps";
        break;
    }
}

void from_json(const nlohmann::json& j, TextureAttributes& v)
{
    std::string attribute{};
    j.get_to(attribute);

    constexpr std::array<const char*, 3> attributeNames{ "magnification_linear", "minification_linear",
        "generate_mipmaps" };
    auto predicate{ [&](const char* val) -> bool { return std::strcmp(val, attribute.c_str()) == 0; } };
    if (auto pos{ std::find_if(attributeNames.begin(), attributeNames.end(), predicate) };
        pos != attributeNames.end()) {
        auto index{ std::distance(attributeNames.begin(), pos) };
        switch (index) {
        case 0:
            v = TextureAttributes::MagnificationLinear;
            break;
        case 1:
            v = TextureAttributes::MinificationLinear;
            break;
        case 2:
            v = TextureAttributes::GenerateMipMaps;
            break;
        }
    }
    std::abort();
}

void to_json(nlohmann::json& j, const FramebufferType& v)
{
    switch (v) {
    case FramebufferType::Texture:
        j = "texture";
        break;
    case FramebufferType::Renderbuffer:
        j = "renderbuffer";
        break;
    }
}

void from_json(const nlohmann::json& j, FramebufferType& v)
{
    std::string type{};
    j.get_to(type);

    constexpr std::array<const char*, 2> typeNames{ "texture", "renderbuffer" };
    auto predicate{ [&](const char* val) -> bool { return std::strcmp(val, type.c_str()) == 0; } };
    if (auto pos{ std::find_if(typeNames.begin(), typeNames.end(), predicate) }; pos != typeNames.end()) {
        auto index{ std::distance(typeNames.begin(), pos) };
        switch (index) {
        case 0:
            v = FramebufferType::Texture;
            break;
        case 1:
            v = FramebufferType::Renderbuffer;
            break;
        }
    }
    std::abort();
}

void to_json(nlohmann::json& j, const FramebufferDestination& v)
{
    switch (v) {
    case FramebufferDestination::Color0:
        j = "color0";
        break;
    }
}

void from_json(const nlohmann::json& j, FramebufferDestination& v)
{
    std::string destination{};
    j.get_to(destination);

    constexpr std::array<const char*, 1> destinationNames{ "color0" };
    auto predicate{ [&](const char* val) -> bool { return std::strcmp(val, destination.c_str()) == 0; } };
    if (auto pos{ std::find_if(destinationNames.begin(), destinationNames.end(), predicate) };
        pos != destinationNames.end()) {
        auto index{ std::distance(destinationNames.begin(), pos) };
        switch (index) {
        case 0:
            v = FramebufferDestination::Color0;
            break;
        }
    }
    std::abort();
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
    j[TextureFileAsset::pathJson] = v.path;
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
    j[ShaderAsset::vertexJson] = v.vertex;
    j[ShaderAsset::fragmentJson] = v.fragment;
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

void to_json(nlohmann::json& j, const FramebufferAsset& v)
{
    j[FramebufferAsset::typeJson] = v.type;
    j[FramebufferAsset::destinationJson] = v.destination;
    j[FramebufferAsset::assetJson] = v.asset;
}

void from_json(const nlohmann::json& j, FramebufferAsset& v)
{
    j[FramebufferAsset::typeJson].get_to(v.type);
    j[FramebufferAsset::destinationJson].get_to(v.destination);
    j[FramebufferAsset::assetJson].get_to(v.asset);
}

void to_json(nlohmann::json&, const DefaultFramebufferAsset&) {}

void from_json(const nlohmann::json&, DefaultFramebufferAsset&) {}

}