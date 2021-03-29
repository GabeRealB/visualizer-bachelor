#include <visualizer/Mesh.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace Visualizer {

auto buffer_bind_fn = overloaded{
    [](std::shared_ptr<GenericBuffer>& buffer) { buffer->bind(); },
    [](std::shared_ptr<VertexAttributeBuffer>& buffer) { buffer->bind(); },
    [](std::shared_ptr<ComplexVertexAttributeBuffer>& buffer) { buffer->bind(); },
};

auto buffer_unbind_fn = overloaded{
    [](std::shared_ptr<GenericBuffer>& buffer) { buffer->unbind(); },
    [](std::shared_ptr<VertexAttributeBuffer>& buffer) { buffer->unbind(); },
    [](std::shared_ptr<ComplexVertexAttributeBuffer>& buffer) { buffer->unbind(); },
};

auto buffer_attach_fn = overloaded{
    [](std::shared_ptr<GenericBuffer>&) {},
    [](std::shared_ptr<VertexAttributeBuffer>& buffer) {
        assert(glGetError() == GL_NO_ERROR);
        glEnableVertexAttribArray(buffer->index());
        glVertexAttribPointer(buffer->index(), buffer->elementSize(), buffer->elementType(), buffer->normalized(),
            buffer->stride(), buffer->offset());
        assert(glGetError() == GL_NO_ERROR);
    },
    [](std::shared_ptr<ComplexVertexAttributeBuffer>& buffer) {
        auto indices = buffer->indices();
        auto element_sizes = buffer->element_sizes();
        auto element_types = buffer->element_types();
        auto normalized = buffer->normalized();
        auto strides = buffer->strides();
        auto offsets = buffer->offsets();
        auto divisors = buffer->divisors();
        auto types = buffer->types();

        for (std::size_t i = 0; i < indices.size(); ++i) {
            assert(glGetError() == GL_NO_ERROR);
            glEnableVertexAttribArray(indices[i]);

            switch (types[i]) {
            case VertexAttributeType::Real:
                glVertexAttribPointer(
                    indices[i], element_sizes[i], element_types[i], normalized[i], strides[i], offsets[i]);
                break;
            case VertexAttributeType::Integer:
                glVertexAttribIPointer(indices[i], element_sizes[i], element_types[i], strides[i], offsets[i]);
                break;
            case VertexAttributeType::Long:
                glVertexAttribLPointer(indices[i], element_sizes[i], element_types[i], strides[i], offsets[i]);
                break;
            }
            glVertexAttribDivisor(indices[i], divisors[i]);
            assert(glGetError() == GL_NO_ERROR);
        }
    },
};

auto buffer_detach_fn = overloaded{
    [](std::shared_ptr<GenericBuffer>& buffer) {
        buffer->bind();
        buffer->unbind();
    },
    [](std::shared_ptr<VertexAttributeBuffer>& buffer) {
        buffer->bind();

        assert(glGetError() == GL_NO_ERROR);
        glDisableVertexAttribArray(buffer->index());
        assert(glGetError() == GL_NO_ERROR);

        buffer->unbind();
    },
    [](std::shared_ptr<ComplexVertexAttributeBuffer>& buffer) {
        buffer->bind();

        auto indices = buffer->indices();

        for (auto&& idx : indices) {
            assert(glGetError() == GL_NO_ERROR);
            glDisableVertexAttribArray(idx);
            assert(glGetError() == GL_NO_ERROR);
        }

        buffer->unbind();
    },
};

Mesh::Mesh()
    : m_key{ 0 }
    , m_arrayObject{ 0 }
    , m_primitiveType{ GL_TRIANGLES }
    , m_indexType{ GL_UNSIGNED_INT }
    , m_indexOffset{ nullptr }
    , m_instances{ 1 }
    , m_attributesMap{}
    , m_attributes_string_map{}
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
    , m_instances{ mesh.m_instances }
    , m_attributesMap{ mesh.m_attributesMap }
    , m_attributes_string_map(mesh.m_attributes_string_map)
    , m_buffers{ mesh.m_buffers }
{
    glGenVertexArrays(1, &m_arrayObject);
    bind();

    for (auto& keyValue : m_buffers) {
        std::visit([](auto& buffer) { buffer->bind(); }, keyValue.second);
        std::visit(buffer_attach_fn, keyValue.second);
    }

    unbind();

    for (auto& keyValue : m_buffers) {
        std::visit([](auto& buffer) { buffer->unbind(); }, keyValue.second);
    }
}

