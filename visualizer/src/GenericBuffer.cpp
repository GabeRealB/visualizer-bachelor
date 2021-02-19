#include <visualizer/GenericBuffer.hpp>

#include <cassert>
#include <utility>

namespace Visualizer {

GenericBuffer::GenericBuffer(GLenum target, GLsizeiptr size, GLenum usage)
    : GenericBuffer{ target, size, usage, nullptr }
{
    assert(glGetError() == GL_NO_ERROR);
}

GenericBuffer::GenericBuffer(GLenum target, GLsizeiptr size, GLenum usage, const void* data)
    : m_id{ 0 }
    , m_target{ target }
    , m_size{ size }
    , m_usage{ usage }
{
    assert(glGetError() == GL_NO_ERROR);
    glGenBuffers(1, &m_id);
    glBindBuffer(m_target, m_id);
    glBufferData(m_target, m_size, data, m_usage);
    glBindBuffer(m_target, 0);
    assert(glGetError() == GL_NO_ERROR);
}

GenericBuffer::GenericBuffer(const GenericBuffer& buffer)
    : m_id{ 0 }
    , m_target{ buffer.m_target }
    , m_size{ buffer.m_size }
    , m_usage{ buffer.m_usage }
{
    assert(glGetError() == GL_NO_ERROR);
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_id);
    glBufferData(GL_COPY_WRITE_BUFFER, m_size, nullptr, m_usage);
    glBindBuffer(GL_COPY_READ_BUFFER, buffer.m_id);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_size);

    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    assert(glGetError() == GL_NO_ERROR);
}

GenericBuffer::GenericBuffer(GenericBuffer&& buffer) noexcept
    : m_id{ std::exchange(buffer.m_id, 0) }
    , m_target{ std::exchange(buffer.m_target, GL_ARRAY_BUFFER) }
    , m_size{ std::exchange(buffer.m_size, 0) }
    , m_usage{ std::exchange(buffer.m_usage, GL_STATIC_DRAW) }
{
    assert(glGetError() == GL_NO_ERROR);
}

GenericBuffer::~GenericBuffer() { glDeleteBuffers(1, &m_id); }

void GenericBuffer::operator=(const GenericBuffer& buffer)
{
    assert(glGetError() == GL_NO_ERROR);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_id);
    glBindBuffer(GL_COPY_READ_BUFFER, buffer.m_id);

    if (buffer.m_size != m_size) {
        glBufferData(GL_COPY_WRITE_BUFFER, m_size, nullptr, m_usage);
    }

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_size);

    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    assert(glGetError() == GL_NO_ERROR);
}

void GenericBuffer::operator=(GenericBuffer&& buffer) noexcept
{
    assert(glGetError() == GL_NO_ERROR);
    glDeleteBuffers(1, &m_id);
    m_id = std::exchange(buffer.m_id, 0);
    m_target = std::exchange(buffer.m_target, GL_ARRAY_BUFFER);
    m_size = std::exchange(buffer.m_size, 0);
    m_usage = std::exchange(buffer.m_usage, GL_STATIC_DRAW);
    assert(glGetError() == GL_NO_ERROR);
}

void GenericBuffer::bind() const
{
    assert(glGetError() == GL_NO_ERROR);
    glBindBuffer(m_target, m_id);
    assert(glGetError() == GL_NO_ERROR);
}

void GenericBuffer::unbind() const {
    assert(glGetError() == GL_NO_ERROR);
    glBindBuffer(m_target, 0);
    assert(glGetError() == GL_NO_ERROR);
}

void* GenericBuffer::map(GLenum access)
{
    GenericBuffer::bind();
    assert(glGetError() == GL_NO_ERROR);
    auto ptr = glMapBuffer(m_target, access);
    assert(glGetError() == GL_NO_ERROR);
    GenericBuffer::unbind();
    return ptr;
}

bool GenericBuffer::unmap()
{
    GenericBuffer::bind();
    assert(glGetError() == GL_NO_ERROR);
    auto result = glUnmapBuffer(m_target);
    assert(glGetError() == GL_NO_ERROR);
    GenericBuffer::unbind();
    return result;
}

GLuint GenericBuffer::id() const { return m_id; }

GLenum GenericBuffer::target() const { return m_target; }

GLsizeiptr GenericBuffer::size() const { return m_size; }

GLenum GenericBuffer::usage() const { return m_usage; }

}