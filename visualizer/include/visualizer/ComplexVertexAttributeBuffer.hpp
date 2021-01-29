#pragma once

#include <span>
#include <vector>

#include <visualizer/GenericBuffer.hpp>

namespace Visualizer {

enum class VertexAttributeType { Real, Integer, Long };

struct VertexAttributeDesc {
    GLuint index;
    GLint element_size;
    GLenum element_type;
    GLboolean normalized;
    GLsizei stride;
    const void* offset;
    GLuint divisor;
    VertexAttributeType type;
};

class ComplexVertexAttributeBuffer : public GenericBuffer {
public:
    ComplexVertexAttributeBuffer() = delete;

    ComplexVertexAttributeBuffer(
        std::span<const VertexAttributeDesc> vertex_attribute_pointers, GLsizeiptr size, GLenum usage);
    ComplexVertexAttributeBuffer(std::span<const VertexAttributeDesc> vertex_attribute_pointers, GLsizeiptr size,
        GLenum usage, const void* data);

    ComplexVertexAttributeBuffer(const ComplexVertexAttributeBuffer& buffer);
    ComplexVertexAttributeBuffer(ComplexVertexAttributeBuffer&& buffer) noexcept;

    virtual ~ComplexVertexAttributeBuffer() = default;

    ComplexVertexAttributeBuffer& operator=(const ComplexVertexAttributeBuffer& buffer);
    ComplexVertexAttributeBuffer& operator=(ComplexVertexAttributeBuffer&& buffer) noexcept;

    void bind() const final;
    void unbind() const final;

    std::span<const GLuint> indices() const;
    std::span<const GLint> element_sizes() const;
    std::span<const GLenum> element_types() const;
    std::span<const GLboolean> normalized() const;
    std::span<const GLsizei> strides() const;
    std::span<const void* const> offsets() const;
    std::span<const GLuint> divisors() const;
    std::span<const VertexAttributeType> types() const;

private:
    std::vector<GLuint> m_indices;
    std::vector<GLint> m_element_sizes;
    std::vector<GLenum> m_element_types;
    std::vector<GLboolean> m_normalized;
    std::vector<GLsizei> m_strides;
    std::vector<const void*> m_offsets;
    std::vector<GLuint> m_divisors;
    std::vector<VertexAttributeType> m_types;
};

}