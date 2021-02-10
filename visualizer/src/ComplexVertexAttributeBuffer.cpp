#include <visualizer/ComplexVertexAttributeBuffer.hpp>

#include <cassert>

namespace Visualizer {

ComplexVertexAttributeBuffer::ComplexVertexAttributeBuffer(
    std::span<const VertexAttributeDesc> vertex_attribute_pointers, GLsizeiptr size, GLenum usage)
    : ComplexVertexAttributeBuffer{ vertex_attribute_pointers, size, usage, nullptr }
{
    assert(glGetError() == GL_NO_ERROR);
}

ComplexVertexAttributeBuffer::ComplexVertexAttributeBuffer(
    std::span<const VertexAttributeDesc> vertex_attribute_pointers, GLsizeiptr size, GLenum usage, const void* data)
    : GenericBuffer{ GL_ARRAY_BUFFER, size, usage, data }
    , m_indices{}
    , m_element_sizes{}
    , m_element_types{}
    , m_normalized{}
    , m_strides{}
    , m_offsets{}
    , m_divisors{}
    , m_types{}
{
    assert(glGetError() == GL_NO_ERROR);
    m_indices.reserve(vertex_attribute_pointers.size());
    m_element_sizes.reserve(vertex_attribute_pointers.size());
    m_element_types.reserve(vertex_attribute_pointers.size());
    m_normalized.reserve(vertex_attribute_pointers.size());
    m_strides.reserve(vertex_attribute_pointers.size());
    m_offsets.reserve(vertex_attribute_pointers.size());
    m_divisors.reserve(vertex_attribute_pointers.size());
    m_types.reserve(vertex_attribute_pointers.size());

    for (auto& attribute : vertex_attribute_pointers) {
        m_indices.push_back(attribute.index);
        m_element_sizes.push_back(attribute.element_size);
        m_element_types.push_back(attribute.element_type);
        m_normalized.push_back(attribute.normalized);
        m_strides.push_back(attribute.stride);
        m_offsets.push_back(attribute.offset);
        m_divisors.push_back(attribute.divisor);
        m_types.push_back(attribute.type);
    }
    assert(glGetError() == GL_NO_ERROR);
}

ComplexVertexAttributeBuffer::ComplexVertexAttributeBuffer(const ComplexVertexAttributeBuffer& buffer)
    : GenericBuffer(static_cast<const GenericBuffer&>(buffer))
    , m_indices{ buffer.m_indices }
    , m_element_sizes{ buffer.m_element_sizes }
    , m_element_types{ buffer.m_element_types }
    , m_normalized{ buffer.m_normalized }
    , m_strides{ buffer.m_strides }
    , m_offsets{ buffer.m_offsets }
    , m_divisors{ buffer.m_divisors }
    , m_types{ buffer.m_types }
{
    assert(glGetError() == GL_NO_ERROR);
}

ComplexVertexAttributeBuffer::ComplexVertexAttributeBuffer(ComplexVertexAttributeBuffer&& buffer) noexcept
    : GenericBuffer(static_cast<GenericBuffer&&>(std::move(buffer)))
    , m_indices{ std::exchange(buffer.m_indices, {}) }
    , m_element_sizes{ std::exchange(buffer.m_element_sizes, {}) }
    , m_element_types{ std::exchange(buffer.m_element_types, {}) }
    , m_normalized{ std::exchange(buffer.m_normalized, {}) }
    , m_strides{ std::exchange(buffer.m_strides, {}) }
    , m_offsets{ std::exchange(buffer.m_offsets, {}) }
    , m_divisors{ std::exchange(buffer.m_divisors, {}) }
    , m_types{ std::exchange(buffer.m_types, {}) }
{
    assert(glGetError() == GL_NO_ERROR);
}

ComplexVertexAttributeBuffer& ComplexVertexAttributeBuffer::operator=(const ComplexVertexAttributeBuffer& buffer)
{
    if (this != &buffer) {
        assert(glGetError() == GL_NO_ERROR);
        static_cast<GenericBuffer&>(*this) = static_cast<const GenericBuffer&>(buffer);
        m_indices = buffer.m_indices;
        m_element_sizes = buffer.m_element_sizes;
        m_element_types = buffer.m_element_types;
        m_normalized = buffer.m_normalized;
        m_strides = buffer.m_strides;
        m_offsets = buffer.m_offsets;
        m_divisors = buffer.m_divisors;
        m_types = buffer.m_types;
        assert(glGetError() == GL_NO_ERROR);
    }

    return *this;
}

ComplexVertexAttributeBuffer& ComplexVertexAttributeBuffer::operator=(ComplexVertexAttributeBuffer&& buffer) noexcept
{
    if (this != &buffer) {
        assert(glGetError() == GL_NO_ERROR);
        static_cast<GenericBuffer&>(*this) = static_cast<GenericBuffer&&>(std::move(buffer));
        m_indices = std::exchange(buffer.m_indices, {});
        m_element_sizes = std::exchange(buffer.m_element_sizes, {});
        m_element_types = std::exchange(buffer.m_element_types, {});
        m_normalized = std::exchange(buffer.m_normalized, {});
        m_strides = std::exchange(buffer.m_strides, {});
        m_offsets = std::exchange(buffer.m_offsets, {});
        m_divisors = std::exchange(buffer.m_divisors, {});
        m_types = std::exchange(buffer.m_types, {});
        assert(glGetError() == GL_NO_ERROR);
    }

    return *this;
}

void ComplexVertexAttributeBuffer::bind() const
{
    GenericBuffer::bind();
    for (std::size_t i = 0; i < m_indices.size(); ++i) {
        assert(glGetError() == GL_NO_ERROR);
        glEnableVertexAttribArray(m_indices[i]);

        switch (m_types[i]) {
        case VertexAttributeType::Real:
            glVertexAttribPointer(
                m_indices[i], m_element_sizes[i], m_element_types[i], m_normalized[i], m_strides[i], m_offsets[i]);
            break;
        case VertexAttributeType::Integer:
            glVertexAttribIPointer(m_indices[i], m_element_sizes[i], m_element_types[i], m_strides[i], m_offsets[i]);
            break;
        case VertexAttributeType::Long:
            glVertexAttribLPointer(m_indices[i], m_element_sizes[i], m_element_types[i], m_strides[i], m_offsets[i]);
            break;
        }
        glVertexAttribDivisor(m_indices[i], m_divisors[i]);
        assert(glGetError() == GL_NO_ERROR);
    }
}

void ComplexVertexAttributeBuffer::unbind() const
{
    for (std::size_t i = 0; i < m_indices.size(); ++i) {
        assert(glGetError() == GL_NO_ERROR);
        glDisableVertexAttribArray(m_indices[i]);
        assert(glGetError() == GL_NO_ERROR);
    }
    GenericBuffer::unbind();
}

std::span<const GLuint> ComplexVertexAttributeBuffer::indices() const { return { m_indices.data(), m_indices.size() }; }

std::span<const GLint> ComplexVertexAttributeBuffer::element_sizes() const
{
    return { m_element_sizes.data(), m_element_sizes.size() };
}

std::span<const GLenum> ComplexVertexAttributeBuffer::element_types() const
{
    return { m_element_types.data(), m_element_types.size() };
}

std::span<const GLboolean> ComplexVertexAttributeBuffer::normalized() const
{
    return { m_normalized.data(), m_normalized.size() };
}

std::span<const GLsizei> ComplexVertexAttributeBuffer::strides() const
{
    return { m_strides.data(), m_strides.size() };
}

std::span<const void* const> ComplexVertexAttributeBuffer::offsets() const
{
    return { m_offsets.data(), m_offsets.size() };
}

std::span<const GLuint> ComplexVertexAttributeBuffer::divisors() const
{
    return { m_divisors.data(), m_divisors.size() };
}

std::span<const VertexAttributeType> ComplexVertexAttributeBuffer::types() const
{
    return { m_types.data(), m_types.size() };
}

}
