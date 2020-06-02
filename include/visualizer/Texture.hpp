#pragma once

#include <cstddef>
#include <glad/glad.h>
#include <memory>

namespace Visualizer {

enum class TextureSlot : std::size_t {
    Slot0 = 0,
    Slot1 = 1,
    Slot2 = 2,
    Slot3 = 3,
    Slot4 = 4,
    Slot5 = 5,
    Slot6 = 6,
    Slot7 = 7,
    Slot8 = 8,
    Slot9 = 9,
    Slot10 = 10,
    Slot11 = 11,
    Slot12 = 12,
    Slot13 = 13,
    Slot14 = 14,
    Slot15 = 15
};

enum class TextureType { Texture2D };
enum class TextureFormat { R, RG, RGB, RGBA, Depth, DepthStencil };
enum class TextureInternalFormat { Byte, Short, Int8, Int16, Int32, UInt8, UInt16, UInt32, Float16, Float32 };
enum class TextureMinificationFilter {
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};
enum class TextureMagnificationFilter { Nearest, Linear };

class Texture {
public:
    Texture(const Texture& other) = delete;
    virtual ~Texture() = default;

    Texture& operator=(const Texture& other) = delete;

    virtual GLuint id() const = 0;
    virtual TextureType type() const = 0;

    virtual void bind(TextureSlot slot) = 0;
    virtual void unbind(TextureSlot slot) = 0;

    virtual void addAttribute(TextureMinificationFilter filter) = 0;
    virtual void addAttribute(TextureMagnificationFilter filter) = 0;
    virtual void copyData(TextureFormat format, TextureInternalFormat internalFormat, GLint mipmapLevel, GLsizei width,
        GLsizei height, GLsizei border, const unsigned char* data)
        = 0;

protected:
    Texture() = default;
};

class Texture2D : public Texture {
public:
    Texture2D();
    Texture2D(const Texture2D& other) = delete;
    Texture2D(Texture2D&& other) noexcept;
    ~Texture2D();

    Texture2D& operator=(const Texture2D& other) = delete;
    Texture2D& operator=(Texture2D&& other) noexcept;

    GLuint id() const final;
    std::size_t width() const;
    std::size_t height() const;
    TextureType type() const final;

    void bind(TextureSlot slot) final;
    void unbind(TextureSlot slot) final;

    void addAttribute(TextureMinificationFilter filter) final;
    void addAttribute(TextureMagnificationFilter filter) final;
    void copyData(TextureFormat format, TextureInternalFormat internalFormat, GLint mipmapLevel, GLsizei width,
        GLsizei height, GLsizei border, const unsigned char* data) final;

private:
    GLuint m_id;
    std::size_t m_width;
    std::size_t m_height;
};

class TextureSampler {
public:
    TextureSampler(const std::shared_ptr<Texture>& texture, TextureSlot slot);
    TextureSampler(const TextureSampler& other) = default;
    TextureSampler(TextureSampler&& other) = default;

    TextureSampler& operator=(const TextureSampler& other) = default;
    TextureSampler& operator=(TextureSampler&& other) noexcept = default;

    TextureSlot slot() const;
    std::weak_ptr<Texture> texture();
    std::weak_ptr<const Texture> texture() const;

    void bind() const;
    void unbind() const;

private:
    TextureSlot m_slot;
    std::weak_ptr<Texture> m_texture;
};

}