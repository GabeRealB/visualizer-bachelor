#include <visualizer/Mesh.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace Visualizer {

Mesh::Mesh()
    : m_key{ 0 }
    , m_arrayObject{ 0 }
    , m_primitiveType{ GL_TRIANGLES }
    , m_indexType{ GL_UNSIGNED_INT }
    , m_indexOffset{ nullptr }
    , m_attributesMap{}
    , m_buffers{}
{
    glGenVertexArrays(1, &m_arrayObject);

    setVertices(nullptr, 0);
    setIndices(nullptr, 0, GL_TRIANGLES);
}

Mesh::Mesh(const Mesh& mesh)
    : m_key{ mesh.m_key }
    , m_arrayObject{ 0 }
    , m_primitiveType{ mesh.m_primitiveType }
    , m_indexType{ mesh.m_indexType }
    , m_indexOffset{ mesh.m_indexOffset }
    , m_attributesMap{ mesh.m_attributesMap }
    , m_buffers{ mesh.m_buffers }
{
    std::cout << "Mesh copy constructor" << std::endl;
    glGenVertexArrays(1, &m_arrayObject);
    bind();

    for (auto& keyValue : m_buffers) {
        std::visit([](auto& buffer) { buffer->bind(); }, keyValue.second);
    }

    unbind();
}

Mesh::Mesh(Mesh&& mesh) noexcept
    : m_key{ std::exchange(mesh.m_key, 0) }
    , m_arrayObject{ std::exchange(mesh.m_arrayObject, 0) }
    , m_primitiveType{ std::exchange(mesh.m_primitiveType, GL_TRIANGLES) }
    , m_indexType{ std::exchange(mesh.m_indexType, GL_UNSIGNED_INT) }
    , m_indexOffset{ std::exchange(mesh.m_indexOffset, nullptr) }
    , m_attributesMap{ std::exchange(mesh.m_attributesMap, {}) }
    , m_buffers{ std::exchange(mesh.m_buffers, {}) }
{
    std::cout << "Mesh move constructor" << std::endl;
}

Mesh::~Mesh() { free(); }

void Mesh::operator=(const Mesh& mesh)
{
    std::cout << "Mesh copy assignment" << std::endl;
    bind();

    for (auto& keyValue : m_buffers) {
        std::visit([](auto& buffer) { buffer->unbind(); }, keyValue.second);
    }

    m_attributesMap.clear();
    m_buffers.clear();

    m_key = mesh.m_key;
    m_primitiveType = mesh.m_primitiveType;
    m_indexType = mesh.m_indexType;
    m_indexOffset = mesh.m_indexOffset;
    m_attributesMap = mesh.m_attributesMap;
    m_buffers = mesh.m_buffers;

    for (auto& keyValue : m_buffers) {
        std::visit([](auto& buffer) { buffer->bind(); }, keyValue.second);
    }

    unbind();
}

void Mesh::operator=(Mesh&& mesh) noexcept
{
    std::cout << "Mesh move assignment" << std::endl;
    free();

    m_key = std::exchange(mesh.m_key, 0);
    m_arrayObject = std::exchange(mesh.m_arrayObject, 0);
    m_primitiveType = std::exchange(mesh.m_primitiveType, GL_TRIANGLES);
    m_indexType = std::exchange(mesh.m_indexType, GL_UNSIGNED_INT);
    m_indexOffset = std::exchange(mesh.m_indexOffset, nullptr);
    m_attributesMap = std::exchange(mesh.m_attributesMap, {});
    m_buffers = std::exchange(mesh.m_buffers, {});
}

void Mesh::setVertices(const glm::vec4* vertices, GLsizeiptr count)
{
    bind();

    auto bufferLocation{ m_attributesMap.find(MeshAttributes::Vertices) };
    if (bufferLocation != m_attributesMap.end()) {
        auto bufferKeyValue{ m_buffers.find(bufferLocation->second) };
        auto& buffer{ bufferKeyValue->second };
        std::visit([](auto& buffer) { return buffer->unbind(); }, buffer);
        m_buffers.erase(bufferKeyValue);
    }

    const void* dataPtr{ nullptr };
    if (vertices != nullptr) {
        dataPtr = glm::value_ptr(*vertices);
    }

    auto ptr{ std::make_shared<VertexAttributeBuffer>(
        0, 4, GL_FLOAT, false, 0, nullptr, count * sizeof(float) * 4, GL_STATIC_DRAW, dataPtr) };
    ptr->bind();

    unbind();
    ptr->unbind();

    auto key{ m_key++ };
    m_attributesMap[MeshAttributes::Vertices] = key;
    m_buffers[key] = std::const_pointer_cast<const VertexAttributeBuffer>(ptr);
}

