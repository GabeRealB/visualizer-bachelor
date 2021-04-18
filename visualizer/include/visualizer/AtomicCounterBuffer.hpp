#pragma once

#include <visualizer/GenericBuffer.hpp>

namespace Visualizer {

/**
 * Wrapper around a VBO of the type GL_ATOMIC_COUNTER_BUFFER
 */
class AtomicCounterBuffer : public GenericBuffer {
public:
    AtomicCounterBuffer() = delete;

    /**
     * @brief Creates a new buffer.
     *
     * @param index Index to which the buffer will be bound.
     * @param element_count Number of elements.
     * @param usage Usage info of the buffer.
     */
    AtomicCounterBuffer(GLuint index, GLsizei element_count, GLenum usage);

    /**
     * @brief Creates a new buffer.
     *
     * @param index Index to which the buffer will be bound.
     * @param element_count Number of elements.
     * @param usage Usage info of the buffer.
     * @param data Buffer data.
     */
    AtomicCounterBuffer(GLuint index, GLsizei element_count, GLenum usage, const GLuint* data);

    /**
     * @brief Copy constructor.
     *
     * @param buffer Existing buffer.
     */
    AtomicCounterBuffer(const AtomicCounterBuffer& buffer);

    /**
     * @brief Move constructor.
     *
     * @param buffer Existing buffer.
     */
    AtomicCounterBuffer(AtomicCounterBuffer&& buffer) noexcept;

    /**
     * @brief Destructor.
     */
    virtual ~AtomicCounterBuffer() = default;

    /**
     * @brief Copy assignment.
     *
     * @param buffer Existing buffer.
     */
    AtomicCounterBuffer& operator=(const AtomicCounterBuffer& buffer);

    /**
     * @brief Move assignment.
     *
     * @param buffer Existing buffer.
     */
    AtomicCounterBuffer& operator=(AtomicCounterBuffer&& buffer) noexcept;

    GLuint index() const;

    GLsizei element_count() const;

private:
    GLuint m_index;
    GLsizei m_element_count;
};

}