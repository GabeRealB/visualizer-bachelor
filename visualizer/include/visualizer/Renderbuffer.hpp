#pragma once

#include <glad/glad.h>

namespace Visualizer {

enum class RenderbufferFormat {
    R8,
    R8UI,
    R8I,
    R16UI,
    R16I,
    R32UI,
    R32I,
    RG8,
    RG8UI,
    RG8I,
    RG16UI,
    RG16I,
    RG32UI,
    RG32I,
    RGB8,
    RGB565,
    RGBA8,
    SRGB8Alpha8,
    RGB5A1,
    RGBA4,
    RGB10A2,
    RGBA8UI,
    RGBA8I,
    RGB10A2UI,
    RGBA16UI,
    RGBA16I,
    RGBA32I,
    RGBA32UI,
    Depth16,
    Depth24,
    Depth32F,
    Depth24Stencil8,
    Depth32FStencil8,
    Stencil8,
};

class Renderbuffer {
public:
    Renderbuffer();
    Renderbuffer(const Renderbuffer& other) = delete;
    Renderbuffer(Renderbuffer&& other) noexcept;
    ~Renderbuffer();

    Renderbuffer& operator=(const Renderbuffer& other) = delete;
    Renderbuffer& operator=(Renderbuffer&& other) noexcept;

    GLuint id() const;
    GLsizei samples() const;

    void bind() const;
    void unbind() const;

    void setFormat(RenderbufferFormat format, GLsizei width, GLsizei height, GLsizei samples);

private:
    GLuint m_id;
    GLsizei m_samples;
};

}