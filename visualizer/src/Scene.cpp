#include <visualizer/Scene.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string_view>
#include <vector>

#include <visualizer/ActiveCameraSwitcher.hpp>
#include <visualizer/AssetDatabase.hpp>
#include <visualizer/Camera.hpp>
#include <visualizer/CameraSwitchingSystem.hpp>
#include <visualizer/CameraTypeSwitchingSystem.hpp>
#include <visualizer/CompositingSystem.hpp>
#include <visualizer/Composition.hpp>
#include <visualizer/Cube.hpp>
#include <visualizer/CubeMovementSystem.hpp>
#include <visualizer/EntityManager.hpp>
#include <visualizer/FixedCamera.hpp>
#include <visualizer/FixedCameraMovementSystem.hpp>
#include <visualizer/FreeFly.hpp>
#include <visualizer/FreeFlyCameraMovementSystem.hpp>
#include <visualizer/Iteration.hpp>
#include <visualizer/MeshDrawingSystem.hpp>
#include <visualizer/Parent.hpp>
#include <visualizer/SystemManager.hpp>

namespace Visualizer {

std::shared_ptr<Visualizer::ShaderProgram> createShaderProgram(
    const std::filesystem::path& vs, const std::filesystem::path& fs)
{
    auto vertexShader{ Visualizer::Shader::create(vs, Visualizer::ShaderType::VertexShader) };
    if (!vertexShader) {
        return nullptr;
    }

    auto fragmentShader{ Visualizer::Shader::create(fs, Visualizer::ShaderType::FragmentShader) };
    if (!fragmentShader) {
        return nullptr;
    }

    return Visualizer::ShaderProgram::create(*vertexShader, *fragmentShader);
}

void initializeAsset(const std::string& name, const Visconfig::Assets::MeshAsset& asset)
{
    auto mesh{ std::make_shared<Mesh>() };

    std::vector<glm::vec4> vertices{};
    std::vector<GLuint> indices{};
    std::vector<glm::vec4> tex_coords0{};

    vertices.reserve(asset.vertices.size());
    indices.reserve(asset.indices.size());
    tex_coords0.reserve(asset.texture_coords0.size());

    for (auto& vertex : asset.vertices) {
        vertices.push_back(glm::make_vec4(vertex.data()));
    }

    for (auto& index : asset.indices) {
        indices.push_back(index);
    }

    for (auto& coord : asset.texture_coords0) {
        tex_coords0.emplace_back(glm::make_vec4(coord.data()));
    }

    mesh->setVertices(vertices.data(), static_cast<GLsizeiptr>(vertices.size()));
    mesh->setIndices(indices.data(), static_cast<GLsizeiptr>(indices.size()), GL_TRIANGLES);
    mesh->setTextureCoordinates0(tex_coords0.data(), static_cast<GLsizeiptr>(tex_coords0.size()));

    AssetDatabase::setAsset(name, { getTypeId<Mesh>(), std::move(mesh) });
}

void initializeAsset(const std::string& name, const Visconfig::Assets::TextureFileAsset& asset)
{
    auto texture{ Texture2D::fromFile(asset.path) };
    for (auto attribute : asset.attributes) {
        switch (attribute) {
        case Visconfig::Assets::TextureAttributes::MagnificationLinear:
            texture->addAttribute(TextureMagnificationFilter::Linear);
            break;
        case Visconfig::Assets::TextureAttributes::MinificationLinear:
            texture->addAttribute(TextureMinificationFilter::LinearMipmapLinear);
            break;
        case Visconfig::Assets::TextureAttributes::GenerateMipMaps:
            break;
        }
    }

    AssetDatabase::setAsset(name, { getTypeId<Texture2D>(), std::move(texture) });
}

void initializeAsset(const std::string& name, const Visconfig::Assets::TextureRawAsset& asset)
{
    auto texture{ std::make_shared<Texture2D>() };

    TextureFormat format{};
    TextureInternalFormat internalFormat{};

    switch (asset.format) {
    case Visconfig::Assets::TextureFormat::R:
        format = TextureFormat::R;
        internalFormat = TextureInternalFormat::Short;
        break;
    case Visconfig::Assets::TextureFormat::RG:
        format = TextureFormat::RG;
        internalFormat = TextureInternalFormat::Short;
        break;
    case Visconfig::Assets::TextureFormat::RGB:
        format = TextureFormat::RGB;
        internalFormat = TextureInternalFormat::Short;
        break;
    case Visconfig::Assets::TextureFormat::RGBA:
        format = TextureFormat::RGBA;
        internalFormat = TextureInternalFormat::Short;
        break;
    case Visconfig::Assets::TextureFormat::R8:
        format = TextureFormat::R;
        internalFormat = TextureInternalFormat::Byte;
        break;
    case Visconfig::Assets::TextureFormat::RGBA16F:
        format = TextureFormat::RGBA;
        internalFormat = TextureInternalFormat::Float16;
        break;
    }

    texture->copyData(format, internalFormat, 0, asset.width, asset.height, 0, nullptr);

    for (auto attribute : asset.attributes) {
        switch (attribute) {
        case Visconfig::Assets::TextureAttributes::MagnificationLinear:
            texture->addAttribute(TextureMagnificationFilter::Linear);
            break;
        case Visconfig::Assets::TextureAttributes::MinificationLinear:
            texture->addAttribute(TextureMinificationFilter::Linear);
            break;
        case Visconfig::Assets::TextureAttributes::GenerateMipMaps:
            break;
        }
    }

    AssetDatabase::setAsset(name, { getTypeId<Texture2D>(), texture });
}

void initializeAsset(const std::string& name, const Visconfig::Assets::ShaderAsset& asset)
{
    auto shader{ createShaderProgram(asset.vertex, asset.fragment) };
    AssetDatabase::setAsset(name, { getTypeId<ShaderProgram>(), shader });
}

void initializeAsset(const std::string& name, const Visconfig::Assets::FramebufferAsset& asset)
{
    auto framebuffer{ std::make_shared<Framebuffer>() };

    for (auto& attachment : asset.attachments) {
        auto attachmentAsset{ AssetDatabase::getAsset(attachment.asset) };

        FramebufferAttachment destination{};

        switch (attachment.destination) {
        case Visconfig::Assets::FramebufferDestination::Color0:
            destination = FramebufferAttachment::Color0;
            break;
        case Visconfig::Assets::FramebufferDestination::Color1:
            destination = FramebufferAttachment::Color1;
            break;
        case Visconfig::Assets::FramebufferDestination::Color2:
            destination = FramebufferAttachment::Color2;
            break;
        case Visconfig::Assets::FramebufferDestination::Color3:
            destination = FramebufferAttachment::Color3;
            break;
        case Visconfig::Assets::FramebufferDestination::Depth:
            destination = FramebufferAttachment::Depth;
            break;
        case Visconfig::Assets::FramebufferDestination::Stencil:
            destination = FramebufferAttachment::Stencil;
            break;
        case Visconfig::Assets::FramebufferDestination::DepthStencil:
            destination = FramebufferAttachment::DepthStencil;
            break;
        }

        switch (attachment.type) {
        case Visconfig::Assets::FramebufferType::Texture:
            framebuffer->attachBuffer(
                destination, std::static_pointer_cast<Texture2D>(std::const_pointer_cast<void>(attachmentAsset.data)));
            break;
        case Visconfig::Assets::FramebufferType::Renderbuffer:
            framebuffer->attachBuffer(destination,
                std::static_pointer_cast<Renderbuffer>(std::const_pointer_cast<void>(attachmentAsset.data)));
            break;
        }
    }

    AssetDatabase::setAsset(name, { getTypeId<Framebuffer>(), framebuffer });
}

void initializeAsset(const std::string& name, const Visconfig::Assets::DefaultFramebufferAsset&)
{
    AssetDatabase::setAsset(name, { getTypeId<Framebuffer>(), Framebuffer::defaultFramebufferPtr() });
}

void initializeAsset(const Visconfig::Asset& asset)
{
    switch (asset.type) {
    case Visconfig::Assets::AssetType::Mesh:
        initializeAsset(asset.name, *std::static_pointer_cast<const Visconfig::Assets::MeshAsset>(asset.data));
        break;
    case Visconfig::Assets::AssetType::TextureFile:
        initializeAsset(asset.name, *std::static_pointer_cast<const Visconfig::Assets::TextureFileAsset>(asset.data));
        break;
    case Visconfig::Assets::AssetType::TextureRaw:
        initializeAsset(asset.name, *std::static_pointer_cast<const Visconfig::Assets::TextureRawAsset>(asset.data));
        break;
    case Visconfig::Assets::AssetType::Shader:
        initializeAsset(asset.name, *std::static_pointer_cast<const Visconfig::Assets::ShaderAsset>(asset.data));
        break;
    case Visconfig::Assets::AssetType::Framebuffer:
        initializeAsset(asset.name, *std::static_pointer_cast<const Visconfig::Assets::FramebufferAsset>(asset.data));
        break;
    case Visconfig::Assets::AssetType::DefaultFramebuffer:
        initializeAsset(
            asset.name, *std::static_pointer_cast<const Visconfig::Assets::DefaultFramebufferAsset>(asset.data));
        break;
    }
}

void addEntity(
    EntityManager& entityManager, std::unordered_map<std::size_t, Entity>& entityIdMap, const Visconfig::Entity& entity)
{
    EntityArchetype archetype{};
    for (auto& component : entity.components) {
        switch (component.type) {
        case Visconfig::Components::ComponentType::Cube:
            archetype = EntityArchetype::with<Cube>(archetype);
            break;
        case Visconfig::Components::ComponentType::Mesh:
            archetype = EntityArchetype::with<std::shared_ptr<Mesh>>(archetype);
            break;
        case Visconfig::Components::ComponentType::Parent:
            archetype = EntityArchetype::with<Parent>(archetype);
            break;
        case Visconfig::Components::ComponentType::Material:
            archetype = EntityArchetype::with<Material>(archetype);
            break;
        case Visconfig::Components::ComponentType::Layer:
            archetype = EntityArchetype::with<RenderLayer>(archetype);
            break;
        case Visconfig::Components::ComponentType::Transform:
            archetype = EntityArchetype::with<Transform>(archetype);
            break;
        case Visconfig::Components::ComponentType::ImplicitIteration:
            archetype = EntityArchetype::with<HomogeneousIteration>(archetype);
            break;
        case Visconfig::Components::ComponentType::ExplicitIteration:
            archetype = EntityArchetype::with<HomogeneousIteration>(archetype);
            break;
        case Visconfig::Components::ComponentType::EntityActivation:
            archetype = EntityArchetype::with<EntityActivation>(archetype);
            break;
        case Visconfig::Components::ComponentType::ExplicitHeterogeneousIteration:
            archetype = EntityArchetype::with<HeterogeneousIteration>(archetype);
            break;
        case Visconfig::Components::ComponentType::Camera:
            archetype = EntityArchetype::with<Camera>(archetype);
            break;
        case Visconfig::Components::ComponentType::FreeFlyCamera:
            archetype = EntityArchetype::with<FreeFly>(archetype);
            break;
        case Visconfig::Components::ComponentType::FixedCamera:
            archetype = EntityArchetype::with<FixedCamera>(archetype);
            break;
        case Visconfig::Components::ComponentType::CameraSwitcher:
            archetype = EntityArchetype::with<ActiveCameraSwitcher>(archetype);
            break;
        case Visconfig::Components::ComponentType::Composition:
            archetype = EntityArchetype::with<Composition>(archetype);
            break;
        }
    }

    const auto [it, success] = entityIdMap.insert({ entity.id, entityManager.addEntity(archetype) });
    if (!success) {
        std::cerr << "Unable to insert into entity map" << std::endl;
    }
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::CubeComponent& component)
{
    (void)component;
    *static_cast<Cube*>(manager.getEntityComponentPointer(entity, getTypeId<Cube>())) = {};
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::MeshComponent& component)
{
    auto meshAsset{ std::static_pointer_cast<Mesh>(
        std::const_pointer_cast<void>(AssetDatabase::getAsset(component.asset).data)) };
    auto& mesh{ *static_cast<std::shared_ptr<Mesh>*>(
        manager.getEntityComponentPointer(entity, getTypeId<std::shared_ptr<Mesh>>())) };
    mesh = std::move(meshAsset);
}

void initializeComponent(ComponentManager& manager, Entity entity,
    const Visconfig::Components::ParentComponent& component, const std::unordered_map<std::size_t, Entity>& entityIdMap)
{
    auto parent{ entityIdMap.at(component.id) };
    *static_cast<Parent*>(manager.getEntityComponentPointer(entity, getTypeId<Parent>())) = Parent{ parent };
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::BoolMaterialAttribute& attribute)
{
    env.set(name, static_cast<GLboolean>(attribute.value));
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::BoolArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, static_cast<GLboolean>(attribute.value[i]), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::IntMaterialAttribute& attribute)
{
    env.set(name, static_cast<GLint>(attribute.value));
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::IntArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, static_cast<GLint>(attribute.value[i]), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::UIntMaterialAttribute& attribute)
{
    env.set(name, static_cast<GLuint>(attribute.value));
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::UIntArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, static_cast<GLuint>(attribute.value[i]), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::FloatMaterialAttribute& attribute)
{
    env.set(name, static_cast<GLfloat>(attribute.value));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::FloatArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, static_cast<GLfloat>(attribute.value[i]), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::BVec2MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec2(attribute.value.data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::BVec2ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec2(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::BVec3MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec3(attribute.value.data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::BVec3ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec3(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::BVec4MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec4(attribute.value.data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::BVec4ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec4(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::IVec2MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec2(attribute.value.data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::IVec2ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec2(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::IVec3MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec3(attribute.value.data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::IVec3ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec3(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::IVec4MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec4(attribute.value.data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::IVec4ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec4(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::UVec2MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec2(attribute.value.data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::UVec2ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec2(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::UVec3MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec3(attribute.value.data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::UVec3ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec3(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::UVec4MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec4(attribute.value.data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::UVec4ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec4(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Vec2MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec2(attribute.value.data()));
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Vec2ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec2(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Vec3MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec3(attribute.value.data()));
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Vec3ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec3(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Vec4MaterialAttribute& attribute)
{
    env.set(name, glm::make_vec4(attribute.value.data()));
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Vec4ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_vec4(attribute.value[i].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Mat2x2MaterialAttribute& attribute)
{
    env.set(name, glm::make_mat2x2(attribute.value[0].data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::Mat2x2ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_mat2x2(attribute.value[i][0].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Mat2x3MaterialAttribute& attribute)
{
    env.set(name, glm::make_mat2x3(attribute.value[0].data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::Mat2x3ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_mat2x3(attribute.value[i][0].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Mat2x4MaterialAttribute& attribute)
{
    env.set(name, glm::make_mat2x4(attribute.value[0].data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::Mat2x4ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_mat2x4(attribute.value[i][0].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Mat3x2MaterialAttribute& attribute)
{
    env.set(name, glm::make_mat3x2(attribute.value[0].data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::Mat3x2ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_mat3x2(attribute.value[i][0].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Mat3x3MaterialAttribute& attribute)
{
    env.set(name, glm::make_mat3x3(attribute.value[0].data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::Mat3x3ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_mat3x3(attribute.value[i][0].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Mat3x4MaterialAttribute& attribute)
{
    env.set(name, glm::make_mat3x4(attribute.value[0].data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::Mat3x4ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_mat3x4(attribute.value[i][0].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Mat4x2MaterialAttribute& attribute)
{
    env.set(name, glm::make_mat4x2(attribute.value[0].data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::Mat4x2ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_mat4x2(attribute.value[i][0].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Mat4x3MaterialAttribute& attribute)
{
    env.set(name, glm::make_mat4x3(attribute.value[0].data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::Mat4x3ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_mat4x3(attribute.value[i][0].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Mat4x4MaterialAttribute& attribute)
{
    env.set(name, glm::make_mat4x4(attribute.value[0].data()));
}

void initializeMaterialAttribute(ShaderEnvironment& env, const std::string& name,
    const Visconfig::Components::Mat4x4ArrayMaterialAttribute& attribute)
{
    for (std::size_t i{ 0 }; i < attribute.value.size(); ++i) {
        env.set(name, glm::make_mat4x4(attribute.value[i][0].data()), i);
    }
}

void initializeMaterialAttribute(
    ShaderEnvironment& env, const std::string& name, const Visconfig::Components::Sampler2DMaterialAttribute& attribute)
{
    auto textureAsset{ std::static_pointer_cast<Texture2D>(
        std::const_pointer_cast<void>(AssetDatabase::getAsset(attribute.asset).data)) };
    auto slot{ static_cast<TextureSlot>(attribute.slot) };

    auto sampler{ TextureSampler<Texture2D>{ textureAsset, slot } };
    env.set(name, sampler);
}

void initializeMaterialAttribute(
    Material& material, const std::string& name, const Visconfig::Components::MaterialAttribute& attribute)
{
    switch (attribute.type) {
    case Visconfig::Components::MaterialAttributeType::Bool:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BoolMaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BoolArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Int:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IntMaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IntArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::UInt:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UIntMaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UIntArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Float:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::FloatMaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::FloatArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::BVec2:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::BVec3:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::BVec4:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::IVec2:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::IVec3:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::IVec4:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::UVec2:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::UVec3:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::UVec4:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Vec2:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Vec3:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Vec4:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat2x2:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat2x3:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat2x4:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat3x2:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat3x3:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat3x4:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat4x2:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat4x3:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat4x4:
        if (!attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Sampler2D:
        initializeMaterialAttribute(material.m_materialVariables, name,
            *std::static_pointer_cast<const Visconfig::Components::Sampler2DMaterialAttribute>(attribute.data));
        break;
    }
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::MaterialComponent& component)
{
    auto shaderAsset{ std::static_pointer_cast<ShaderProgram>(
        std::const_pointer_cast<void>(AssetDatabase::getAsset(component.asset).data)) };
    auto material{ Material{
        ShaderEnvironment{ *shaderAsset, ParameterQualifier::Material }, std::move(shaderAsset) } };

    for (auto& attribute : component.attributes) {
        initializeMaterialAttribute(material, attribute.first, attribute.second);
    }

    *static_cast<Material*>(manager.getEntityComponentPointer(entity, getTypeId<Material>())) = std::move(material);
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::LayerComponent& component)
{
    *static_cast<RenderLayer*>(manager.getEntityComponentPointer(entity, getTypeId<RenderLayer>()))
        = RenderLayer{ component.mask };
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::TransformComponent& component)
{
    *static_cast<Transform*>(manager.getEntityComponentPointer(entity, getTypeId<Transform>())) = Transform{
        glm::quat{ glm::make_vec3(component.rotation) },
        glm::make_vec3(component.position),
        glm::make_vec3(component.scale),
    };
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::ImplicitIterationComponent& component)
{
    std::vector<glm::vec3> positions{};
    positions.reserve(component.numIterations[0] * component.numIterations[1] * component.numIterations[2]);

    glm::uvec3 order{};
    switch (component.order) {
    case Visconfig::Components::IterationOrder::XYZ:
        order = { 0, 1, 2 };
        break;
    case Visconfig::Components::IterationOrder::XZY:
        order = { 0, 2, 1 };
        break;
    case Visconfig::Components::IterationOrder::YXZ:
        order = { 1, 0, 2 };
        break;
    case Visconfig::Components::IterationOrder::YZX:
        order = { 2, 0, 1 };
        break;
    case Visconfig::Components::IterationOrder::ZXY:
        order = { 1, 2, 0 };
        break;
    case Visconfig::Components::IterationOrder::ZYX:
        order = { 2, 1, 0 };
        break;
    }

    glm::uvec3 position{ 0, 0, 0 };
    positions.push_back(position);

    while (position[order[0]] != component.numIterations[order[0]]
        || position[order[1]] != component.numIterations[order[1]]
        || position[order[2]] != component.numIterations[order[2]]) {
        if (position[order[0]] < component.numIterations[order[0]]) {
            position[order[0]]++;
        } else {
            position[order[0]] = 0;
            if (position[order[1]] < component.numIterations[order[1]]) {
                position[order[1]]++;
            } else {
                position[order[1]] = 0;
                if (position[order[2]] < component.numIterations[order[2]]) {
                    position[order[2]]++;
                } else {
                    position[order[2]] = 0;
                }
            }
        }

        positions.push_back(position);
    }

    *static_cast<HomogeneousIteration*>(manager.getEntityComponentPointer(entity, getTypeId<HomogeneousIteration>()))
        = HomogeneousIteration{ std::move(positions), component.ticksPerIteration, 0, 0 };
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::ExplicitIterationComponent& component)
{
    std::vector<glm::vec3> positions{};
    positions.reserve(component.positions.size());

    for (auto& pos : component.positions) {
        positions.push_back(glm::make_vec3(pos.data()));
    }

    *static_cast<HomogeneousIteration*>(manager.getEntityComponentPointer(entity, getTypeId<HomogeneousIteration>()))
        = HomogeneousIteration{ std::move(positions), component.ticksPerIteration, 0, 0 };
}

void initializeComponent(ComponentManager& manager, Entity entity,
    const Visconfig::Components::EntityActivationComponent& component,
    const std::unordered_map<std::size_t, Entity>& entityIdMap)
{
    std::vector<Entity> entities{};
    entities.reserve(component.entities.size());

    std::vector<std::size_t> ticks{};
    ticks.reserve(component.ticksPerIteration.size());

    for (auto entityId : component.entities) {
        entities.push_back(entityIdMap.at(entityId));
    }

    for (auto tick : component.ticksPerIteration) {
        ticks.push_back(tick);
    }

    auto& entityActivation{ *static_cast<EntityActivation*>(
        manager.getEntityComponentPointer(entity, getTypeId<EntityActivation>())) };
    entityActivation.entities = entities;
    entityActivation.ticksPerIteration = ticks;
    entityActivation.layer.m_layerMask = component.layer;
    entityActivation.index = 0;
    entityActivation.tick = 0;
}

void initializeComponent(ComponentManager& manager, Entity entity,
    const Visconfig::Components::ExplicitHeterogeneousIterationComponent& component)
{
    std::vector<glm::vec3> scales{};
    std::vector<glm::vec3> positions{};
    std::vector<std::size_t> ticksPerIteration{};
    scales.reserve(component.scales.size());
    positions.reserve(component.positions.size());
    ticksPerIteration.reserve(component.ticksPerIteration.size());

    for (auto& scale : component.scales) {
        scales.push_back(glm::make_vec3(scale.data()));
    }
    for (auto& pos : component.positions) {
        positions.push_back(glm::make_vec3(pos.data()));
    }
    for (auto& ticks : component.ticksPerIteration) {
        ticksPerIteration.push_back(ticks);
    }

    *static_cast<HeterogeneousIteration*>(
        manager.getEntityComponentPointer(entity, getTypeId<HeterogeneousIteration>()))
        = HeterogeneousIteration{ std::move(scales), std::move(positions), std::move(ticksPerIteration), 0, 0 };
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::CameraComponent& component)
{
    std::unordered_map<std::string, std::shared_ptr<Framebuffer>> targets{};

    for (auto& target : component.targets) {
        auto framebufferAsset{ std::static_pointer_cast<Framebuffer>(
            std::const_pointer_cast<void>(AssetDatabase::getAsset(target.second).data)) };
        targets.insert_or_assign(target.first, std::move(framebufferAsset));
    }

    *static_cast<Camera*>(manager.getEntityComponentPointer(entity, getTypeId<Camera>())) = Camera{ component.active,
        component.fixed, RenderLayer{ component.layerMask.to_ullong() }, nullptr, std::move(targets) };
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::FreeFlyCameraComponent& component)
{
    (void)component;
    *static_cast<FreeFly*>(manager.getEntityComponentPointer(entity, getTypeId<FreeFly>())) = {};
}

void initializeComponent(ComponentManager& manager, Entity entity,
    const Visconfig::Components::FixedCameraComponent& component,
    const std::unordered_map<std::size_t, Entity>& entityIdMap)
{
    auto& camera{ *static_cast<FixedCamera*>(manager.getEntityComponentPointer(entity, getTypeId<FixedCamera>())) };

    camera.focus = entityIdMap.at(component.focus);
    camera.distance = component.distance;
    camera.horizontalAngle = component.horizontalAngle;
    camera.verticalAngle = camera.verticalAngle + (glm::pi<float>() / 2);
}

void initializeComponent(ComponentManager& manager, Entity entity,
    const Visconfig::Components::CameraSwitcherComponent& component,
    const std::unordered_map<std::size_t, Entity>& entityIdMap)
{
    auto& switcher{ *static_cast<ActiveCameraSwitcher*>(
        manager.getEntityComponentPointer(entity, getTypeId<ActiveCameraSwitcher>())) };

    for (auto id : component.cameras) {
        switcher.cameras.push_back(entityIdMap.at(id));
    }

    switcher.current = component.active;
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::CompositionComponent& component)
{
    std::vector<CompositionOperation> operations{};
    operations.reserve(component.operations.size());

    for (auto& operation : component.operations) {

        auto position{ glm::make_vec2(operation.position) };
        auto scale{ glm::make_vec2(operation.scale) };

        std::vector<std::shared_ptr<Texture2D>> sources{};

        for (const auto& src : operation.sourceTexture) {
            auto sourceTextureAsset{ std::static_pointer_cast<Texture2D>(
                std::const_pointer_cast<void>(AssetDatabase::getAsset(src).data)) };
            sources.push_back(std::move(sourceTextureAsset));
        }

        auto shaderAsset{ std::static_pointer_cast<ShaderProgram>(
            std::const_pointer_cast<void>(AssetDatabase::getAsset(operation.shader).data)) };
        auto material{ Material{
            ShaderEnvironment{ *shaderAsset, ParameterQualifier::Program }, std::move(shaderAsset) } };

        auto framebufferAsset{ std::static_pointer_cast<Framebuffer>(
            std::const_pointer_cast<void>(AssetDatabase::getAsset(operation.target).data)) };

        operations.push_back({ std::move(material),
            Transform{ glm::identity<glm::quat>(), glm::vec3{ position, 0 }, glm::vec3{ scale, 0 } },
            std::move(sources), std::move(framebufferAsset) });
    }

    *static_cast<Composition*>(manager.getEntityComponentPointer(entity, getTypeId<Composition>()))
        = Composition{ std::move(operations) };
}

void initializeEntity(ComponentManager& manager, const std::unordered_map<std::size_t, Entity>& entityIdMap,
    const Visconfig::Entity& entity)
{
    auto ecs_entity{ entityIdMap.at(entity.id) };

    for (auto& component : entity.components) {
        switch (component.type) {
        case Visconfig::Components::ComponentType::Cube:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::CubeComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::Mesh:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::MeshComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::Parent:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::ParentComponent>(component.data), entityIdMap);
            break;
        case Visconfig::Components::ComponentType::Material:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::MaterialComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::Layer:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::LayerComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::Transform:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::TransformComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::ImplicitIteration:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::ImplicitIterationComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::ExplicitIteration:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::ExplicitIterationComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::EntityActivation:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::EntityActivationComponent>(component.data),
                entityIdMap);
            break;
        case Visconfig::Components::ComponentType::ExplicitHeterogeneousIteration:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::ExplicitHeterogeneousIterationComponent>(
                    component.data));
            break;
        case Visconfig::Components::ComponentType::Camera:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::CameraComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::FreeFlyCamera:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::FreeFlyCameraComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::FixedCamera:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::FixedCameraComponent>(component.data),
                entityIdMap);
            break;
        case Visconfig::Components::ComponentType::CameraSwitcher:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::CameraSwitcherComponent>(component.data),
                entityIdMap);
            break;
        case Visconfig::Components::ComponentType::Composition:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::CompositionComponent>(component.data));
            break;
        }
    }
}

World initializeWorld(const Visconfig::World& world)
{
    World ecsWorld{};

    std::unordered_map<std::size_t, Entity> entityIdMap{};

    auto componentManager{ ecsWorld.addManager<ComponentManager>() };
    auto entityManager{ ecsWorld.addManager<EntityManager>() };

    for (auto& entity : world.entities) {
        addEntity(*entityManager, entityIdMap, entity);
    }

    for (auto& entity : world.entities) {
        initializeEntity(*componentManager, entityIdMap, entity);
    }

    auto systemManager{ ecsWorld.addManager<SystemManager>() };

    systemManager->addSystem<CubeMovementSystem>("tick"sv);
    systemManager->addSystem<CameraSwitchingSystem>("tick"sv);
    systemManager->addSystem<CameraTypeSwitchingSystem>("tick"sv);
    systemManager->addSystem<FreeFlyCameraMovementSystem>("tick"sv);
    systemManager->addSystem<FixedCameraMovementSystem>("tick"sv);

    systemManager->addSystem<MeshDrawingSystem>("draw"sv);
    systemManager->addSystem<CompositingSystem>("composite"sv);

    return ecsWorld;
}

Scene initializeScene(const Visconfig::Config& config)
{
    for (auto& asset : config.assets) {
        initializeAsset(asset);
    }

    Scene scene{};
    scene.activeWorld = 0;

    for (auto& world : config.worlds) {
        scene.worlds.push_back(initializeWorld(world));
    }

    return scene;
}

void tick(Scene& scene)
{
    using namespace std::literals;

    for (auto& world : scene.worlds) {
        auto systemManager{ world.getManager<SystemManager>() };
        systemManager->run("tick"sv);
    }
}

void draw(const Scene& scene)
{
    using namespace std::literals;

    for (auto& world : scene.worlds) {
        auto systemManager{ world.getManager<SystemManager>() };
        systemManager->run("draw"sv);
        systemManager->run("post-process"sv);
    }

    for (auto& world : scene.worlds) {
        auto systemManager{ world.getManager<SystemManager>() };
        systemManager->run("composite"sv);
    }
}

}