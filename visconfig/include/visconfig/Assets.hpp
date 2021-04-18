#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace Visconfig::Assets {

enum class AssetType {
    Mesh,
    TextureFile,
    TextureRaw,
    TextureMultisampleRaw,
    CuboidRenderPipeline,
    Renderbuffer,
    Shader,
    Framebuffer,
    DefaultFramebuffer,
};

struct AssetData {
};

void to_json(nlohmann::json& j, const std::shared_ptr<AssetData>& v, AssetType type);
void from_json(const nlohmann::json& j, std::shared_ptr<AssetData>& v, AssetType type);

enum class MeshAttributeElementSize {
    One,
    Two,
    Three,
    Four,
    BGRA,
};

enum class MeshAttributeElementType {
    Byte,
    UByte,
    Short,
    UShort,
    Int,
    UInt,
    HFloat,
    Float,
    Double,
    Fixed,
};

enum class MeshAttributeUsage {
    StreamDraw,
    StreamRead,
    StreamCopy,
    StaticDraw,
    StaticRead,
    StaticCopy,
    DynamicDraw,
    DynamicRead,
    DynamicCopy,
};

struct SimpleMeshAttribute {
    bool normalized;
    std::size_t size;
    unsigned int index;
    std::size_t stride;
    std::size_t offset;
    MeshAttributeUsage usage;
    std::vector<std::byte> data;
    MeshAttributeElementSize element_size;
    MeshAttributeElementType element_type;
};

struct MeshAsset : public AssetData {
    std::vector<std::array<float, 4>> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<std::array<float, 4>> texture_coords0;
    std::map<std::string, SimpleMeshAttribute> simple_attributes;

    static constexpr const char* vertices_json{ "vertices" };
    static constexpr const char* indices_json{ "indices" };
    static constexpr const char* texture_coords0_json{ "texture_coords0" };
    static constexpr const char* simple_attributes_json{ "simple_attributes" };
};

enum class TextureFormat { R, RG, RGB, RGBA, R8, RGBA16F };
enum class TextureDataType { Byte, Float, UInt, UInt8 };
enum class TextureAttributes { MagnificationLinear, MinificationLinear, GenerateMipMaps };

struct TextureFileAsset : public AssetData {
    TextureDataType data_type;
    std::filesystem::path path;
    std::vector<TextureAttributes> attributes;
};

struct TextureRawAsset : public AssetData {
    std::size_t width;
    std::size_t height;
    TextureFormat format;
    std::vector<TextureAttributes> attributes;
};

struct TextureMultisampleRawAsset : public AssetData {
    std::size_t width;
    std::size_t height;
    std::size_t samples;
    TextureFormat format;
    std::vector<TextureAttributes> attributes;
};

struct CuboidRenderPipelineAsset : public AssetData {
    std::size_t samples;
    std::size_t transparency_layers;
    std::array<std::size_t, 2> render_resolution;
};

enum class RenderbufferFormat { Depth24 };

struct RenderbufferAsset : public AssetData {
    std::size_t width;
    std::size_t height;
    std::size_t samples;
    RenderbufferFormat format;
};

struct ShaderAsset : public AssetData {
    std::filesystem::path vertex;
    std::filesystem::path fragment;
};

enum class FramebufferType { Texture, TextureMultisample, Renderbuffer };
enum class FramebufferDestination { Color0, Color1, Color2, Color3, Depth, Stencil, DepthStencil };

struct FramebufferAttachment {
    FramebufferType type;
    FramebufferDestination destination;
    std::string asset;
};

struct FramebufferAsset : public AssetData {
    std::vector<FramebufferAttachment> attachments;
    std::size_t viewportWidth;
    std::size_t viewportHeight;
    std::size_t viewportStartX;
    std::size_t viewportStartY;
};

struct DefaultFramebufferAsset : public AssetData {
};
}

/* STD Extensions */

namespace nlohmann {
template <> struct adl_serializer<std::filesystem::path> {
    static void to_json(nlohmann::json& j, const std::filesystem::path& v) { j = v.string(); }

    static void from_json(const nlohmann::json& j, std::filesystem::path& v) { v = j.get<std::string>(); }
};
}

