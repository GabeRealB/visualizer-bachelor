#include <visualizer/Scene.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include <visualizer/AssetDatabase.hpp>
#include <visualizer/Camera.hpp>
#include <visualizer/CameraInitializationSystem.hpp>
#include <visualizer/CameraMovementSystem.hpp>
#include <visualizer/CompositingSystem.hpp>
#include <visualizer/Composition.hpp>
#include <visualizer/Cube.hpp>
#include <visualizer/CubeInitializationSystem.hpp>
#include <visualizer/CubeMovementSystem.hpp>
#include <visualizer/Framebuffer.hpp>
#include <visualizer/FreeFly.hpp>
#include <visualizer/Iteration.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/MeshDrawingSystem.hpp>
#include <visualizer/Parent.hpp>
#include <visualizer/RenderLayer.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/SystemManager.hpp>
#include <visualizer/Texture.hpp>

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

std::optional<Scene> loadScene(const VisualizerConfiguration& conf)
{
    using namespace std::literals;

    if (conf.cubes.size() == 0 || conf.cubes.size() != conf.views.size()) {
        return std::nullopt;
    }

    Scene scene{};
    scene.activeWorld = 0;
    scene.worlds.reserve(conf.cubes.size());

    std::vector<std::shared_ptr<Texture2D>> renderTargets;
    renderTargets.reserve(conf.cubes.size());

    for (std::size_t i = 0; i < renderTargets.capacity(); ++i) {
        auto texture{ std::make_shared<Texture2D>() };
        texture->addAttribute(TextureMinificationFilter::Linear);
        texture->addAttribute(TextureMagnificationFilter::Linear);
        texture->copyData(TextureFormat::RGBA, TextureInternalFormat::Short, 0,
            static_cast<GLsizei>(conf.resolution[0]), static_cast<GLsizei>(conf.resolution[1]), 0, nullptr);
        renderTargets.push_back(texture);
    }

    auto cubeShaderProgram{ createShaderProgram("shader/cube.vs.glsl", "shader/cube.fs.glsl") };
    if (cubeShaderProgram == nullptr) {
        std::cerr << "ERROR: Could not create the cube shader program!" << std::endl;
        return std::nullopt;
    }

    scene.worlds.emplace_back();
    auto& world{ scene.worlds.back() };
    world.addManager<ComponentManager>();
    world.addManager<EntityManager>();
    auto systemManager{ world.addManager<SystemManager>() };

    systemManager->addSystem<CubeInitializationSystem>("initialization"sv);
    systemManager->addSystem<CameraInitializationSystem>("initialization"sv);

    systemManager->addSystem<CubeMovementSystem>("tick"sv);
    systemManager->addSystem<CameraMovementSystem>("tick"sv);

    systemManager->addSystem<MeshDrawingSystem>("draw"sv);
    systemManager->addSystem<CompositingSystem>("composite"sv, conf, renderTargets);

    SystemParameterMap initParameters{};
    CubeInitializationSystem::Data cubeInitData{ conf, cubeShaderProgram };
    CameraInitializationSystem::Data cameraInitData{ renderTargets };
    initParameters.insert<CubeInitializationSystem>(&cubeInitData);
    initParameters.insert<CameraInitializationSystem>(&cameraInitData);
    systemManager->run("initialization"sv, initParameters);

    return scene;
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
            texture->addAttribute(TextureMinificationFilter::Linear);
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
    case Visconfig::Assets::TextureFormat::RGB:
        format = TextureFormat::RGB;
        internalFormat = TextureInternalFormat::Short;
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

    AssetDatabase::setAsset(name, { getTypeId<Texture2D>(), std::move(texture) });
}

