#pragma once

#include <visualizer/GenericBuffer.hpp>

namespace Visualizer {

/**
 * Wrapper around a VBO of the type GL_ARRAY_BUFFER
 */
class VertexAttributeBuffer : public GenericBuffer {
public:
    VertexAttributeBuffer() = delete;

    /**
     * @brief Creates a new buffer.
     *
     * @param index Index to which the buffer will be bound.
     * @param elementSize Size of an element.
     * @param elementType Type of the element.
     * @param normalized Is normalized.
     * @param stride Stride.
     * @param offset Offset from start.
     * @param size Size of the buffer.
     * @param usage Usage info of the buffer.
     */
    VertexAttributeBuffer(GLuint index, GLint elementSize, GLenum elementType, GLboolean normalized, GLsizei stride,
        const void* offset, GLsizeiptr size, GLenum usage);

    /**
     * @brief Creates a new buffer.
     *
     * @param index Index to which the buffer will be bound.
     * @param elementSize Size of an element.
     * @param elementType Type of the element.
     * @param normalized Is normalized.
     * @param stride Stride.
     * @param offset Offset from start.
     * @param size Size of the buffer.
     * @param usage Usage info of the buffer.
     * @param data Buffer data.
     */
    VertexAttributeBuffer(GLuint index, GLint elementSize, GLenum elementType, GLboolean normalized, GLsizei stride,
        const void* offset, GLsizeiptr size, GLenum usage, const void* data);

    /**
     * @brief Copy constructor.
     *
     * @param buffer Existing buffer.
     */
    VertexAttributeBuffer(const VertexAttributeBuffer& buffer);

    /**
     * @brief Move constructor.
     *
     * @param buffer Existing buffer.
     */
    VertexAttributeBuffer(VertexAttributeBuffer&& buffer) noexcept;

    /**
     * @brief Destructor.
     */
    virtual ~VertexAttributeBuffer() = default;

    /**
     * @brief Copy assignment.
     *
     * @param buffer Existing buffer.
     */
    void operator=(const VertexAttributeBuffer& buffer);

    /**
     * @brief Move assignment.
     *
     * @param buffer Existing buffer.
     */
    void operator=(VertexAttributeBuffer&& buffer) noexcept;

    /**
     * @brief Binds the VBO to the current context.
     */
    void bind() const final;

    /**
     * @brief Unbinds the VBO from the current context.
     */
    void unbind() const final;

    /**
     * @brief Get the index to which the buffer will be bound
     *
     * @return Index to which the buffer will be bound
     */
    GLuint index() const;

    /**
     * @brief Get the element size.
     *
     * @return Element size.
     */
    GLint elementSize() const;

    /**
     * @brief Get the type of the element.
     *
     * @return Element type.
     */
    GLenum elementType() const;

    /**
     * @brief Get if the element is normalized.
     *
     * @return Is normalized.
     */
    GLboolean normalized() const;

    /**
     * @brief Get the stride of the buffer.
     *
     * @return Stride.
     */
    GLsizei stride() const;

    /**
     * @brief Get the offset of the buffer.
     *
     * @return Buffer offset.
     */
    const void* offset() const;

private:
    GLuint m_index;
    GLint m_elementSize;
    GLenum m_elementType;
    GLboolean m_normalized;
    GLsizei m_stride;
    const void* m_offset;
};

}