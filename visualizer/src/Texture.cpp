#include <visualizer/Texture.hpp>

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <utility>

namespace Visualizer {

Texture2D::Texture2D()
    : m_id{ 0 }
    , m_width{ 0 }
    , m_height{ 0 }
{
    glGenTextures(1, &m_id);
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
        glDeleteTextures(1, &m_id);
    }
}

std::shared_ptr<Texture2D> Texture2D::fromFile(const std::filesystem::path& filePath)
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
            texture->copyData(TextureFormat::R, TextureInternalFormat::Byte, 0, width, height, 0, data.get());
            break;
        case 2:
            texture->copyData(TextureFormat::RG, TextureInternalFormat::Byte, 0, width, height, 0, data.get());
            break;
        case 3:
            texture->copyData(TextureFormat::RGB, TextureInternalFormat::Byte, 0, width, height, 0, data.get());
            break;
        case 4:
            texture->copyData(TextureFormat::RGBA, TextureInternalFormat::Byte, 0, width, height, 0, data.get());
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
            glDeleteTextures(1, &m_id);
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

    switch (slot) {
    case TextureSlot::Slot0:
        glActiveTexture(GL_TEXTURE0);
        break;
    case TextureSlot::Slot1:
        glActiveTexture(GL_TEXTURE1);
        break;
    case TextureSlot::Slot2:
        glActiveTexture(GL_TEXTURE2);
        break;
    case TextureSlot::Slot3:
        glActiveTexture(GL_TEXTURE3);
        break;
    case TextureSlot::Slot4:
        glActiveTexture(GL_TEXTURE4);
        break;
    case TextureSlot::Slot5:
        glActiveTexture(GL_TEXTURE5);
        break;
    case TextureSlot::Slot6:
        glActiveTexture(GL_TEXTURE6);
        break;
    case TextureSlot::Slot7:
        glActiveTexture(GL_TEXTURE7);
        break;
    case TextureSlot::Slot8:
        glActiveTexture(GL_TEXTURE8);
        break;
    case TextureSlot::Slot9:
        glActiveTexture(GL_TEXTURE9);
        break;
    case TextureSlot::Slot10:
        glActiveTexture(GL_TEXTURE10);
        break;
    case TextureSlot::Slot11:
        glActiveTexture(GL_TEXTURE11);
        break;
    case TextureSlot::Slot12:
        glActiveTexture(GL_TEXTURE12);
        break;
    case TextureSlot::Slot13:
        glActiveTexture(GL_TEXTURE13);
        break;
    case TextureSlot::Slot14:
        glActiveTexture(GL_TEXTURE14);
        break;
    case TextureSlot::Slot15:
        glActiveTexture(GL_TEXTURE15);
        break;
    default:
        return;
    }

    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture2D::unbind(TextureSlot slot)
{
    if (m_id == 0) {
        return;
    }

    switch (slot) {
    case TextureSlot::Slot0:
        glActiveTexture(GL_TEXTURE0);
        break;
    case TextureSlot::Slot1:
        glActiveTexture(GL_TEXTURE1);
        break;
    case TextureSlot::Slot2:
        glActiveTexture(GL_TEXTURE2);
        break;
    case TextureSlot::Slot3:
        glActiveTexture(GL_TEXTURE3);
        break;
    case TextureSlot::Slot4:
        glActiveTexture(GL_TEXTURE4);
        break;
    case TextureSlot::Slot5:
        glActiveTexture(GL_TEXTURE5);
        break;
    case TextureSlot::Slot6:
        glActiveTexture(GL_TEXTURE6);
        break;
    case TextureSlot::Slot7:
        glActiveTexture(GL_TEXTURE7);
        break;
    case TextureSlot::Slot8:
        glActiveTexture(GL_TEXTURE8);
        break;
    case TextureSlot::Slot9:
        glActiveTexture(GL_TEXTURE9);
        break;
    case TextureSlot::Slot10:
        glActiveTexture(GL_TEXTURE10);
        break;
    case TextureSlot::Slot11:
        glActiveTexture(GL_TEXTURE11);
        break;
    case TextureSlot::Slot12:
        glActiveTexture(GL_TEXTURE12);
        break;
    case TextureSlot::Slot13:
        glActiveTexture(GL_TEXTURE13);
        break;
    case TextureSlot::Slot14:
        glActiveTexture(GL_TEXTURE14);
        break;
    case TextureSlot::Slot15:
        glActiveTexture(GL_TEXTURE15);
        break;
    default:
        return;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::addAttribute(TextureMinificationFilter filter)
{
    bind(TextureSlot::TmpSlot);
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
    unbind(TextureSlot::TmpSlot);
}

void Texture2D::addAttribute(TextureMagnificationFilter filter)
{
    bind(TextureSlot::TmpSlot);
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
    unbind(TextureSlot::TmpSlot);
}

void Texture2D::copyData(TextureFormat format, TextureInternalFormat internalFormat, GLint mipmapLevel, GLsizei width,
    GLsizei height, GLsizei border, const unsigned char* data)
{
    GLenum formatGl;
    GLint internalFormatGl;

    switch (format) {
    case TextureFormat::R:
        formatGl = GL_RED;
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_R8;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_R16;
            break;
        case TextureInternalFormat::Int8:
            internalFormatGl = GL_R8I;
            break;
        case TextureInternalFormat::Int16:
            internalFormatGl = GL_R16I;
            break;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_R32I;
            break;
        case TextureInternalFormat::UInt8:
            internalFormatGl = GL_R8UI;
            break;
        case TextureInternalFormat::UInt16:
            internalFormatGl = GL_R16UI;
            break;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_R32UI;
            break;
        case TextureInternalFormat::Float16:
            internalFormatGl = GL_R16F;
            break;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_R32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::RG:
        formatGl = GL_RG;
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_RG8;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_RG16;
            break;
        case TextureInternalFormat::Int8:
            internalFormatGl = GL_RG8I;
            break;
        case TextureInternalFormat::Int16:
            internalFormatGl = GL_RG16I;
            break;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_RG32I;
            break;
        case TextureInternalFormat::UInt8:
            internalFormatGl = GL_RG8UI;
            break;
        case TextureInternalFormat::UInt16:
            internalFormatGl = GL_RG16UI;
            break;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_RG32UI;
            break;
        case TextureInternalFormat::Float16:
            internalFormatGl = GL_RG16F;
            break;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_RG32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::RGB:
        formatGl = GL_RGB;
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_RGB8;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_RGB16;
            break;
        case TextureInternalFormat::Int8:
            internalFormatGl = GL_RGB8I;
            break;
        case TextureInternalFormat::Int16:
            internalFormatGl = GL_RGB16I;
            break;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_RGB32I;
            break;
        case TextureInternalFormat::UInt8:
            internalFormatGl = GL_RGB8UI;
            break;
        case TextureInternalFormat::UInt16:
            internalFormatGl = GL_RGB16UI;
            break;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_RGB32UI;
            break;
        case TextureInternalFormat::Float16:
            internalFormatGl = GL_RGB16F;
            break;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_RGB32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::RGBA:
        formatGl = GL_RGBA;
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_RGBA8;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_RGBA16;
            break;
        case TextureInternalFormat::Int8:
            internalFormatGl = GL_RGBA8I;
            break;
        case TextureInternalFormat::Int16:
            internalFormatGl = GL_RGBA16I;
            break;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_RGBA32I;
            break;
        case TextureInternalFormat::UInt8:
            internalFormatGl = GL_RGBA8UI;
            break;
        case TextureInternalFormat::UInt16:
            internalFormatGl = GL_RGBA16UI;
            break;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_RGBA32UI;
            break;
        case TextureInternalFormat::Float16:
            internalFormatGl = GL_RGBA16F;
            break;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_RGBA32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::Depth:
        formatGl = GL_DEPTH;
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_DEPTH_COMPONENT16;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_DEPTH_COMPONENT24;
            break;
        case TextureInternalFormat::Int8:
            return;
        case TextureInternalFormat::Int16:
            return;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_DEPTH_COMPONENT32;
            break;
        case TextureInternalFormat::UInt8:
            return;
        case TextureInternalFormat::UInt16:
            return;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_DEPTH_COMPONENT32;
            break;
        case TextureInternalFormat::Float16:
            return;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_DEPTH_COMPONENT32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::DepthStencil:
        formatGl = GL_DEPTH_STENCIL;
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            return;
        case TextureInternalFormat::Short:
            return;
        case TextureInternalFormat::Int8:
            return;
        case TextureInternalFormat::Int16:
            return;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_DEPTH24_STENCIL8;
            break;
        case TextureInternalFormat::UInt8:
            return;
        case TextureInternalFormat::UInt16:
            return;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_DEPTH24_STENCIL8;
            break;
        case TextureInternalFormat::Float16:
            return;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_DEPTH32F_STENCIL8;
            break;
        default:
            return;
        }
        break;
    default:
        return;
    }

    m_width = width;
    m_height = height;

    bind(TextureSlot::TmpSlot);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, internalFormatGl, width, height, border, formatGl, GL_UNSIGNED_BYTE, data);
    unbind(TextureSlot::TmpSlot);
}

