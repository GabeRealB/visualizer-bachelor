#pragma once

#include <glad/glad.h>
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>
#include <variant>

#include <visualizer/Renderbuffer.hpp>
#include <visualizer/Texture.hpp>

namespace Visualizer {

enum class FramebufferBinding { Read, Write, ReadWrite };
enum class FramebufferBufferType { Texture, Renderbuffer };
enum class FramebufferAttachment { Color0, Color1, Color2, Color3, Depth, Stencil, DepthStencil };

enum class FramebufferCopyFlags { Color, Depth, Stencil };
enum class FramebufferCopyFilter { Nearest, Linear };

struct Rect {
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
};

class Framebuffer {
public:
    Framebuffer();
    Framebuffer(const Framebuffer& other) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    ~Framebuffer();

    Framebuffer& operator=(const Framebuffer& other) = delete;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    static const Framebuffer& defaultFramebuffer();
    static std::shared_ptr<const Framebuffer> defaultFramebufferPtr();

    GLuint id() const;
    Rect viewport() const;
    bool isComplete() const;

    const std::shared_ptr<Texture> texture(FramebufferAttachment attachment) const;
    const std::shared_ptr<Renderbuffer> renderbuffer(FramebufferAttachment attachment) const;
    std::optional<FramebufferBufferType> bufferType(FramebufferAttachment attachment) const;

    void bind(FramebufferBinding binding) const;
    void unbind(FramebufferBinding binding) const;

    void copyTo(Framebuffer& framebuffer, std::span<FramebufferCopyFlags> flags, FramebufferCopyFilter filter) const;
    void setViewport(GLint xCoord, GLint yCoord, GLsizei width, GLsizei height);

    void attachBuffer(FramebufferAttachment attachment, std::shared_ptr<Texture> texture);
    void attachBuffer(FramebufferAttachment attachment, std::shared_ptr<Renderbuffer> renderbuffer);
    void removeBuffer(FramebufferAttachment attachment);

private:
    Framebuffer(std::nullptr_t);

    using BufferVariant = std::variant<std::shared_ptr<Texture>, std::shared_ptr<Renderbuffer>>;

    GLuint m_id;
    std::optional<Rect> m_viewport;
    std::unordered_map<FramebufferAttachment, BufferVariant> m_buffers;
};

}