namespace Visconfig::Assets {
/*Enums*/

NLOHMANN_JSON_SERIALIZE_ENUM(AssetType,
    {
        { AssetType::Mesh, "mesh" },
        { AssetType::TextureFile, "texture_file" },
        { AssetType::TextureRaw, "texture_raw" },
        { AssetType::TextureMultisampleRaw, "texture_multisample_raw" },
        { AssetType::CuboidRenderPipeline, "cuboid_render_pipeline" },
        { AssetType::Renderbuffer, "renderbuffer" },
        { AssetType::Shader, "shader" },
        { AssetType::Framebuffer, "framebuffer" },
        { AssetType::DefaultFramebuffer, "default_framebuffer" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(MeshAttributeElementSize,
    {
        { MeshAttributeElementSize::One, "1" },
        { MeshAttributeElementSize::Two, "2" },
        { MeshAttributeElementSize::Three, "3" },
        { MeshAttributeElementSize::Four, "4" },
        { MeshAttributeElementSize::BGRA, "BGRA" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(MeshAttributeElementType,
    {
        { MeshAttributeElementType::Byte, "byte" },
        { MeshAttributeElementType::UByte, "u_byte" },
        { MeshAttributeElementType::Short, "short" },
        { MeshAttributeElementType::UShort, "u_short" },
        { MeshAttributeElementType::Int, "int" },
        { MeshAttributeElementType::UInt, "u_int" },
        { MeshAttributeElementType::HFloat, "h_float" },
        { MeshAttributeElementType::Float, "float" },
        { MeshAttributeElementType::Double, "double" },
        { MeshAttributeElementType::Fixed, "fixed" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(MeshAttributeUsage,
    {
        { MeshAttributeUsage::StreamDraw, "stream_draw" },
        { MeshAttributeUsage::StreamRead, "stream_read" },
        { MeshAttributeUsage::StreamCopy, "stream_copy" },
        { MeshAttributeUsage::StaticDraw, "static_draw" },
        { MeshAttributeUsage::StaticRead, "static_read" },
        { MeshAttributeUsage::StaticCopy, "static_copy" },
        { MeshAttributeUsage::DynamicDraw, "dynamic_draw" },
        { MeshAttributeUsage::DynamicRead, "dynamic_read" },
        { MeshAttributeUsage::DynamicCopy, "dynamic_copy" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(TextureFormat,
    {
        { TextureFormat::R, "r" },
        { TextureFormat::RG, "rg" },
        { TextureFormat::RGB, "rgb" },
        { TextureFormat::RGBA, "rgba" },
        { TextureFormat::R8, "r8" },
        { TextureFormat::RGBA16F, "rgba16" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(TextureDataType,
    {
        { TextureDataType::Byte, "byte" },
        { TextureDataType::Float, "float" },
        { TextureDataType::UInt, "uint" },
        { TextureDataType::UInt8, "uint_8" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(TextureAttributes,
    {
        { TextureAttributes::MagnificationLinear, "magnification_linear" },
        { TextureAttributes::MinificationLinear, "minification_linear" },
        { TextureAttributes::GenerateMipMaps, "generate_mipmaps" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(RenderbufferFormat,
    {
        { RenderbufferFormat::Depth24, "depth_24" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(FramebufferType,
    {
        { FramebufferType::Texture, "texture" },
        { FramebufferType::TextureMultisample, "texture_multisample" },
        { FramebufferType::Renderbuffer, "renderbuffer" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(FramebufferDestination,
    {
        { FramebufferDestination::Color0, "color0" },
        { FramebufferDestination::Color1, "color1" },
        { FramebufferDestination::Color2, "color2" },
        { FramebufferDestination::Color3, "color3" },
        { FramebufferDestination::Depth, "depth" },
        { FramebufferDestination::Stencil, "stencil" },
        { FramebufferDestination::DepthStencil, "depth_stencil" },
    })

/*Internal Structs*/

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    SimpleMeshAttribute, normalized, size, index, stride, offset, usage, data, element_size, element_type)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FramebufferAttachment, type, destination, asset)

/*Assets*/

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MeshAsset, vertices, indices, texture_coords0, simple_attributes)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextureFileAsset, data_type, path, attributes)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextureRawAsset, width, height, format, attributes)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextureMultisampleRawAsset, width, height, samples, format, attributes)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CuboidRenderPipelineAsset, samples, transparency_layers, render_resolution)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderbufferAsset, width, height, samples, format)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShaderAsset, vertex, fragment)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    FramebufferAsset, attachments, viewportWidth, viewportHeight, viewportStartX, viewportStartY)

inline void to_json(nlohmann::json&, const DefaultFramebufferAsset&) {}
inline void from_json(const nlohmann::json&, DefaultFramebufferAsset&) {}

}