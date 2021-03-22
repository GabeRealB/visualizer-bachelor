#pragma once

#include <concepts>
#include <cstddef>
#include <filesystem>
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
    Slot15 = 15,
    TmpSlot = 15
};

enum class TextureType { Texture2D, Texture2DMultisample };
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

protected:
    Texture() = default;
};

class Texture2D : public Texture {
public:
    Texture2D();
    Texture2D(const Texture2D& other) = delete;
    Texture2D(Texture2D&& other) noexcept;
    ~Texture2D();

    static std::shared_ptr<Texture2D> fromFile(
        const std::filesystem::path& filePath, TextureInternalFormat internal_format = TextureInternalFormat::Byte);

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
        GLsizei height, GLsizei border, const unsigned char* data);

private:
    GLuint m_id;
    std::size_t m_width;
    std::size_t m_height;
};

class Texture2DMultisample : public Texture {
public:
    Texture2DMultisample();
    Texture2DMultisample(const Texture2DMultisample& other) = delete;
    Texture2DMultisample(Texture2DMultisample&& other) noexcept;
    ~Texture2DMultisample();

    Texture2DMultisample& operator=(const Texture2DMultisample& other) = delete;
    Texture2DMultisample& operator=(Texture2DMultisample&& other) noexcept;

    GLuint id() const final;
    GLsizei samples() const;
    std::size_t width() const;
    std::size_t height() const;
    TextureType type() const final;

    void bind(TextureSlot slot) final;
    void unbind(TextureSlot slot) final;

    void addAttribute(TextureMinificationFilter filter) final;
    void addAttribute(TextureMagnificationFilter filter) final;
    void initialize(
        TextureFormat format, TextureInternalFormat internalFormat, GLsizei samples, GLsizei width, GLsizei height);

private:
    GLuint m_id;
    GLsizei m_samples;
    std::size_t m_width;
    std::size_t m_height;
};

template <typename T>
requires std::derived_from<T, Texture>
class TextureSampler {
public:
    TextureSampler(const std::shared_ptr<T>& texture, TextureSlot slot)
        : m_slot{ slot }
        , m_texture{ texture }
    {
    }

    TextureSampler(const TextureSampler<T>& other) = default;
    TextureSampler(TextureSampler<T>&& other) noexcept = default;

    TextureSampler<T>& operator=(const TextureSampler<T>& other) = default;
    TextureSampler<T>& operator=(TextureSampler<T>&& other) noexcept = default;

    TextureSlot slot() const { return m_slot; }
    std::weak_ptr<T> texture() { return m_texture; }
    std::weak_ptr<const T> texture() const { return m_texture; }

    void bind() const
    {
        if (auto tex{ m_texture.lock() }) {
            tex->bind(m_slot);
        }
    }

    void unbind() const
    {
        if (auto tex{ m_texture.lock() }) {
            tex->unbind(m_slot);
        }
    }

private:
    TextureSlot m_slot;
    std::weak_ptr<T> m_texture;
};

}