void initializeAsset(const std::string& name, const Visconfig::Assets::ShaderAsset& asset)
{
    auto shader{ createShaderProgram(asset.vertex, asset.fragment) };
    AssetDatabase::setAsset(name, { getTypeId<ShaderProgram>(), std::move(shader) });
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

    AssetDatabase::setAsset(name, { getTypeId<Framebuffer>(), std::move(framebuffer) });
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
    EntityManager& manager, std::unordered_map<std::size_t, Entity>& entityIdMap, const Visconfig::Entity& entity)
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
            archetype = EntityArchetype::with<Iteration>(archetype);
            break;
        case Visconfig::Components::ComponentType::ExplicitIteration:
            archetype = EntityArchetype::with<Iteration>(archetype);
            break;
        case Visconfig::Components::ComponentType::Camera:
            archetype = EntityArchetype::with<Camera>(archetype);
            break;
        case Visconfig::Components::ComponentType::FreeFlyCamera:
            archetype = EntityArchetype::with<FreeFly>(archetype);
            break;
        case Visconfig::Components::ComponentType::Composition:
            archetype = EntityArchetype::with<Composition>(archetype);
            break;
        }
    }

    entityIdMap.insert_or_assign(entity.id, manager.addEntity(archetype));
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
    *static_cast<std::shared_ptr<Mesh>*>(manager.getEntityComponentPointer(entity, getTypeId<std::shared_ptr<Mesh>>()))
        = std::move(meshAsset);
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
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BoolMaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BoolArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Int:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IntMaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IntArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::UInt:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UIntMaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UIntArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Float:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::FloatMaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::FloatArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::BVec2:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::BVec3:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::BVec4:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::BVec4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::IVec2:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::IVec3:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::IVec4:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::IVec4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::UVec2:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::UVec3:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::UVec4:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::UVec4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Vec2:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Vec3:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Vec4:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Vec4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat2x2:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat2x3:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat2x4:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat2x4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat3x2:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat3x3:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat3x4:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x4MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat3x4ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat4x2:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x2MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x2ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat4x3:
        if (attribute.isArray) {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x3MaterialAttribute>(attribute.data));
        } else {
            initializeMaterialAttribute(material.m_materialVariables, name,
                *std::static_pointer_cast<const Visconfig::Components::Mat4x3ArrayMaterialAttribute>(attribute.data));
        }
        break;
    case Visconfig::Components::MaterialAttributeType::Mat4x4:
        if (attribute.isArray) {
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

    /*
     * TODO: Populate positions
     */

    *static_cast<Iteration*>(manager.getEntityComponentPointer(entity, getTypeId<Iteration>()))
        = Iteration{ std::move(positions), component.ticksPerIteration, 0 };
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::ExplicitIterationComponent& component)
{
    std::vector<glm::vec3> positions{};
    positions.reserve(component.positions.size());

    for (auto& pos : component.positions) {
        positions.push_back(glm::make_vec3(pos.data()));
    }

    *static_cast<Iteration*>(manager.getEntityComponentPointer(entity, getTypeId<Iteration>()))
        = Iteration{ std::move(positions), component.ticksPerIteration, 0 };
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

    *static_cast<Camera*>(manager.getEntityComponentPointer(entity, getTypeId<Camera>()))
        = Camera{ true, RenderLayer{ component.layerMask.to_ullong() }, nullptr, std::move(targets) };
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::FreeFlyCameraComponent& component)
{
    (void)component;
    *static_cast<FreeFly*>(manager.getEntityComponentPointer(entity, getTypeId<FreeFly>())) = {};
}

void initializeComponent(
    ComponentManager& manager, Entity entity, const Visconfig::Components::CompositionComponent& component)
{
    std::vector<CompositionOperation> operations{};
    operations.reserve(component.operations.size());

    for (auto& operation : component.operations) {

        auto position{ glm::make_vec2(operation.position) };
        auto scale{ glm::make_vec2(operation.scale) };

        auto sourceTextureAsset{ std::static_pointer_cast<Texture2D>(
            std::const_pointer_cast<void>(AssetDatabase::getAsset(operation.sourceTexture).data)) };
        auto framebufferAsset{ std::static_pointer_cast<Framebuffer>(
            std::const_pointer_cast<void>(AssetDatabase::getAsset(operation.target).data)) };

        operations.push_back({ Transform{ glm::identity<glm::quat>(), glm::vec3{ position, 0 }, glm::vec3{ scale, 0 } },
            std::move(sourceTextureAsset), std::move(framebufferAsset) });
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
        case Visconfig::Components::ComponentType::Camera:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::CameraComponent>(component.data));
            break;
        case Visconfig::Components::ComponentType::FreeFlyCamera:
            initializeComponent(manager, ecs_entity,
                *std::static_pointer_cast<const Visconfig::Components::FreeFlyCameraComponent>(component.data));
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
    systemManager->addSystem<CameraMovementSystem>("tick"sv);

    systemManager->addSystem<MeshDrawingSystem>("draw"sv);

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