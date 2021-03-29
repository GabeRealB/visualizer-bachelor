#include <visualizer/Framebuffer.hpp>

#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <utility>

namespace Visualizer {

Framebuffer::Framebuffer()
    : m_id{ 0 }
    , m_buffers{}
{
    assert(glGetError() == GL_NO_ERROR);
    glGenFramebuffers(1, &m_id);
    assert(glGetError() == GL_NO_ERROR);
}

Framebuffer::Framebuffer(std::nullptr_t)
    : m_id{ 0 }
    , m_buffers{}
{
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_id{ std::exchange(other.m_id, 0) }
    , m_buffers{ std::exchange(other.m_buffers, {}) }
{
}

Framebuffer::~Framebuffer()
{
    if (m_id != 0) {
        assert(glGetError() == GL_NO_ERROR);
        glDeleteFramebuffers(1, &m_id);
        assert(glGetError() == GL_NO_ERROR);
    }
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other) {
        if (m_id != 0) {
            assert(glGetError() == GL_NO_ERROR);
            glDeleteFramebuffers(1, &m_id);
            assert(glGetError() == GL_NO_ERROR);
        }

        m_id = std::exchange(other.m_id, 0);
        m_buffers = std::exchange(other.m_buffers, {});
    }
    return *this;
}

const Framebuffer& Framebuffer::defaultFramebuffer()
{
    static Framebuffer framebuffer{ nullptr };
    return framebuffer;
}

std::shared_ptr<const Framebuffer> Framebuffer::defaultFramebufferPtr()
{
    static auto framebuffer{ std::shared_ptr<Framebuffer>(new Framebuffer{ nullptr }) };
    return framebuffer;
}

GLuint Framebuffer::id() const { return m_id; }

Rect Framebuffer::viewport() const
{
    if (m_viewport) {
        return *m_viewport;
    } else {
        GLint width;
        GLint height;
        glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
        return { .x = 0, .y = 0, .width = width, .height = height };
    }
}

bool Framebuffer::isComplete() const
{
    bind(FramebufferBinding::Read);
    auto status{ glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) };
    unbind(FramebufferBinding::Read);
    return status == GL_FRAMEBUFFER_COMPLETE;
}

const std::shared_ptr<Texture> Framebuffer::texture(FramebufferAttachment attachment) const
{
    if (auto pos{ m_buffers.find(attachment) }; pos != m_buffers.end()) {
        if (std::holds_alternative<std::shared_ptr<Texture>>(pos->second)) {
            return std::get<std::shared_ptr<Texture>>(pos->second);
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

const std::shared_ptr<Renderbuffer> Framebuffer::renderbuffer(FramebufferAttachment attachment) const
{
    if (auto pos{ m_buffers.find(attachment) }; pos != m_buffers.end()) {
        if (std::holds_alternative<std::shared_ptr<Renderbuffer>>(pos->second)) {
            return std::get<std::shared_ptr<Renderbuffer>>(pos->second);
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

std::optional<FramebufferBufferType> Framebuffer::bufferType(FramebufferAttachment attachment) const
{
    if (auto pos{ m_buffers.find(attachment) }; pos != m_buffers.end()) {
        if (std::holds_alternative<std::shared_ptr<Texture>>(pos->second)) {
            return FramebufferBufferType::Texture;
        } else if (std::holds_alternative<std::shared_ptr<Renderbuffer>>(pos->second)) {
            return FramebufferBufferType::Renderbuffer;
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }
}

void Framebuffer::bind(FramebufferBinding binding) const
{
    assert(glGetError() == GL_NO_ERROR);
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
    assert(glGetError() == GL_NO_ERROR);

    auto view{ viewport() };
    glViewport(view.x, view.y, view.width, view.height);
    assert(glGetError() == GL_NO_ERROR);
}

void Framebuffer::unbind(FramebufferBinding binding) const
{
    assert(glGetError() == GL_NO_ERROR);
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
    assert(glGetError() == GL_NO_ERROR);
}

void Framebuffer::copyTo(
    Framebuffer& framebuffer, std::span<FramebufferCopyFlags> flags, FramebufferCopyFilter filter) const
{
    if (!isComplete() || !framebuffer.isComplete()) {
        std::cerr << "The framebuffers are not complete" << std::endl;
    }

    auto srcPort{ viewport() };
    auto dstPort{ framebuffer.viewport() };

    if (srcPort.width > dstPort.width) {
        srcPort.width = dstPort.width;
    }
    if (srcPort.height > dstPort.height) {
        srcPort.height = dstPort.height;
    }

    GLbitfield glMask = 0;
    GLenum glFilter = 0;

    for (auto flag : flags) {
        switch (flag) {
        case FramebufferCopyFlags::Color:
            glMask |= GL_COLOR_BUFFER_BIT;
            break;
        case FramebufferCopyFlags::Depth:
            glMask |= GL_DEPTH_BUFFER_BIT;
            break;
        case FramebufferCopyFlags::Stencil:
            glMask |= GL_STENCIL_BUFFER_BIT;
            break;
        }
    }

    switch (filter) {
    case FramebufferCopyFilter::Nearest:
        glFilter = GL_NEAREST;
        break;
    case FramebufferCopyFilter::Linear:
        glFilter = GL_LINEAR;
        break;
    }

    bind(FramebufferBinding::Read);
    framebuffer.bind(FramebufferBinding::Write);

    assert(glGetError() == GL_NO_ERROR);
    glBlitFramebuffer(srcPort.x, srcPort.y, srcPort.x + srcPort.width, srcPort.y + srcPort.height, dstPort.x, dstPort.y,
        dstPort.x + dstPort.width, dstPort.y + dstPort.height, glMask, glFilter);
    assert(glGetError() == GL_NO_ERROR);

    framebuffer.unbind(FramebufferBinding::Write);
    unbind(FramebufferBinding::Read);
}

void Framebuffer::setViewport(GLint xCoord, GLint yCoord, GLsizei width, GLsizei height)
{
    if (width == 0 || height == 0) {
        m_viewport = std::nullopt;
    } else {
        m_viewport = { .x = xCoord, .y = yCoord, .width = width, .height = height };
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
    texture->bind(TextureSlot::TmpSlot);
    texture->unbind(TextureSlot::TmpSlot);

    assert(glGetError() == GL_NO_ERROR);
    switch (texture->type()) {
    case TextureType::TextureBuffer:
        unbind(FramebufferBinding::ReadWrite);
        return;
    case TextureType::Texture2D:
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentGl, GL_TEXTURE_2D, texture->id(), 0);
        break;
    case TextureType::Texture2DMultisample:
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentGl, GL_TEXTURE_2D_MULTISAMPLE, texture->id(), 0);
        break;
    }
    assert(glGetError() == GL_NO_ERROR);

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
    assert(glGetError() == GL_NO_ERROR);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentGl, GL_RENDERBUFFER, renderbuffer->id());
    assert(glGetError() == GL_NO_ERROR);
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
        assert(glGetError() == GL_NO_ERROR);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentGl, GL_RENDERBUFFER, 0);
        assert(glGetError() == GL_NO_ERROR);
        unbind(FramebufferBinding::ReadWrite);

        m_buffers.erase(pos);
    }
}

}