#include <visualizer/CubeInitializationSystem.hpp>

#include <visualizer/Camera.hpp>
#include <visualizer/CubeTickInfo.hpp>
#include <visualizer/Parent.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

std::shared_ptr<Mesh> generateCubeMesh()
{
    auto mesh{ std::make_shared<Mesh>() };

    constexpr glm::vec4 vertices[]{
        { -0.5f, -0.5f, 0.5f, 1.0f }, // lower-left-front
        { 0.5f, -0.5f, 0.5f, 1.0f }, // lower-right-front
        { 0.5f, 0.5f, 0.5f, 1.0f }, // top-right-front
        { -0.5f, 0.5f, 0.5f, 1.0f }, // top-left-front

        { -0.5f, -0.5f, -0.5f, 1.0f }, // lower-left-back
        { 0.5f, -0.5f, -0.5f, 1.0f }, // lower-right-back
        { 0.5f, 0.5f, -0.5f, 1.0f }, // top-right-back
        { -0.5f, 0.5f, -0.5f, 1.0f }, // top-left-back
    };

    constexpr glm::vec4 flattenedVertices[]{
        // front
        vertices[0],
        vertices[1],
        vertices[2],
        vertices[0],
        vertices[2],
        vertices[3],

        // top
        vertices[3],
        vertices[2],
        vertices[6],
        vertices[3],
        vertices[6],
        vertices[7],

        // right
        vertices[1],
        vertices[5],
        vertices[6],
        vertices[1],
        vertices[6],
        vertices[2],

        // left
        vertices[4],
        vertices[0],
        vertices[3],
        vertices[4],
        vertices[3],
        vertices[7],

        // bottom
        vertices[4],
        vertices[5],
        vertices[1],
        vertices[4],
        vertices[1],
        vertices[0],

        // back
        vertices[5],
        vertices[4],
        vertices[7],
        vertices[5],
        vertices[7],
        vertices[6],
    };

    constexpr glm::vec3 texCoords[]{
        { 0.0f, 0.0f, 0.0f }, // lower-left
        { 1.0f, 0.0f, 0.0f }, // lower-right
        { 1.0f, 1.0f, 0.0f }, // top-right
        { 1.0f, 0.0f, 0.0f }, // top-left
    };

    constexpr glm::vec4 flattenedTexCoords[]{
        // front
        { texCoords[0], 0.0f },
        { texCoords[1], 0.0f },
        { texCoords[2], 0.0f },
        { texCoords[0], 0.0f },
        { texCoords[2], 0.0f },
        { texCoords[3], 0.0f },

        // top
        { texCoords[0], 1.0f },
        { texCoords[1], 1.0f },
        { texCoords[2], 1.0f },
        { texCoords[0], 1.0f },
        { texCoords[2], 1.0f },
        { texCoords[3], 1.0f },

        // right
        { texCoords[0], 2.0f },
        { texCoords[1], 2.0f },
        { texCoords[2], 2.0f },
        { texCoords[0], 2.0f },
        { texCoords[2], 2.0f },
        { texCoords[3], 2.0f },

        // left
        { texCoords[0], 2.0f },
        { texCoords[1], 2.0f },
        { texCoords[2], 2.0f },
        { texCoords[0], 2.0f },
        { texCoords[2], 2.0f },
        { texCoords[3], 2.0f },

        // bottom
        { texCoords[0], 1.0f },
        { texCoords[1], 1.0f },
        { texCoords[2], 1.0f },
        { texCoords[0], 1.0f },
        { texCoords[2], 1.0f },
        { texCoords[3], 1.0f },

        // back
        { texCoords[0], 0.0f },
        { texCoords[1], 0.0f },
        { texCoords[2], 0.0f },
        { texCoords[0], 0.0f },
        { texCoords[2], 0.0f },
        { texCoords[3], 0.0f },
    };

    constexpr GLuint indices[]{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };

    mesh->setVertices(flattenedVertices, sizeof(flattenedVertices) / sizeof(glm::vec4));
    mesh->setTextureCoordinates0(flattenedTexCoords, sizeof(flattenedTexCoords) / sizeof(glm::vec4));
    mesh->setIndices(indices, sizeof(indices) / sizeof(GLuint), GL_TRIANGLES);
    return mesh;
}

CubeInitializationSystem::CubeInitializationSystem()
    : m_cubeMesh{ generateCubeMesh() }
    , m_cubeArchetype{ EntityArchetype::create<Transform, std::shared_ptr<Mesh>, Material, RenderLayer>() }
    , m_childrenArchetype{ EntityArchetype::with<CubeTickInfo, Parent>(m_cubeArchetype) }
    , m_cubeQuery{ m_cubeArchetype }
    , m_childrenQuery{ m_childrenArchetype }
    , m_cubeTexture{ Texture2D::fromFile("textures/square.png") }
    , m_entityManager{}
    , m_componentManager{}
{
    m_cubeTexture->addAttribute(TextureMinificationFilter::Linear);
    m_cubeTexture->addAttribute(TextureMagnificationFilter::Linear);
}

void CubeInitializationSystem::initialize()
{
    m_entityManager = m_world->getManager<EntityManager>();
    m_componentManager = m_world->getManager<ComponentManager>();
}

void CubeInitializationSystem::terminate()
{
    m_entityManager = nullptr;
    m_componentManager = nullptr;
}

void CubeInitializationSystem::run(void* data)
{
    if (data) {
        run(*static_cast<Data*>(data));
    }
}

