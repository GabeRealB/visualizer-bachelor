#pragma once

#include <glad/glad.h>
#include <memory>
#include <unordered_map>
#include <variant>

#include <visualizer/Renderbuffer.hpp>
#include <visualizer/Texture.hpp>

namespace Visualizer {

enum class FramebufferBinding { Read, Write, ReadWrite };
enum class FramebufferAttachment { Color0, Color1, Color2, Color3, Depth, Stencil, DepthStencil };

class Framebuffer {
public:
    Framebuffer();
    Framebuffer(const Framebuffer& other) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    ~Framebuffer();

    Framebuffer& operator=(const Framebuffer& other) = delete;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    GLuint id() const;
    bool isComplete() const;

    void bind(FramebufferBinding binding) const;
    void unbind(FramebufferBinding binding) const;

    void attachBuffer(FramebufferAttachment attachment, std::shared_ptr<Texture> texture);
    void attachBuffer(FramebufferAttachment attachment, std::shared_ptr<Renderbuffer> renderbuffer);
    void removeBuffer(FramebufferAttachment attachment);

private:
    using BufferVariant = std::variant<std::shared_ptr<Texture>, std::shared_ptr<Renderbuffer>>;

    GLuint m_id;
    std::unordered_map<FramebufferAttachment, BufferVariant> m_buffers;
};

}