#include <visualizer/Renderbuffer.hpp>

#include <utility>

namespace Visualizer {

Renderbuffer::Renderbuffer()
    : m_id{ 0 }
{
    glGenRenderbuffers(1, &m_id);
}

Renderbuffer::Renderbuffer(Renderbuffer&& other) noexcept
    : m_id{ std::exchange(other.m_id, 0) }
{
}

Renderbuffer::~Renderbuffer()
{
    if (m_id != 0) {
        glDeleteRenderbuffers(1, &m_id);
    }
}

Renderbuffer& Renderbuffer::operator=(Renderbuffer&& other) noexcept
{
    if (this != &other) {
        if (m_id != 0) {
            glDeleteRenderbuffers(1, &m_id);
        }
        m_id = std::exchange(other.m_id, 0);
    }
    return *this;
}

GLuint Renderbuffer::id() const { return m_id; }

void Renderbuffer::bind() const { glBindRenderbuffer(GL_RENDERBUFFER, m_id); }

void Renderbuffer::unbind() const { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

void Renderbuffer::setFormat(RenderbufferFormat format, GLsizei width, GLsizei height)
{
    GLenum formatGl;

    switch (format) {
    case RenderbufferFormat::RGBA:
        formatGl = GL_RGBA;
        break;
    case RenderbufferFormat::Depth24Stencil8:
        formatGl = GL_DEPTH24_STENCIL8;
        break;
    default:
        return;
    }

    bind();
    glRenderbufferStorage(GL_RENDERBUFFER, formatGl, width, height);
    unbind();
}

}