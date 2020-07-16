#pragma once
#include <cstddef>
#include <glad/glad.h>

namespace Visualizer {

/**
 * A wrapper around a generic VBO
 */
class GenericBuffer {
public:
    GenericBuffer() = delete;

    /**
     * @brief Creates a new buffer
     *
     * @param target The target to which the buffer will be bound.
     * @param size Size of the buffer.
     * @param usage Buffer usage info.
     */
    GenericBuffer(GLenum target, GLsizeiptr size, GLenum usage);

    /**
     * @brief Creates a new buffer and initializes it.
     *
     * @param target The target to which the buffer will be bound.
     * @param size Size of the buffer.
     * @param usage Buffer usage info.
     * @param data Data that will be copied into the buffer.
     */
    GenericBuffer(GLenum target, GLsizeiptr size, GLenum usage, const void* data);

    /**
     * @brief Creates a copy of an existing buffer.
     *
     * @param buffer Buffer to copy.
     */
    GenericBuffer(const GenericBuffer& buffer);

    /**
     * @brief Buffer move constructor.
     *
     * @param buffer Existing buffer.
     */
    GenericBuffer(GenericBuffer&& buffer) noexcept;

    /**
     * Deletes the buffer.
     */
    virtual ~GenericBuffer();

    /**
     * @brief Copy assignment.
     *
     * @param buffer Existing buffer.
     */
    void operator=(const GenericBuffer& buffer);

    /**
     * @brief Move assignment.
     *
     * @param buffer Existing buffer.
     */
    void operator=(GenericBuffer&& buffer) noexcept;

    /**
     * @brief Binds the buffer to the current context.
     */
    virtual void bind() const;

    /**
     * @brief Unbinds the buffer from the current context.
     */
    virtual void unbind() const;

    /**
     * @brief Returns the id of the buffer.
     *
     * @return Id of the buffer.
     */
    GLuint id() const;

    /**
     * @brief Returns the target the buffer will be bound to.
     *
     * @return Target of the buffer.
     */
    GLenum target() const;

    /**
     * @brief Returns the size of the buffer.
     *
     * @return Size of the buffer.
     */
    GLsizeiptr size() const;

    /**
     * @brief Returns the usage info of the buffer.
     *
     * @return Usage info.
     */
    GLenum usage() const;

private:
    GLuint m_id;
    GLenum m_target;
    GLsizeiptr m_size;
    GLenum m_usage;
};

}
