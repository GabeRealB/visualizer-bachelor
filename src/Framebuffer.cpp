#include <visualizer/Framebuffer.hpp>

#include <utility>

namespace Visualizer {

Framebuffer::Framebuffer()
    : m_id{ 0 }
    , m_buffers{}
{
    glGenFramebuffers(1, &m_id);
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_id{ std::exchange(other.m_id, 0) }
    , m_buffers{ std::exchange(other.m_buffers, {}) }
{
}

Framebuffer::~Framebuffer()
{
    if (m_id != 0) {
        glDeleteFramebuffers(1, &m_id);
    }
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other) {
        if (m_id != 0) {
            glDeleteFramebuffers(1, &m_id);
        }

        m_id = std::exchange(other.m_id, 0);
        m_buffers = std::exchange(other.m_buffers, {});
    }
    return *this;
}

GLuint Framebuffer::id() const { return m_id; }

bool Framebuffer::isComplete() const
{
    bind(FramebufferBinding::Read);
    auto status{ glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) };
    unbind(FramebufferBinding::Read);
    return status == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::bind(FramebufferBinding binding) const
{
    switch (binding) {
    case FramebufferBinding::Read:
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
        break;
    case FramebufferBinding::Write:
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_id);
        break;
    case FramebufferBinding::ReadWrite:
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
        break;
    default:
        return;
    }
}

void Framebuffer::unbind(FramebufferBinding binding) const
{
    switch (binding) {
    case FramebufferBinding::Read:
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        break;
    case FramebufferBinding::Write:
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        break;
    case FramebufferBinding::ReadWrite:
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        break;
    default:
        return;
    }
}

void Framebuffer::attachBuffer(FramebufferAttachment attachment, std::shared_ptr<Texture> texture)
{
    if (texture == nullptr) {
        return;
    }
    GLenum attachmentGl;

    switch (attachment) {
    case FramebufferAttachment::Color0:
        attachmentGl = GL_COLOR_ATTACHMENT0;
        break;
    case FramebufferAttachment::Color1:
        attachmentGl = GL_COLOR_ATTACHMENT1;
        break;
    case FramebufferAttachment::Color2:
        attachmentGl = GL_COLOR_ATTACHMENT2;
        break;
    case FramebufferAttachment::Color3:
        attachmentGl = GL_COLOR_ATTACHMENT3;
        break;
    case FramebufferAttachment::Depth:
        attachmentGl = GL_DEPTH_ATTACHMENT;
        break;
    case FramebufferAttachment::Stencil:
        attachmentGl = GL_STENCIL_ATTACHMENT;
        break;
    case FramebufferAttachment::DepthStencil:
        attachmentGl = GL_DEPTH_STENCIL_ATTACHMENT;
        break;
    default:
        return;
    }

    bind(FramebufferBinding::ReadWrite);

    switch (texture->type()) {
    case TextureType::Texture2D:
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentGl, GL_TEXTURE_2D, texture->id(), 0);
        break;
    default:
        return;
    }

    m_buffers.insert_or_assign(attachment, std::move(texture));
    unbind(FramebufferBinding::ReadWrite);
}

void Framebuffer::attachBuffer(FramebufferAttachment attachment, std::shared_ptr<Renderbuffer> renderbuffer)
{
    if (renderbuffer == nullptr) {
        return;
    }
    GLenum attachmentGl;

    switch (attachment) {
    case FramebufferAttachment::Color0:
        attachmentGl = GL_COLOR_ATTACHMENT0;
        break;
    case FramebufferAttachment::Color1:
        attachmentGl = GL_COLOR_ATTACHMENT1;
        break;
    case FramebufferAttachment::Color2:
        attachmentGl = GL_COLOR_ATTACHMENT2;
        break;
    case FramebufferAttachment::Color3:
        attachmentGl = GL_COLOR_ATTACHMENT3;
        break;
    case FramebufferAttachment::Depth:
        attachmentGl = GL_DEPTH_ATTACHMENT;
        break;
    case FramebufferAttachment::Stencil:
        attachmentGl = GL_STENCIL_ATTACHMENT;
        break;
    case FramebufferAttachment::DepthStencil:
        attachmentGl = GL_DEPTH_STENCIL_ATTACHMENT;
        break;
    default:
        return;
    }

    bind(FramebufferBinding::ReadWrite);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentGl, GL_RENDERBUFFER, renderbuffer->id());
    m_buffers.insert_or_assign(attachment, std::move(renderbuffer));
    unbind(FramebufferBinding::ReadWrite);
}

void Framebuffer::removeBuffer(FramebufferAttachment attachment)
{
    if (auto pos{ m_buffers.find(attachment) }; pos != m_buffers.end()) {
        GLenum attachmentGl;

        switch (attachment) {
        case FramebufferAttachment::Color0:
            attachmentGl = GL_COLOR_ATTACHMENT0;
            break;
        case FramebufferAttachment::Color1:
            attachmentGl = GL_COLOR_ATTACHMENT1;
            break;
        case FramebufferAttachment::Color2:
            attachmentGl = GL_COLOR_ATTACHMENT2;
            break;
        case FramebufferAttachment::Color3:
            attachmentGl = GL_COLOR_ATTACHMENT3;
            break;
        case FramebufferAttachment::Depth:
            attachmentGl = GL_DEPTH_ATTACHMENT;
            break;
        case FramebufferAttachment::Stencil:
            attachmentGl = GL_STENCIL_ATTACHMENT;
            break;
        case FramebufferAttachment::DepthStencil:
            attachmentGl = GL_DEPTH_STENCIL_ATTACHMENT;
            break;
        default:
            return;
        }

        bind(FramebufferBinding::ReadWrite);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentGl, GL_RENDERBUFFER, 0);
        unbind(FramebufferBinding::ReadWrite);

        m_buffers.erase(pos);
    }
}

}