Texture2DMultisample::Texture2DMultisample()
    : m_id{ 0 }
    , m_samples{ 0 }
    , m_width{ 0 }
    , m_height{ 0 }
{
    glGenTextures(1, &m_id);
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
        glDeleteTextures(1, &m_id);
    }
}

Texture2DMultisample& Texture2DMultisample::operator=(Texture2DMultisample&& other) noexcept
{
    if (this != &other) {
        if (m_id != 0) {
            glDeleteTextures(1, &m_id);
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

    switch (slot) {
    case TextureSlot::Slot0:
        glActiveTexture(GL_TEXTURE0);
        break;
    case TextureSlot::Slot1:
        glActiveTexture(GL_TEXTURE1);
        break;
    case TextureSlot::Slot2:
        glActiveTexture(GL_TEXTURE2);
        break;
    case TextureSlot::Slot3:
        glActiveTexture(GL_TEXTURE3);
        break;
    case TextureSlot::Slot4:
        glActiveTexture(GL_TEXTURE4);
        break;
    case TextureSlot::Slot5:
        glActiveTexture(GL_TEXTURE5);
        break;
    case TextureSlot::Slot6:
        glActiveTexture(GL_TEXTURE6);
        break;
    case TextureSlot::Slot7:
        glActiveTexture(GL_TEXTURE7);
        break;
    case TextureSlot::Slot8:
        glActiveTexture(GL_TEXTURE8);
        break;
    case TextureSlot::Slot9:
        glActiveTexture(GL_TEXTURE9);
        break;
    case TextureSlot::Slot10:
        glActiveTexture(GL_TEXTURE10);
        break;
    case TextureSlot::Slot11:
        glActiveTexture(GL_TEXTURE11);
        break;
    case TextureSlot::Slot12:
        glActiveTexture(GL_TEXTURE12);
        break;
    case TextureSlot::Slot13:
        glActiveTexture(GL_TEXTURE13);
        break;
    case TextureSlot::Slot14:
        glActiveTexture(GL_TEXTURE14);
        break;
    case TextureSlot::Slot15:
        glActiveTexture(GL_TEXTURE15);
        break;
    default:
        return;
    }

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_id);
}

void Texture2DMultisample::unbind(TextureSlot slot)
{
    if (m_id == 0) {
        return;
    }

    switch (slot) {
    case TextureSlot::Slot0:
        glActiveTexture(GL_TEXTURE0);
        break;
    case TextureSlot::Slot1:
        glActiveTexture(GL_TEXTURE1);
        break;
    case TextureSlot::Slot2:
        glActiveTexture(GL_TEXTURE2);
        break;
    case TextureSlot::Slot3:
        glActiveTexture(GL_TEXTURE3);
        break;
    case TextureSlot::Slot4:
        glActiveTexture(GL_TEXTURE4);
        break;
    case TextureSlot::Slot5:
        glActiveTexture(GL_TEXTURE5);
        break;
    case TextureSlot::Slot6:
        glActiveTexture(GL_TEXTURE6);
        break;
    case TextureSlot::Slot7:
        glActiveTexture(GL_TEXTURE7);
        break;
    case TextureSlot::Slot8:
        glActiveTexture(GL_TEXTURE8);
        break;
    case TextureSlot::Slot9:
        glActiveTexture(GL_TEXTURE9);
        break;
    case TextureSlot::Slot10:
        glActiveTexture(GL_TEXTURE10);
        break;
    case TextureSlot::Slot11:
        glActiveTexture(GL_TEXTURE11);
        break;
    case TextureSlot::Slot12:
        glActiveTexture(GL_TEXTURE12);
        break;
    case TextureSlot::Slot13:
        glActiveTexture(GL_TEXTURE13);
        break;
    case TextureSlot::Slot14:
        glActiveTexture(GL_TEXTURE14);
        break;
    case TextureSlot::Slot15:
        glActiveTexture(GL_TEXTURE15);
        break;
    default:
        return;
    }

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void Texture2DMultisample::addAttribute(TextureMinificationFilter filter)
{
    bind(TextureSlot::TmpSlot);
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
    unbind(TextureSlot::TmpSlot);
}

void Texture2DMultisample::addAttribute(TextureMagnificationFilter filter)
{
    bind(TextureSlot::TmpSlot);
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
    unbind(TextureSlot::TmpSlot);
}

void Texture2DMultisample::initialize(
    TextureFormat format, TextureInternalFormat internalFormat, GLsizei samples, GLsizei width, GLsizei height)
{
    GLint internalFormatGl;

    switch (format) {
    case TextureFormat::R:
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_R8;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_R16;
            break;
        case TextureInternalFormat::Int8:
            internalFormatGl = GL_R8I;
            break;
        case TextureInternalFormat::Int16:
            internalFormatGl = GL_R16I;
            break;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_R32I;
            break;
        case TextureInternalFormat::UInt8:
            internalFormatGl = GL_R8UI;
            break;
        case TextureInternalFormat::UInt16:
            internalFormatGl = GL_R16UI;
            break;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_R32UI;
            break;
        case TextureInternalFormat::Float16:
            internalFormatGl = GL_R16F;
            break;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_R32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::RG:
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_RG8;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_RG16;
            break;
        case TextureInternalFormat::Int8:
            internalFormatGl = GL_RG8I;
            break;
        case TextureInternalFormat::Int16:
            internalFormatGl = GL_RG16I;
            break;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_RG32I;
            break;
        case TextureInternalFormat::UInt8:
            internalFormatGl = GL_RG8UI;
            break;
        case TextureInternalFormat::UInt16:
            internalFormatGl = GL_RG16UI;
            break;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_RG32UI;
            break;
        case TextureInternalFormat::Float16:
            internalFormatGl = GL_RG16F;
            break;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_RG32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::RGB:
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_RGB8;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_RGB16;
            break;
        case TextureInternalFormat::Int8:
            internalFormatGl = GL_RGB8I;
            break;
        case TextureInternalFormat::Int16:
            internalFormatGl = GL_RGB16I;
            break;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_RGB32I;
            break;
        case TextureInternalFormat::UInt8:
            internalFormatGl = GL_RGB8UI;
            break;
        case TextureInternalFormat::UInt16:
            internalFormatGl = GL_RGB16UI;
            break;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_RGB32UI;
            break;
        case TextureInternalFormat::Float16:
            internalFormatGl = GL_RGB16F;
            break;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_RGB32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::RGBA:
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_RGBA8;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_RGBA16;
            break;
        case TextureInternalFormat::Int8:
            internalFormatGl = GL_RGBA8I;
            break;
        case TextureInternalFormat::Int16:
            internalFormatGl = GL_RGBA16I;
            break;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_RGBA32I;
            break;
        case TextureInternalFormat::UInt8:
            internalFormatGl = GL_RGBA8UI;
            break;
        case TextureInternalFormat::UInt16:
            internalFormatGl = GL_RGBA16UI;
            break;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_RGBA32UI;
            break;
        case TextureInternalFormat::Float16:
            internalFormatGl = GL_RGBA16F;
            break;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_RGBA32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::Depth:
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            internalFormatGl = GL_DEPTH_COMPONENT16;
            break;
        case TextureInternalFormat::Short:
            internalFormatGl = GL_DEPTH_COMPONENT24;
            break;
        case TextureInternalFormat::Int8:
            return;
        case TextureInternalFormat::Int16:
            return;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_DEPTH_COMPONENT32;
            break;
        case TextureInternalFormat::UInt8:
            return;
        case TextureInternalFormat::UInt16:
            return;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_DEPTH_COMPONENT32;
            break;
        case TextureInternalFormat::Float16:
            return;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_DEPTH_COMPONENT32F;
            break;
        default:
            return;
        }
        break;
    case TextureFormat::DepthStencil:
        switch (internalFormat) {
        case TextureInternalFormat::Byte:
            return;
        case TextureInternalFormat::Short:
            return;
        case TextureInternalFormat::Int8:
            return;
        case TextureInternalFormat::Int16:
            return;
        case TextureInternalFormat::Int32:
            internalFormatGl = GL_DEPTH24_STENCIL8;
            break;
        case TextureInternalFormat::UInt8:
            return;
        case TextureInternalFormat::UInt16:
            return;
        case TextureInternalFormat::UInt32:
            internalFormatGl = GL_DEPTH24_STENCIL8;
            break;
        case TextureInternalFormat::Float16:
            return;
        case TextureInternalFormat::Float32:
            internalFormatGl = GL_DEPTH32F_STENCIL8;
            break;
        default:
            return;
        }
        break;
    default:
        return;
    }

    m_samples = samples;
    m_width = width;
    m_height = height;

    bind(TextureSlot::TmpSlot);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormatGl, width, height, GL_TRUE);
    unbind(TextureSlot::TmpSlot);
}

}