void CubeInitializationSystem::run(CubeInitializationSystem::Data& data)
{
    for (std::size_t i = 0; i < data.m_config.cubes.size(); i++) {
        std::vector<const InnerCube*> innerCubes{};
        std::vector<Transform> transforms{};
        std::vector<CubeTickInfo> tickInfos{};
        std::vector<glm::vec4> colors{};
        std::vector<glm::vec3> scales{};

        for (auto child{ static_cast<const InnerCube*>(&data.m_config.cubes[i]) }; child != nullptr;
             child = child->innerCube.get()) {
            innerCubes.push_back(child);
            colors.emplace_back(glm::vec3(child->color[0], child->color[1], child->color[2]) / 255.0f, 0.6f);
        }

        transforms.reserve(innerCubes.size());
        tickInfos.reserve(innerCubes.size() - 1);
        scales.reserve(innerCubes.size());
        for (std::size_t j = 0; j < innerCubes.size(); ++j) {
            if (j == 0) {
                auto& rootCube{ data.m_config.cubes[i] };

                transforms.push_back(Transform{ .rotation = glm::identity<glm::quat>(),
                    .position = { rootCube.position[0], rootCube.position[1], rootCube.position[2] },
                    .scale = { rootCube.tiling[0], rootCube.tiling[1], rootCube.tiling[2] } });
                scales.push_back(transforms.front().scale);
            } else {
                auto& parent{ *innerCubes[j - 1] };
                auto& child{ *innerCubes[j] };

                auto parentTiling{ glm::ivec3{ parent.tiling[0], parent.tiling[1], parent.tiling[2] } };
                auto childTiling{ glm::ivec3{ child.tiling[0], child.tiling[1], child.tiling[2] } };

                if ((childTiling.x > parentTiling.x) || (childTiling.y > parentTiling.y)
                    || (childTiling.z > parentTiling.z)) {
                    return;
                }

                auto scale{ glm::vec3(childTiling) / glm::vec3(parentTiling) };
                auto halfScale{ scale / 2.0f };

                transforms.push_back(Transform{ .rotation = glm::identity<glm::quat>(),
                    .position = glm::vec3{ -0.5f + halfScale.x, 0.5f - halfScale.y, -0.5f + halfScale.z },
                    .scale = scale });

                CubeTickInfo tickInfo{};

                auto maxIterations{ parentTiling / childTiling };
                tickInfo.limits = maxIterations - 1;
                tickInfo.currentTick = 0;
                tickInfo.currentIter = { 0, 0, 0 };
                tickInfo.tickRate = glm::compMul(childTiling);
                switch (innerCubes[j]->traversalOrder) {
                case TraversalOrder::XYZ:
                    tickInfo.order = { 0, 1, 2 };
                    break;
                case TraversalOrder::XZY:
                    tickInfo.order = { 0, 2, 1 };
                    break;
                case TraversalOrder::YXZ:
                    tickInfo.order = { 1, 0, 2 };
                    break;
                case TraversalOrder::YZX:
                    tickInfo.order = { 2, 0, 1 };
                    break;
                case TraversalOrder::ZXY:
                    tickInfo.order = { 1, 2, 0 };
                    break;
                case TraversalOrder::ZYX:
                    tickInfo.order = { 2, 1, 0 };
                    break;
                }

                tickInfos.push_back(tickInfo);
                scales.push_back(scales.back() * transforms.back().scale);
            }
        }

        auto rootEntity{ m_entityManager->addEntity(m_cubeArchetype) };
        auto childrenEntities{ m_entityManager->addEntities(innerCubes.size() - 1, m_childrenArchetype) };
        TextureSampler<Texture2D> gridTexture{ m_cubeTexture, TextureSlot::Slot0 };

        m_cubeQuery.query(*m_componentManager)
            .filter<RenderLayer>([](const RenderLayer* layer) -> bool { return layer->m_layerMask == 0; })
            .iterate<Transform, std::shared_ptr<Mesh>, Material, RenderLayer>(
                [&, &cubeMesh = m_cubeMesh](std::size_t j, Transform* transform, std::shared_ptr<Mesh>* mesh,
                    Material* material, RenderLayer* layer) {
                    *transform = transforms[j];
                    *mesh = cubeMesh;
                    *material = { ShaderEnvironment{ *data.m_shader, ParameterQualifier::Material }, data.m_shader };
                    material->m_materialVariables.set("diffuseColor", colors[j]);
                    material->m_materialVariables.set("gridTexture", gridTexture);

                    auto absoluteScale{ scales[j] };
                    std::array<glm::vec2, 3> textureScaling{
                        glm::vec2{ absoluteScale.x, absoluteScale.y },
                        glm::vec2{ absoluteScale.x, absoluteScale.z },
                        glm::vec2{ absoluteScale.y, absoluteScale.z },
                    };

                    material->m_materialVariables.setArray(
                        "gridScale", std::span<const glm::vec2>{ textureScaling.data(), 3 });

                    *layer = RenderLayer::layer(i);
                });

        m_childrenQuery.query(*m_componentManager)
            .filter<Parent>([](const Parent* parent) -> bool { return parent->m_parent.id == 0; })
            .iterate<CubeTickInfo, Parent>([&](std::size_t j, CubeTickInfo* tickInfo, Parent* parent) {
                *tickInfo = tickInfos[j];
                tickInfo->startPos = transforms[j + 1].position;

                if (j == 0) {
                    parent->m_parent = rootEntity;
                } else {
                    parent->m_parent = childrenEntities[j - 1];
                }
            });
    }
}

}