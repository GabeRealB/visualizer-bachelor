#include <visualizer/VertexAttributeBuffer.hpp>

#include <utility>
#include <cassert>

namespace Visualizer {

VertexAttributeBuffer::VertexAttributeBuffer(GLuint index, GLint elementSize, GLenum elementType, GLboolean normalized,
    GLsizei stride, const void* offset, GLsizeiptr size, GLenum usage)
    : VertexAttributeBuffer{ index, elementSize, elementType, normalized, stride, offset, size, usage, nullptr }
{
    assert(glGetError() == GL_NO_ERROR);
}

VertexAttributeBuffer::VertexAttributeBuffer(GLuint index, GLint elementSize, GLenum elementType, GLboolean normalized,
    GLsizei stride, const void* offset, GLsizeiptr size, GLenum usage, const void* data)
    : GenericBuffer{ GL_ARRAY_BUFFER, size, usage, data }
    , m_index{ index }
    , m_elementSize{ elementSize }
    , m_elementType{ elementType }
    , m_normalized{ normalized }
    , m_stride{ stride }
    , m_offset{ offset }
{
    assert(glGetError() == GL_NO_ERROR);
}

VertexAttributeBuffer::VertexAttributeBuffer(const VertexAttributeBuffer& buffer)
    : GenericBuffer(static_cast<const GenericBuffer&>(buffer))
    , m_index{ buffer.m_index }
    , m_elementSize{ buffer.m_elementSize }
    , m_elementType{ buffer.m_elementType }
    , m_normalized{ buffer.m_normalized }
    , m_stride{ buffer.m_stride }
    , m_offset{ buffer.m_offset }
{
    assert(glGetError() == GL_NO_ERROR);
}

VertexAttributeBuffer::VertexAttributeBuffer(VertexAttributeBuffer&& buffer) noexcept
    : GenericBuffer(static_cast<GenericBuffer&&>(buffer))
    , m_index{ std::exchange(buffer.m_index, 0) }
    , m_elementSize{ std::exchange(buffer.m_elementSize, 1) }
    , m_elementType{ std::exchange(buffer.m_elementType, GL_BYTE) }
    , m_normalized{ std::exchange(buffer.m_normalized, false) }
    , m_stride{ std::exchange(buffer.m_stride, 0) }
    , m_offset{ std::exchange(buffer.m_offset, nullptr) }
{
    assert(glGetError() == GL_NO_ERROR);
}

void VertexAttributeBuffer::operator=(const VertexAttributeBuffer& buffer)
{
    assert(glGetError() == GL_NO_ERROR);
    static_cast<GenericBuffer&>(*this) = static_cast<const GenericBuffer&>(buffer);
    m_index = buffer.m_index;
    m_elementSize = buffer.m_elementSize;
    m_elementType = buffer.m_elementType;
    m_normalized = buffer.m_normalized;
    m_stride = buffer.m_stride;
    m_offset = buffer.m_offset;
    assert(glGetError() == GL_NO_ERROR);
}

void VertexAttributeBuffer::operator=(VertexAttributeBuffer&& buffer) noexcept
{
    assert(glGetError() == GL_NO_ERROR);
    static_cast<GenericBuffer&>(*this) = static_cast<GenericBuffer&&>(std::move(buffer));
    m_index = std::exchange(buffer.m_index, 0);
    m_elementSize = std::exchange(buffer.m_elementSize, 1);
    m_elementType = std::exchange(buffer.m_elementType, GL_BYTE);
    m_normalized = std::exchange(buffer.m_normalized, false);
    m_stride = std::exchange(buffer.m_stride, 0);
    m_offset = std::exchange(buffer.m_offset, nullptr);
    assert(glGetError() == GL_NO_ERROR);
}

GLuint VertexAttributeBuffer::index() const { return m_index; }

GLint VertexAttributeBuffer::elementSize() const { return m_elementSize; }

GLenum VertexAttributeBuffer::elementType() const { return m_elementType; }

GLboolean VertexAttributeBuffer::normalized() const { return m_normalized; }

GLsizei VertexAttributeBuffer::stride() const { return m_stride; }

const void* VertexAttributeBuffer::offset() const { return m_offset; }

}