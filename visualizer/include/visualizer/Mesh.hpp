#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <variant>

#include <visualizer/ComplexVertexAttributeBuffer.hpp>
#include <visualizer/VertexAttributeBuffer.hpp>

namespace Visualizer {

/**
 * Types of supported buffers.
 */
enum class MeshAttributes : int {
    Vertices = 0,
    Normals = 1,
    Tangents = 2,
    Color = 3,
    TextureCoordinate0 = 4,
    TextureCoordinate1 = 5,
    TextureCoordinate2 = 6,
    TextureCoordinate3 = 7,
    Indices = 8,
};

using VertexBufferVariant = std::variant<std::shared_ptr<const GenericBuffer>,
    std::shared_ptr<const VertexAttributeBuffer>, std::shared_ptr<const ComplexVertexAttributeBuffer>>;

/**
 * A wrapper around a VAO.
 */
class Mesh {
public:
    /**
     * @brief Creates an empty mesh.
     */
    Mesh();

    /**
     * @brief Copy constructor.
     *
     * @param mesh Existing mesh.
     */
    Mesh(const Mesh& mesh);

    /**
     * @brief Move constructor.
     * @param mesh Existing mesh.
     */
    Mesh(Mesh&& mesh) noexcept;

    /**
     * @brief Destructor.
     */
    ~Mesh();

    /**
     * @brief Copy assignment.
     *
     * @param mesh Existing mesh.
     */
    void operator=(const Mesh& mesh);

    /**
     * @brief Move assignment.
     *
     * @param mesh Existing mesh.
     */
    void operator=(Mesh&& mesh) noexcept;

    /**
     * @brief Creates a new VertexAttributeBuffer to store the vertices.
     *
     * @param vertices Vertices of the mesh.
     * @param count Number of vertices.
     */
    void setVertices(const glm::vec4* vertices, GLsizeiptr count);

    /**
     * @brief Creates a new VertexAttributeBuffer to store the first TextureCoordinates.
     *
     * @param coordinates First TextureCoordinates.
     * @param count Number of TextureCoordinates
     */
    void setTextureCoordinates0(const glm::vec4* coordinates, GLsizeiptr count);

    /**
     * @brief Creates a new GenericBuffer to store the indices.
     *
     * @param indices Indices.
     * @param count Number of indices.
     * @param primitiveType Used primitive type.
     */
    void setIndices(const GLuint* indices, GLsizeiptr count, GLenum primitiveType);

    void set_complex_attribute(const std::string& name, std::span<const VertexAttributeDesc> vertex_attributes,
        GLsizeiptr size, GLenum usage, const void* data);

    const VertexBufferVariant& get_vertex_buffer(MeshAttributes type) const;
    const VertexBufferVariant& get_vertex_buffer(const std::string& name) const;

    /**
     * @brief Binds the VAO to the current context.
     */
    void bind() const;

    /**
     * @brief Unbinds the VAO from the current context.
     */
    void unbind() const;

    void set_num_instances(GLsizei instances);

    /**
     * @brief Get the number of vertices.
     *
     * @return Number of vertices.
     */
    GLsizeiptr getVertexCount() const;

    /**
     * @brief Get the number of indices.
     *
     * @return Number of indices.
     */
    GLsizeiptr getIndexCount() const;

    /**
     * @brief Get the id of the VAO.
     *
     * @return VAO.
     */
    GLuint arrayObject() const;

    /**
     * @brief Get the used primitive type.
     *
     * @return Used primitive type.
     */
    GLenum primitiveType() const;

    /**
     * @brief Get the type used to store an index.
     *
     * @note Hardcoded to GL_UNSIGNED_INT.
     *
     * @return Index type.
     */
    GLenum indexType() const;

    /**
     * @brief Gets the offset of the index buffer.
     *
     * @note Hardcoded to nullptr.
     *
     * @return Offset of the index buffer.
     */
    const void* indexOffset() const;

    GLsizei instances() const;

private:
    void free();

    int m_key;
    GLuint m_arrayObject;
    GLenum m_primitiveType;
    GLenum m_indexType;
    const void* m_indexOffset;
    GLsizei m_instances;

    std::unordered_map<MeshAttributes, int> m_attributesMap;
    std::unordered_map<std::string, int> m_attributes_string_map;
    std::unordered_map<int, VertexBufferVariant> m_buffers;
};

}