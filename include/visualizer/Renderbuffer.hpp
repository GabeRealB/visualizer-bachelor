#pragma once

#include <glad/glad.h>

namespace Visualizer {

enum class RenderbufferFormat { RGBA, Depth24Stencil8 };

class Renderbuffer {
public:
    Renderbuffer();
    Renderbuffer(const Renderbuffer& other) = delete;
    Renderbuffer(Renderbuffer&& other) noexcept;
    ~Renderbuffer();

    Renderbuffer& operator=(const Renderbuffer& other) = delete;
    Renderbuffer& operator=(Renderbuffer&& other) noexcept;

    GLuint id() const;

    void bind() const;
    void unbind() const;

    void setFormat(RenderbufferFormat format, GLsizei width, GLsizei height);

private:
    GLuint m_id;
};

}