void Mesh::setTextureCoordinates0(const glm::vec4* coordinates, GLsizeiptr count)
{
    bind();

    auto bufferLocation{ m_attributesMap.find(MeshAttributes::TextureCoordinate0) };
    if (bufferLocation != m_attributesMap.end()) {
        auto bufferKeyValue{ m_buffers.find(bufferLocation->second) };
        auto& buffer{ bufferKeyValue->second };
        std::visit([](auto& buffer) { return buffer->unbind(); }, buffer);
        m_buffers.erase(bufferKeyValue);
    }

    const void* dataPtr{ nullptr };
    if (coordinates != nullptr) {
        dataPtr = glm::value_ptr(*coordinates);
    }

    auto ptr{ std::make_shared<VertexAttributeBuffer>(
        1, 4, GL_FLOAT, false, 0, nullptr, count * sizeof(float) * 4, GL_STATIC_DRAW, dataPtr) };
    ptr->bind();

    unbind();
    ptr->unbind();

    auto key{ m_key++ };
    m_attributesMap[MeshAttributes::TextureCoordinate0] = key;
    m_buffers[key] = std::const_pointer_cast<const VertexAttributeBuffer>(ptr);
}

void Mesh::setIndices(const GLuint* indices, GLsizeiptr count, GLenum primitiveType)
{
    bind();

    auto bufferLocation{ m_attributesMap.find(MeshAttributes::Indices) };
    if (bufferLocation != m_attributesMap.end()) {
        auto bufferKeyValue{ m_buffers.find(bufferLocation->second) };
        auto& buffer{ bufferKeyValue->second };
        std::visit([](auto& buffer) { return buffer->unbind(); }, buffer);
        m_buffers.erase(bufferKeyValue);
    }

    auto ptr{ std::make_shared<GenericBuffer>(
        GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), GL_STATIC_DRAW, indices) };
    ptr->bind();

    unbind();
    ptr->unbind();

    auto key{ m_key++ };
    m_attributesMap[MeshAttributes::Indices] = key;
    m_buffers[key] = std::const_pointer_cast<const GenericBuffer>(ptr);

    m_indexType = GL_UNSIGNED_INT;
    m_primitiveType = primitiveType;
}

void Mesh::bind() const { glBindVertexArray(m_arrayObject); }

void Mesh::unbind() const { glBindVertexArray(0); }

GLsizeiptr Mesh::getVertexCount() const
{
    auto bufferLocation{ m_attributesMap.find(MeshAttributes::Vertices) };
    if (bufferLocation == m_attributesMap.end()) {
        return 0;
    } else {
        auto& buffer{ m_buffers.at(bufferLocation->second) };
        auto size{ std::visit([](auto& buffer) { return buffer->size(); }, buffer) };

        return size / (sizeof(float) * 4);
    }
}

GLsizeiptr Mesh::getIndexCount() const
{
    auto bufferLocation{ m_attributesMap.find(MeshAttributes::Indices) };
    if (bufferLocation == m_attributesMap.end()) {
        return 0;
    } else {
        auto& buffer{ m_buffers.at(bufferLocation->second) };
        auto size{ std::visit([](auto& buffer) { return buffer->size(); }, buffer) };

        return size / sizeof(GLuint);
    }
}

GLuint Mesh::arrayObject() const { return m_arrayObject; }

GLenum Mesh::primitiveType() const { return m_primitiveType; }

GLenum Mesh::indexType() const { return m_indexType; }

const void* Mesh::indexOffset() const { return m_indexOffset; }

void Mesh::free()
{
    m_attributesMap.clear();
    m_buffers.clear();
    glDeleteVertexArrays(1, &m_arrayObject);
}

}
