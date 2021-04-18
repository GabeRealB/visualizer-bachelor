#include <visualizer/Texture.hpp>

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cassert>
#include <utility>

namespace Visualizer {

auto get_texture_slot_number_fn = [](TextureSlot slot) -> auto { return static_cast<std::size_t>(slot); };

auto get_texture_slot_enum_fn = [](TextureSlot slot) -> auto
{
    return static_cast<GLenum>(GL_TEXTURE0 + get_texture_slot_number_fn(slot));
};

auto get_texture_format_fn = [](TextureFormat format, TextureInternalFormat internal_format) {
    bool is_integer = false;
    bool depth_valid = false;
    bool depth_stencil_valid = false;

    switch (internal_format) {
    case TextureInternalFormat::Int8:
    case TextureInternalFormat::Int16:
    case TextureInternalFormat::Int32:
    case TextureInternalFormat::UInt8:
    case TextureInternalFormat::UInt16:
    case TextureInternalFormat::UInt32:
        is_integer = true;
    default:
        break;
    }

    switch (internal_format) {
    case TextureInternalFormat::Byte:
    case TextureInternalFormat::Short:
    case TextureInternalFormat::Int32:
    case TextureInternalFormat::UInt32:
    case TextureInternalFormat::Float32:
        depth_valid = true;
    default:
        break;
    }

    switch (internal_format) {
    case TextureInternalFormat::Int32:
    case TextureInternalFormat::UInt32:
    case TextureInternalFormat::Float32:
        depth_stencil_valid = true;
    default:
        break;
    }

    switch (format) {
    case TextureFormat::R:
        return is_integer ? GL_RED_INTEGER : GL_RED;
    case TextureFormat::RG:
        return is_integer ? GL_RG_INTEGER : GL_RG;
    case TextureFormat::RGB:
        return is_integer ? GL_RGB_INTEGER : GL_RGB;
    case TextureFormat::RGBA:
        return is_integer ? GL_RGBA_INTEGER : GL_RGBA;
    case TextureFormat::Depth:
        return depth_valid ? GL_DEPTH : GL_INVALID_ENUM;
    case TextureFormat::DepthStencil:
        return depth_stencil_valid ? GL_DEPTH_STENCIL : GL_INVALID_ENUM;
    }
};

auto get_texture_internal_format_fn = [](TextureFormat format, TextureInternalFormat internal_format) {
    switch (format) {
    case TextureFormat::R:
        switch (internal_format) {
        case TextureInternalFormat::Byte:
            return GL_R8;
        case TextureInternalFormat::Short:
            return GL_R16;
        case TextureInternalFormat::Int8:
            return GL_R8I;
        case TextureInternalFormat::Int16:
            return GL_R16I;
        case TextureInternalFormat::Int32:
            return GL_R32I;
        case TextureInternalFormat::UInt8:
            return GL_R8UI;
        case TextureInternalFormat::UInt16:
            return GL_R16UI;
        case TextureInternalFormat::UInt32:
            return GL_R32UI;
        case TextureInternalFormat::Float16:
            return GL_R16F;
        case TextureInternalFormat::Float32:
            return GL_R32F;
        }
    case TextureFormat::RG:
        switch (internal_format) {
        case TextureInternalFormat::Byte:
            return GL_RG8;
        case TextureInternalFormat::Short:
            return GL_RG16;
        case TextureInternalFormat::Int8:
            return GL_RG8I;
        case TextureInternalFormat::Int16:
            return GL_RG16I;
        case TextureInternalFormat::Int32:
            return GL_RG32I;
        case TextureInternalFormat::UInt8:
            return GL_RG8UI;
        case TextureInternalFormat::UInt16:
            return GL_RG16UI;
        case TextureInternalFormat::UInt32:
            return GL_RG32UI;
        case TextureInternalFormat::Float16:
            return GL_RG16F;
        case TextureInternalFormat::Float32:
            return GL_RG32F;
        }
    case TextureFormat::RGB:
        switch (internal_format) {
        case TextureInternalFormat::Byte:
            return GL_RGB8;
        case TextureInternalFormat::Short:
            return GL_RGB16;
        case TextureInternalFormat::Int8:
            return GL_RGB8I;
        case TextureInternalFormat::Int16:
            return GL_RGB16I;
        case TextureInternalFormat::Int32:
            return GL_RGB32I;
        case TextureInternalFormat::UInt8:
            return GL_RGB8UI;
        case TextureInternalFormat::UInt16:
            return GL_RGB16UI;
        case TextureInternalFormat::UInt32:
            return GL_RGB32UI;
        case TextureInternalFormat::Float16:
            return GL_RGB16F;
        case TextureInternalFormat::Float32:
            return GL_RGB32F;
        }
    case TextureFormat::RGBA:
        switch (internal_format) {
        case TextureInternalFormat::Byte:
            return GL_RGBA8;
        case TextureInternalFormat::Short:
            return GL_RGBA16;
        case TextureInternalFormat::Int8:
            return GL_RGBA8I;
        case TextureInternalFormat::Int16:
            return GL_RGBA16I;
        case TextureInternalFormat::Int32:
            return GL_RGBA32I;
        case TextureInternalFormat::UInt8:
            return GL_RGBA8UI;
        case TextureInternalFormat::UInt16:
            return GL_RGBA16UI;
        case TextureInternalFormat::UInt32:
            return GL_RGBA32UI;
        case TextureInternalFormat::Float16:
            return GL_RGBA16F;
        case TextureInternalFormat::Float32:
            return GL_RGBA32F;
        }
    case TextureFormat::Depth:
        switch (internal_format) {
        case TextureInternalFormat::Byte:
            return GL_DEPTH_COMPONENT16;
        case TextureInternalFormat::Short:
            return GL_DEPTH_COMPONENT24;
        case TextureInternalFormat::Int32:
            return GL_DEPTH_COMPONENT32;
        case TextureInternalFormat::UInt32:
            return GL_DEPTH_COMPONENT32;
        case TextureInternalFormat::Float32:
            return GL_DEPTH_COMPONENT32F;
        default:
            return GL_INVALID_ENUM;
        }
    case TextureFormat::DepthStencil:
        switch (internal_format) {
        case TextureInternalFormat::Int32:
            return GL_DEPTH24_STENCIL8;
        case TextureInternalFormat::UInt32:
            return GL_DEPTH24_STENCIL8;
        case TextureInternalFormat::Float32:
            return GL_DEPTH32F_STENCIL8;
        default:
            return GL_INVALID_ENUM;
        }
    }
};

TextureBuffer::TextureBuffer()
    : m_id{ 0 }
    , m_size{ 0 }
    , m_buffer{}
{
    assert(glGetError() == GL_NO_ERROR);
    glGenTextures(1, &m_id);
    assert(glGetError() == GL_NO_ERROR);
}

TextureBuffer::TextureBuffer(TextureBuffer&& other) noexcept
    : m_id{ std::exchange(other.m_id, 0) }
    , m_size{ std::exchange(other.m_size, 0) }
    , m_buffer{ std::exchange(other.m_buffer, {}) }
{
}

TextureBuffer::~TextureBuffer()
{
    if (m_id != 0) {
        assert(glGetError() == GL_NO_ERROR);
        glDeleteTextures(1, &m_id);
        assert(glGetError() == GL_NO_ERROR);
    }
}

TextureBuffer& TextureBuffer::operator=(TextureBuffer&& other) noexcept
{
    if (this != &other) {
        if (m_id != 0) {
            assert(glGetError() == GL_NO_ERROR);
            glDeleteTextures(1, &m_id);
            assert(glGetError() == GL_NO_ERROR);
        }

        m_id = std::exchange(other.m_id, 0);
        m_size = std::exchange(other.m_size, 0);
        m_buffer = std::exchange(other.m_buffer, {});
    }
    return *this;
}

GLuint TextureBuffer::id() const { return m_id; }

std::size_t TextureBuffer::size() const { return m_size; }

TextureType TextureBuffer::type() const { return TextureType::TextureBuffer; }

std::shared_ptr<GenericBuffer> TextureBuffer::buffer() { return m_buffer; }

std::shared_ptr<const GenericBuffer> TextureBuffer::buffer() const { return m_buffer; }

void TextureBuffer::bind(TextureSlot slot)
{
    if (m_id == 0) {
        return;
    }

    assert(glGetError() == GL_NO_ERROR);
    glActiveTexture(get_texture_slot_enum_fn(slot));

    glBindTexture(GL_TEXTURE_BUFFER, m_id);
    assert(glGetError() == GL_NO_ERROR);
}

void TextureBuffer::unbind(TextureSlot slot)
{
    if (m_id == 0) {
        return;
    }

    assert(glGetError() == GL_NO_ERROR);
    glActiveTexture(get_texture_slot_enum_fn(slot));

    glBindTexture(GL_TEXTURE_BUFFER, 0);
    assert(glGetError() == GL_NO_ERROR);
}

void TextureBuffer::bind_image(TextureSlot slot, int level, bool layered, int layer, GLenum access,
    TextureFormat format, TextureInternalFormat internal_format)
{
    auto texture_unit_gl = static_cast<GLuint>(get_texture_slot_number_fn(slot));
    auto texture_gl = static_cast<GLuint>(m_id);
    auto level_gl = static_cast<GLint>(level);
    auto layered_gl = static_cast<GLboolean>(layered);
    auto layer_gl = static_cast<GLint>(layer);
    auto format_gl = get_texture_internal_format_fn(format, internal_format);

    if (format_gl == GL_INVALID_ENUM) {
        return;
    }

    assert(glGetError() == GL_NO_ERROR);
    glBindImageTexture(texture_unit_gl, texture_gl, level_gl, layered_gl, layer_gl, access, format_gl);
    assert(glGetError() == GL_NO_ERROR);
}

void TextureBuffer::unbind_image(TextureSlot slot)
{
    auto texture_unit_gl = static_cast<GLuint>(get_texture_slot_number_fn(slot));

    assert(glGetError() == GL_NO_ERROR);
    glBindImageTexture(texture_unit_gl, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
    assert(glGetError() == GL_NO_ERROR);
}

void TextureBuffer::addAttribute(TextureMinificationFilter) {}

void TextureBuffer::addAttribute(TextureMagnificationFilter) {}

void TextureBuffer::bind_buffer(
    std::shared_ptr<GenericBuffer> buffer, TextureFormat format, TextureInternalFormat internal_format)
{
    bind(TextureSlot::TmpSlot);

    GLint internal_format_gl = get_texture_internal_format_fn(format, internal_format);
    if (internal_format_gl == GL_INVALID_ENUM) {
        return;
    }

    if (buffer) {
        m_size = buffer->size();
        buffer->bind();
        assert(glGetError() == GL_NO_ERROR);
        glTexBuffer(GL_TEXTURE_BUFFER, internal_format_gl, buffer->id());
        assert(glGetError() == GL_NO_ERROR);
        buffer->unbind();
    } else {
        m_size = 0;
        assert(glGetError() == GL_NO_ERROR);
        glTexBuffer(GL_TEXTURE_BUFFER, internal_format_gl, 0);
        assert(glGetError() == GL_NO_ERROR);
    }

    unbind(TextureSlot::TmpSlot);

    m_buffer = std::move(buffer);
}

Texture2D::Texture2D()
    : m_id{ 0 }
    , m_width{ 0 }
    , m_height{ 0 }
{
    assert(glGetError() == GL_NO_ERROR);
    glGenTextures(1, &m_id);
    assert(glGetError() == GL_NO_ERROR);
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : m_id{ std::exchange(other.m_id, 0) }
    , m_width{ std::exchange(other.m_width, 0) }
    , m_height{ std::exchange(other.m_height, 0) }
{
}

Texture2D::~Texture2D()
{
    if (m_id != 0) {
        assert(glGetError() == GL_NO_ERROR);
        glDeleteTextures(1, &m_id);
        assert(glGetError() == GL_NO_ERROR);
    }
}

std::shared_ptr<Texture2D> Texture2D::fromFile(
    const std::filesystem::path& filePath, TextureInternalFormat internal_format)
{
    if (!std::filesystem::exists(filePath)) {
        return nullptr;
    } else {
        int width;
        int height;
        int channels;
        // stbi_uc* data{ nullptr };

        auto deleter = [](stbi_uc* ptr) { stbi_image_free(ptr); };
        std::unique_ptr<stbi_uc, decltype(deleter)> data{ nullptr };

#ifdef _WIN32
        auto utf8_path{ filePath.string() };
        data = std::unique_ptr<stbi_uc, decltype(deleter)>{ stbi_load(utf8_path.c_str(), &width, &height, &channels, 0),
            deleter };
#else
        data = std::unique_ptr<stbi_uc, decltype(deleter)>{ stbi_load(filePath.c_str(), &width, &height, &channels, 0),
            deleter };
#endif

        if (data == nullptr) {
            return nullptr;
        }

        auto texture{ std::make_shared<Texture2D>() };

        switch (channels) {
        case 1:
            texture->copyData(TextureFormat::R, internal_format, 0, width, height, 0, data.get());
            break;
        case 2:
            texture->copyData(TextureFormat::RG, internal_format, 0, width, height, 0, data.get());
            break;
        case 3:
            texture->copyData(TextureFormat::RGB, internal_format, 0, width, height, 0, data.get());
            break;
        case 4:
            texture->copyData(TextureFormat::RGBA, internal_format, 0, width, height, 0, data.get());
            break;
        default:
            return nullptr;
        }

        return texture;
    }
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
{
    if (this != &other) {
        if (m_id != 0) {
            assert(glGetError() == GL_NO_ERROR);
            glDeleteTextures(1, &m_id);
            assert(glGetError() == GL_NO_ERROR);
        }

        m_id = std::exchange(other.m_id, 0);
        m_width = std::exchange(other.m_width, 0);
        m_height = std::exchange(other.m_height, 0);
    }
    return *this;
}

GLuint Texture2D::id() const { return m_id; }

std::size_t Texture2D::width() const { return m_width; }

std::size_t Texture2D::height() const { return m_height; }

TextureType Texture2D::type() const { return TextureType::Texture2D; }

void Texture2D::bind(TextureSlot slot)
{
    if (m_id == 0) {
        return;
    }

    assert(glGetError() == GL_NO_ERROR);
    glActiveTexture(get_texture_slot_enum_fn(slot));

    glBindTexture(GL_TEXTURE_2D, m_id);
    assert(glGetError() == GL_NO_ERROR);
}

void Texture2D::unbind(TextureSlot slot)
{
    if (m_id == 0) {
        return;
    }

    assert(glGetError() == GL_NO_ERROR);
    glActiveTexture(get_texture_slot_enum_fn(slot));

    glBindTexture(GL_TEXTURE_2D, 0);
    assert(glGetError() == GL_NO_ERROR);
}

void Texture2D::bind_image(TextureSlot slot, int level, bool layered, int layer, GLenum access,
    TextureFormat format, TextureInternalFormat internal_format)
{
    auto texture_unit_gl = static_cast<GLuint>(get_texture_slot_number_fn(slot));
    auto texture_gl = static_cast<GLuint>(m_id);
    auto level_gl = static_cast<GLint>(level);
    auto layered_gl = static_cast<GLboolean>(layered);
    auto layer_gl = static_cast<GLint>(layer);
    auto format_gl = get_texture_internal_format_fn(format, internal_format);

    if (format_gl == GL_INVALID_ENUM) {
        return;
    }

    assert(glGetError() == GL_NO_ERROR);
    glBindImageTexture(texture_unit_gl, texture_gl, level_gl, layered_gl, layer_gl, access, format_gl);
    assert(glGetError() == GL_NO_ERROR);
}

void Texture2D::unbind_image(TextureSlot slot)
{
    auto texture_unit_gl = static_cast<GLuint>(get_texture_slot_number_fn(slot));

    assert(glGetError() == GL_NO_ERROR);
    glBindImageTexture(texture_unit_gl, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
    assert(glGetError() == GL_NO_ERROR);
}

void Texture2D::addAttribute(TextureMinificationFilter filter)
{
    bind(TextureSlot::TmpSlot);
    assert(glGetError() == GL_NO_ERROR);
    switch (filter) {
    case TextureMinificationFilter::Nearest:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    case TextureMinificationFilter::Linear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case TextureMinificationFilter::NearestMipmapNearest:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        break;
    case TextureMinificationFilter::LinearMipmapNearest:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        break;
    case TextureMinificationFilter::NearestMipmapLinear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        break;
    case TextureMinificationFilter::LinearMipmapLinear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        break;
    default:
        break;
    }
    assert(glGetError() == GL_NO_ERROR);
    unbind(TextureSlot::TmpSlot);
}

void Texture2D::addAttribute(TextureMagnificationFilter filter)
{
    bind(TextureSlot::TmpSlot);
    assert(glGetError() == GL_NO_ERROR);
    switch (filter) {
    case TextureMagnificationFilter::Nearest:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case TextureMagnificationFilter::Linear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    default:
        break;
    }
    assert(glGetError() == GL_NO_ERROR);
    unbind(TextureSlot::TmpSlot);
}

void Texture2D::copyData(TextureFormat format, TextureInternalFormat internalFormat, GLint mipmapLevel, GLsizei width,
    GLsizei height, GLsizei border, const unsigned char* data)
{
    GLenum formatGl = get_texture_format_fn(format, internalFormat);
    GLint internalFormatGl = get_texture_internal_format_fn(format, internalFormat);
    if (formatGl == GL_INVALID_ENUM || internalFormatGl == GL_INVALID_ENUM) {
        return;
    }

    m_width = width;
    m_height = height;

    bind(TextureSlot::TmpSlot);
    assert(glGetError() == GL_NO_ERROR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, internalFormatGl, width, height, border, formatGl, GL_UNSIGNED_BYTE, data);
    assert(glGetError() == GL_NO_ERROR);
    unbind(TextureSlot::TmpSlot);
}

Texture2DMultisample::Texture2DMultisample()
    : m_id{ 0 }
    , m_samples{ 0 }
    , m_width{ 0 }
    , m_height{ 0 }
{
    assert(glGetError() == GL_NO_ERROR);
    glGenTextures(1, &m_id);
    assert(glGetError() == GL_NO_ERROR);
}

Texture2DMultisample::Texture2DMultisample(Texture2DMultisample&& other) noexcept
    : m_id{ std::exchange(other.m_id, 0) }
    , m_samples{ std::exchange(other.m_samples, 0) }
    , m_width{ std::exchange(other.m_width, 0) }
    , m_height{ std::exchange(other.m_height, 0) }
{
}

Texture2DMultisample::~Texture2DMultisample()
{
    if (m_id != 0) {
        assert(glGetError() == GL_NO_ERROR);
        glDeleteTextures(1, &m_id);
        assert(glGetError() == GL_NO_ERROR);
    }
}

Texture2DMultisample& Texture2DMultisample::operator=(Texture2DMultisample&& other) noexcept
{
    if (this != &other) {
        if (m_id != 0) {
            assert(glGetError() == GL_NO_ERROR);
            glDeleteTextures(1, &m_id);
            assert(glGetError() == GL_NO_ERROR);
        }

        m_id = std::exchange(other.m_id, 0);
        m_samples = std::exchange(other.m_samples, 0);
        m_width = std::exchange(other.m_width, 0);
        m_height = std::exchange(other.m_height, 0);
    }
    return *this;
}

GLuint Texture2DMultisample::id() const { return m_id; }

GLsizei Texture2DMultisample::samples() const { return m_samples; }

std::size_t Texture2DMultisample::width() const { return m_width; }

std::size_t Texture2DMultisample::height() const { return m_height; }

TextureType Texture2DMultisample::type() const { return TextureType::Texture2DMultisample; }

void Texture2DMultisample::bind(TextureSlot slot)
{
    if (m_id == 0) {
        return;
    }

    assert(glGetError() == GL_NO_ERROR);
    glActiveTexture(get_texture_slot_enum_fn(slot));

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_id);
    assert(glGetError() == GL_NO_ERROR);
}

void Texture2DMultisample::unbind(TextureSlot slot)
{
    if (m_id == 0) {
        return;
    }

    assert(glGetError() == GL_NO_ERROR);
    glActiveTexture(get_texture_slot_enum_fn(slot));

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    assert(glGetError() == GL_NO_ERROR);
}

void Texture2DMultisample::bind_image(TextureSlot slot, int level, bool layered, int layer, GLenum access,
    TextureFormat format, TextureInternalFormat internal_format)
{
    auto texture_unit_gl = static_cast<GLuint>(get_texture_slot_number_fn(slot));
    auto texture_gl = static_cast<GLuint>(m_id);
    auto level_gl = static_cast<GLint>(level);
    auto layered_gl = static_cast<GLboolean>(layered);
    auto layer_gl = static_cast<GLint>(layer);
    auto format_gl = get_texture_internal_format_fn(format, internal_format);

    if (format_gl == GL_INVALID_ENUM) {
        return;
    }

    assert(glGetError() == GL_NO_ERROR);
    glBindImageTexture(texture_unit_gl, texture_gl, level_gl, layered_gl, layer_gl, access, format_gl);
    assert(glGetError() == GL_NO_ERROR);
}

void Texture2DMultisample::unbind_image(TextureSlot slot)
{
    auto texture_unit_gl = static_cast<GLuint>(get_texture_slot_number_fn(slot));

    assert(glGetError() == GL_NO_ERROR);
    glBindImageTexture(texture_unit_gl, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
    assert(glGetError() == GL_NO_ERROR);
}

void Texture2DMultisample::addAttribute(TextureMinificationFilter filter)
{
    bind(TextureSlot::TmpSlot);
    assert(glGetError() == GL_NO_ERROR);
    switch (filter) {
    case TextureMinificationFilter::Nearest:
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    case TextureMinificationFilter::Linear:
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case TextureMinificationFilter::NearestMipmapNearest:
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        break;
    case TextureMinificationFilter::LinearMipmapNearest:
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        break;
    case TextureMinificationFilter::NearestMipmapLinear:
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        break;
    case TextureMinificationFilter::LinearMipmapLinear:
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        break;
    default:
        break;
    }
    assert(glGetError() == GL_NO_ERROR);
    unbind(TextureSlot::TmpSlot);
}

void Texture2DMultisample::addAttribute(TextureMagnificationFilter filter)
{
    bind(TextureSlot::TmpSlot);
    assert(glGetError() == GL_NO_ERROR);
    switch (filter) {
    case TextureMagnificationFilter::Nearest:
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case TextureMagnificationFilter::Linear:
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    default:
        break;
    }
    assert(glGetError() == GL_NO_ERROR);
    unbind(TextureSlot::TmpSlot);
}

void Texture2DMultisample::initialize(
    TextureFormat format, TextureInternalFormat internalFormat, GLsizei samples, GLsizei width, GLsizei height)
{
    GLint internalFormatGl = get_texture_internal_format_fn(format, internalFormat);
    if (internalFormatGl == GL_INVALID_ENUM) {
        return;
    }

    GLint max_samples;
    glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

    m_samples = samples = samples < max_samples ? samples : max_samples;
    m_width = width;
    m_height = height;

    bind(TextureSlot::TmpSlot);
    assert(glGetError() == GL_NO_ERROR);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormatGl, width, height, GL_TRUE);
    assert(glGetError() == GL_NO_ERROR);
    unbind(TextureSlot::TmpSlot);
}

}