Mesh::Mesh(Mesh&& mesh) noexcept
    : m_key{ std::exchange(mesh.m_key, 0) }
    , m_arrayObject{ std::exchange(mesh.m_arrayObject, 0) }
    , m_primitiveType{ std::exchange(mesh.m_primitiveType, GL_TRIANGLES) }
    , m_indexType{ std::exchange(mesh.m_indexType, GL_UNSIGNED_INT) }
    , m_indexOffset{ std::exchange(mesh.m_indexOffset, nullptr) }
    , m_instances{ std::exchange(mesh.m_instances, 1) }
    , m_attributesMap{ std::exchange(mesh.m_attributesMap, {}) }
    , m_attributes_string_map{ std::exchange(mesh.m_attributes_string_map, {}) }
    , m_buffers{ std::exchange(mesh.m_buffers, {}) }
{
}

Mesh::~Mesh() { free(); }

void Mesh::operator=(const Mesh& mesh)
{
    bind();

    for (auto& keyValue : m_buffers) {
        std::visit(buffer_detach_fn, keyValue.second);
    }

    m_attributesMap.clear();
    m_buffers.clear();

    m_key = mesh.m_key;
    m_primitiveType = mesh.m_primitiveType;
    m_indexType = mesh.m_indexType;
    m_indexOffset = mesh.m_indexOffset;
    m_instances = mesh.m_instances;
    m_attributesMap = mesh.m_attributesMap;
    m_attributes_string_map = mesh.m_attributes_string_map;
    m_buffers = mesh.m_buffers;

    for (auto& keyValue : m_buffers) {
        std::visit([](auto& buffer) { buffer->bind(); }, keyValue.second);
        std::visit(buffer_attach_fn, keyValue.second);
    }

    unbind();

    for (auto& keyValue : m_buffers) {
        std::visit([](auto& buffer) { buffer->unbind(); }, keyValue.second);
    }
}

void Mesh::operator=(Mesh&& mesh) noexcept
{
    free();

    m_key = std::exchange(mesh.m_key, 0);
    m_arrayObject = std::exchange(mesh.m_arrayObject, 0);
    m_primitiveType = std::exchange(mesh.m_primitiveType, GL_TRIANGLES);
    m_indexType = std::exchange(mesh.m_indexType, GL_UNSIGNED_INT);
    m_indexOffset = std::exchange(mesh.m_indexOffset, nullptr);
    m_instances = std::exchange(mesh.m_instances, 1);
    m_attributesMap = std::exchange(mesh.m_attributesMap, {});
    m_attributes_string_map = std::exchange(mesh.m_attributes_string_map, {});
    m_buffers = std::exchange(mesh.m_buffers, {});
}

void Mesh::setVertices(const glm::vec4* vertices, GLsizeiptr count)
{
    bind();

    auto bufferLocation{ m_attributesMap.find(MeshAttributes::Vertices) };
    if (bufferLocation != m_attributesMap.end()) {
        auto bufferKeyValue{ m_buffers.find(bufferLocation->second) };
        auto& buffer{ bufferKeyValue->second };
        std::visit(buffer_detach_fn, buffer);
        m_buffers.erase(bufferKeyValue);
    }

    const void* dataPtr{ nullptr };
    if (vertices != nullptr) {
        dataPtr = glm::value_ptr(*vertices);
    }

    auto ptr{ std::make_shared<VertexAttributeBuffer>(
        0, 4, GL_FLOAT, false, 0, nullptr, count * sizeof(float) * 4, GL_STATIC_DRAW, dataPtr) };
    ptr->bind();

    buffer_attach_fn.operator()(ptr);

    unbind();
    ptr->unbind();

    auto key{ m_key++ };
    m_attributesMap[MeshAttributes::Vertices] = key;
    m_buffers[key] = ptr;
}

