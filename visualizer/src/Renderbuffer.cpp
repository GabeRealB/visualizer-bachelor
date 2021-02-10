#include <visualizer/Renderbuffer.hpp>

#include <cassert>
#include <utility>

namespace Visualizer {

Renderbuffer::Renderbuffer()
    : m_id{ 0 }
    , m_samples{ 0 }
{
    assert(glGetError() == GL_NO_ERROR);
    glGenRenderbuffers(1, &m_id);
    assert(glGetError() == GL_NO_ERROR);
}

Renderbuffer::Renderbuffer(Renderbuffer&& other) noexcept
    : m_id{ std::exchange(other.m_id, 0) }
    , m_samples{ std::exchange(other.m_samples, 0) }
{
}

Renderbuffer::~Renderbuffer()
{
    if (m_id != 0) {
        assert(glGetError() == GL_NO_ERROR);
        glDeleteRenderbuffers(1, &m_id);
        assert(glGetError() == GL_NO_ERROR);
    }
}

Renderbuffer& Renderbuffer::operator=(Renderbuffer&& other) noexcept
{
    if (this != &other) {
        if (m_id != 0) {
            assert(glGetError() == GL_NO_ERROR);
            glDeleteRenderbuffers(1, &m_id);
            assert(glGetError() == GL_NO_ERROR);
        }
        m_id = std::exchange(other.m_id, 0);
        m_samples = std::exchange(other.m_samples, 0);
    }
    return *this;
}

GLuint Renderbuffer::id() const { return m_id; }

GLsizei Renderbuffer::samples() const { return m_samples; }

void Renderbuffer::bind() const
{
    assert(glGetError() == GL_NO_ERROR);
    glBindRenderbuffer(GL_RENDERBUFFER, m_id);
    assert(glGetError() == GL_NO_ERROR);
}

void Renderbuffer::unbind() const
{
    assert(glGetError() == GL_NO_ERROR);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    assert(glGetError() == GL_NO_ERROR);
}

void Renderbuffer::setFormat(RenderbufferFormat format, GLsizei width, GLsizei height, GLsizei samples)
{
    GLenum formatGl;

    switch (format) {
    case RenderbufferFormat::R8:
        formatGl = GL_R8;
        break;
    case RenderbufferFormat::R8UI:
        formatGl = GL_R8UI;
        break;
    case RenderbufferFormat::R8I:
        formatGl = GL_R8I;
        break;
    case RenderbufferFormat::R16UI:
        formatGl = GL_R16UI;
        break;
    case RenderbufferFormat::R16I:
        formatGl = GL_R16I;
        break;
    case RenderbufferFormat::R32UI:
        formatGl = GL_R32UI;
        break;
    case RenderbufferFormat::R32I:
        formatGl = GL_R32I;
        break;
    case RenderbufferFormat::RG8:
        formatGl = GL_RG8;
        break;
    case RenderbufferFormat::RG8UI:
        formatGl = GL_RG8UI;
        break;
    case RenderbufferFormat::RG8I:
        formatGl = GL_RG8I;
        break;
    case RenderbufferFormat::RG16UI:
        formatGl = GL_RG16UI;
        break;
    case RenderbufferFormat::RG16I:
        formatGl = GL_RG16I;
        break;
    case RenderbufferFormat::RG32UI:
        formatGl = GL_RG32UI;
        break;
    case RenderbufferFormat::RG32I:
        formatGl = GL_RG32I;
        break;
    case RenderbufferFormat::RGB8:
        formatGl = GL_RGB8;
        break;
    case RenderbufferFormat::RGB565:
        formatGl = GL_RGB565;
        break;
    case RenderbufferFormat::RGBA8:
        formatGl = GL_RGBA8;
        break;
    case RenderbufferFormat::SRGB8Alpha8:
        formatGl = GL_SRGB8_ALPHA8;
        break;
    case RenderbufferFormat::RGB5A1:
        formatGl = GL_RGB5_A1;
        break;
    case RenderbufferFormat::RGBA4:
        formatGl = GL_RGBA4;
        break;
    case RenderbufferFormat::RGB10A2:
        formatGl = GL_RGB10_A2;
        break;
    case RenderbufferFormat::RGBA8UI:
        formatGl = GL_RGBA8UI;
        break;
    case RenderbufferFormat::RGBA8I:
        formatGl = GL_RGBA8I;
        break;
    case RenderbufferFormat::RGB10A2UI:
        formatGl = GL_RGB10_A2UI;
        break;
    case RenderbufferFormat::RGBA16UI:
        formatGl = GL_RGBA16UI;
        break;
    case RenderbufferFormat::RGBA16I:
        formatGl = GL_RGBA16I;
        break;
    case RenderbufferFormat::RGBA32I:
        formatGl = GL_RGBA32I;
        break;
    case RenderbufferFormat::RGBA32UI:
        formatGl = GL_RGBA32UI;
        break;
    case RenderbufferFormat::Depth16:
        formatGl = GL_DEPTH_COMPONENT16;
        break;
    case RenderbufferFormat::Depth24:
        formatGl = GL_DEPTH_COMPONENT24;
        break;
    case RenderbufferFormat::Depth32F:
        formatGl = GL_DEPTH_COMPONENT32F;
        break;
    case RenderbufferFormat::Depth24Stencil8:
        formatGl = GL_DEPTH24_STENCIL8;
        break;
    case RenderbufferFormat::Depth32FStencil8:
        formatGl = GL_DEPTH32F_STENCIL8;
        break;
    case RenderbufferFormat::Stencil8:
        formatGl = GL_STENCIL_INDEX8;
        break;
    default:
        return;
    }

    GLint max_samples;
    glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

    m_samples = samples < max_samples ? samples : max_samples;

    bind();
    assert(glGetError() == GL_NO_ERROR);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, formatGl, width, height);
    assert(glGetError() == GL_NO_ERROR);
    unbind();
}

}