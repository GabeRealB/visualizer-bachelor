#include <visualizer/AtomicCounterBuffer.hpp>

#include <cassert>
#include <utility>

namespace Visualizer {

AtomicCounterBuffer::AtomicCounterBuffer(GLuint index, GLsizei element_count, GLenum usage)
    : AtomicCounterBuffer{ index, element_count, usage, nullptr }
{
    assert(glGetError() == GL_NO_ERROR);
}

AtomicCounterBuffer::AtomicCounterBuffer(GLuint index, GLsizei element_count, GLenum usage, const GLuint* data)
    : GenericBuffer{ GL_ATOMIC_COUNTER_BUFFER, static_cast<GLsizeiptr>(element_count * sizeof(GLuint)), usage, data }
    , m_index{ index }
    , m_element_count{ element_count }
{
    assert(glGetError() == GL_NO_ERROR);
}

AtomicCounterBuffer::AtomicCounterBuffer(const AtomicCounterBuffer& buffer)
    : GenericBuffer(static_cast<const GenericBuffer&>(buffer))
    , m_index{ buffer.m_index }
    , m_element_count{ buffer.m_element_count }
{
    assert(glGetError() == GL_NO_ERROR);
}

AtomicCounterBuffer::AtomicCounterBuffer(AtomicCounterBuffer&& buffer) noexcept
    : GenericBuffer(static_cast<GenericBuffer&&>(buffer))
    , m_index{ std::exchange(buffer.m_index, 0) }
    , m_element_count{ std::exchange(buffer.m_element_count, 0) }
{
    assert(glGetError() == GL_NO_ERROR);
}

AtomicCounterBuffer& AtomicCounterBuffer::operator=(const AtomicCounterBuffer& buffer)
{
    if (this != &buffer) {
        assert(glGetError() == GL_NO_ERROR);
        static_cast<GenericBuffer&>(*this) = static_cast<const GenericBuffer&>(buffer);
        m_index = buffer.m_index;
        m_element_count = buffer.m_element_count;
        assert(glGetError() == GL_NO_ERROR);
    }
    return *this;
}

AtomicCounterBuffer& AtomicCounterBuffer::operator=(AtomicCounterBuffer&& buffer) noexcept
{
    if (this != &buffer) {
        assert(glGetError() == GL_NO_ERROR);
        static_cast<GenericBuffer&>(*this) = static_cast<GenericBuffer&&>(std::move(buffer));
        m_index = std::exchange(buffer.m_index, 0);
        m_element_count = std::exchange(buffer.m_element_count, 0);
        assert(glGetError() == GL_NO_ERROR);
    }
    return *this;
}

GLuint AtomicCounterBuffer::index() const { return m_index; }

GLsizei AtomicCounterBuffer::element_count() const { return m_element_count; }

}