void Mesh::setTextureCoordinates0(const glm::vec4* coordinates, GLsizeiptr count)
{
    bind();

    auto bufferLocation{ m_attributesMap.find(MeshAttributes::TextureCoordinate0) };
    if (bufferLocation != m_attributesMap.end()) {
        auto bufferKeyValue{ m_buffers.find(bufferLocation->second) };
        auto& buffer{ bufferKeyValue->second };
        std::visit(buffer_detach_fn, buffer);
        m_buffers.erase(bufferKeyValue);
    }

    const void* dataPtr{ nullptr };
    if (coordinates != nullptr) {
        dataPtr = glm::value_ptr(*coordinates);
    }

    auto ptr{ std::make_shared<VertexAttributeBuffer>(
        1, 4, GL_FLOAT, false, 0, nullptr, count * sizeof(float) * 4, GL_STATIC_DRAW, dataPtr) };
    ptr->bind();

    buffer_attach_fn.operator()(ptr);

    unbind();
    ptr->unbind();

    auto key{ m_key++ };
    m_attributesMap[MeshAttributes::TextureCoordinate0] = key;
    m_buffers[key] = ptr;
}

void Mesh::setIndices(const GLuint* indices, GLsizeiptr count, GLenum primitiveType)
{
    bind();

    auto bufferLocation{ m_attributesMap.find(MeshAttributes::Indices) };
    if (bufferLocation != m_attributesMap.end()) {
        auto bufferKeyValue{ m_buffers.find(bufferLocation->second) };
        auto& buffer{ bufferKeyValue->second };
        std::visit(buffer_detach_fn, buffer);
        m_buffers.erase(bufferKeyValue);
    }

    auto ptr{ std::make_shared<GenericBuffer>(
        GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), GL_STATIC_DRAW, indices) };
    ptr->bind();

    unbind();
    ptr->unbind();

    auto key{ m_key++ };
    m_attributesMap[MeshAttributes::Indices] = key;
    m_buffers[key] = ptr;

    m_indexType = GL_UNSIGNED_INT;
    m_primitiveType = primitiveType;
}

void Mesh::set_attribute(const std::string& name, VertexBufferVariant attribute)
{
    bind();

    auto buffer_location{ m_attributes_string_map.find(name) };
    if (buffer_location != m_attributes_string_map.end()) {
        auto buffer_key_value{ m_buffers.find(buffer_location->second) };
        auto& buffer{ buffer_key_value->second };
        std::visit(buffer_detach_fn, buffer);
        m_buffers.erase(buffer_key_value);
    }

    std::visit(buffer_bind_fn, attribute);
    std::visit(buffer_attach_fn, attribute);

    unbind();
    std::visit(buffer_unbind_fn, attribute);

    auto key{ m_key++ };
    m_attributes_string_map[name] = key;
    m_buffers[key] = std::move(attribute);
}

void Mesh::set_simple_attribute(const std::string& name, GLuint index, GLint element_size, GLenum element_type,
    GLboolean normalized, GLsizei stride, const void* offset, GLsizeiptr size, GLenum usage, const void* data)
{
    auto attribute{ std::make_shared<VertexAttributeBuffer>(
        index, element_size, element_type, normalized, stride, offset, size, usage, data) };
    set_attribute(name, std::move(attribute));
}

void Mesh::set_complex_attribute(const std::string& name, std::span<const VertexAttributeDesc> vertex_attributes,
    GLsizeiptr size, GLenum usage, const void* data)
{
    auto attribute{ std::make_shared<ComplexVertexAttributeBuffer>(vertex_attributes, size, usage, data) };
    set_attribute(name, std::move(attribute));
}

VertexBufferVariant& Mesh::get_vertex_buffer(MeshAttributes type)
{
    assert(m_attributesMap.contains(type));
    return m_buffers.at(m_attributesMap.at(type));
}

const VertexBufferVariant& Mesh::get_vertex_buffer(MeshAttributes type) const
{
    assert(m_attributesMap.contains(type));
    return m_buffers.at(m_attributesMap.at(type));
}

VertexBufferVariant& Mesh::get_vertex_buffer(const std::string& name)
{
    assert(m_attributes_string_map.contains(name));
    return m_buffers.at(m_attributes_string_map.at(name));
}

const VertexBufferVariant& Mesh::get_vertex_buffer(const std::string& name) const
{
    assert(m_attributes_string_map.contains(name));
    return m_buffers.at(m_attributes_string_map.at(name));
}

void Mesh::bind() const { glBindVertexArray(m_arrayObject); }

void Mesh::unbind() const { glBindVertexArray(0); }

void Mesh::set_num_instances(GLsizei instances) { m_instances = instances; }

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

GLsizei Mesh::instances() const { return m_instances; }

void Mesh::free()
{
    m_attributesMap.clear();
    m_attributes_string_map.clear();
    m_buffers.clear();
    glDeleteVertexArrays(1, &m_arrayObject);
}

}
