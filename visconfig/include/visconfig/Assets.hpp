#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include <nlohmann/json.hpp>

namespace Visconfig::Assets {

enum class AssetType {
    Mesh,
    TextureFile,
    TextureRaw,
    TextureMultisampleRaw,
    Renderbuffer,
    Shader,
    Framebuffer,
    DefaultFramebuffer,
};

struct AssetData {
};

void to_json(nlohmann::json& j, const std::shared_ptr<AssetData>& v, AssetType type);
void from_json(const nlohmann::json& j, std::shared_ptr<AssetData>& v, AssetType type);

struct MeshAsset : public AssetData {
    std::vector<std::array<float, 4>> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<std::array<float, 4>> texture_coords0;

    static constexpr const char* verticesJson{ "vertices" };
    static constexpr const char* indicesJson{ "indices" };
    static constexpr const char* texture_coords0Json{ "texture_coords0" };
};

enum class TextureFormat { R, RG, RGB, RGBA, R8, RGBA16F };
enum class TextureAttributes { MagnificationLinear, MinificationLinear, GenerateMipMaps };

struct TextureFileAsset : public AssetData {
    std::filesystem::path path;
    std::vector<TextureAttributes> attributes;

    static constexpr const char* pathJson{ "path" };
    static constexpr const char* attributesJson{ "attributes" };
};

struct TextureRawAsset : public AssetData {
    std::size_t width;
    std::size_t height;
    TextureFormat format;
    std::vector<TextureAttributes> attributes;

    static constexpr const char* widthJson{ "width" };
    static constexpr const char* heightJson{ "height" };
    static constexpr const char* formatJson{ "format" };
    static constexpr const char* attributesJson{ "attributes" };
};

struct TextureMultisampleRawAsset : public AssetData {
    std::size_t width;
    std::size_t height;
    std::size_t samples;
    TextureFormat format;
    std::vector<TextureAttributes> attributes;

    static constexpr const char* widthJson{ "width" };
    static constexpr const char* heightJson{ "height" };
    static constexpr const char* samplesJson{ "samples" };
    static constexpr const char* formatJson{ "format" };
    static constexpr const char* attributesJson{ "attributes" };
};

enum class RenderbufferFormat { Depth24 };

struct RenderbufferAsset : public AssetData {
    std::size_t width;
    std::size_t height;
    std::size_t samples;
    RenderbufferFormat format;

    static constexpr const char* widthJson{ "width" };
    static constexpr const char* heightJson{ "height" };
    static constexpr const char* samplesJson{ "samples" };
    static constexpr const char* formatJson{ "format" };
};

struct ShaderAsset : public AssetData {
    std::filesystem::path vertex;
    std::filesystem::path fragment;

    static constexpr const char* vertexJson{ "vertex" };
    static constexpr const char* fragmentJson{ "fragment" };
};

enum class FramebufferType { Texture, TextureMultisample, Renderbuffer };
enum class FramebufferDestination { Color0, Color1, Color2, Color3, Depth, Stencil, DepthStencil };

struct FramebufferAttachment {
    FramebufferType type;
    FramebufferDestination destination;
    std::string asset;

    static constexpr const char* typeJson{ "type" };
    static constexpr const char* destinationJson{ "destination" };
    static constexpr const char* assetJson{ "asset" };
};

struct FramebufferAsset : public AssetData {
    std::vector<FramebufferAttachment> attachments;
    std::size_t viewportWidth;
    std::size_t viewportHeight;
    std::size_t viewportStartX;
    std::size_t viewportStartY;

    static constexpr const char* attachmentsJson{ "attachments" };
    static constexpr const char* viewportWidthJson{ "viewport_width" };
    static constexpr const char* viewportHeightJson{ "viewport_height" };
    static constexpr const char* viewportStartXJson{ "viewport_start_x" };
    static constexpr const char* viewportStartYJson{ "viewport_start_y" };
};

struct DefaultFramebufferAsset : public AssetData {
};

/*Enums*/

void to_json(nlohmann::json& j, const AssetType& v);
void from_json(const nlohmann::json& j, AssetType& v);

void to_json(nlohmann::json& j, const TextureFormat& v);
void from_json(const nlohmann::json& j, TextureFormat& v);

void to_json(nlohmann::json& j, const TextureAttributes& v);
void from_json(const nlohmann::json& j, TextureAttributes& v);

void to_json(nlohmann::json& j, const RenderbufferFormat& v);
void from_json(const nlohmann::json& j, RenderbufferFormat& v);

void to_json(nlohmann::json& j, const FramebufferType& v);
void from_json(const nlohmann::json& j, FramebufferType& v);

void to_json(nlohmann::json& j, const FramebufferDestination& v);
void from_json(const nlohmann::json& j, FramebufferDestination& v);

/*Assets*/

void to_json(nlohmann::json& j, const MeshAsset& v);
void from_json(const nlohmann::json& j, MeshAsset& v);

void to_json(nlohmann::json& j, const TextureFileAsset& v);
void from_json(const nlohmann::json& j, TextureFileAsset& v);

void to_json(nlohmann::json& j, const TextureRawAsset& v);
void from_json(const nlohmann::json& j, TextureRawAsset& v);

void to_json(nlohmann::json& j, const TextureMultisampleRawAsset& v);
void from_json(const nlohmann::json& j, TextureMultisampleRawAsset& v);

void to_json(nlohmann::json& j, const RenderbufferAsset& v);
void from_json(const nlohmann::json& j, RenderbufferAsset& v);

void to_json(nlohmann::json& j, const ShaderAsset& v);
void from_json(const nlohmann::json& j, ShaderAsset& v);

void to_json(nlohmann::json& j, const FramebufferAsset& v);
void from_json(const nlohmann::json& j, FramebufferAsset& v);

void to_json(nlohmann::json& j, const DefaultFramebufferAsset& v);
void from_json(const nlohmann::json& j, DefaultFramebufferAsset& v);

/*Internal Structs*/
void to_json(nlohmann::json& j, const FramebufferAttachment& v);
void from_json(const nlohmann::json& j, FramebufferAttachment& v);

}