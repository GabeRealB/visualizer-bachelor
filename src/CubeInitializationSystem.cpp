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

    constexpr GLuint indices[]{
        0, 1, 2, 0, 2, 3, // front
        3, 2, 6, 3, 6, 7, // top
        1, 5, 6, 1, 6, 2, // right
        4, 0, 3, 4, 3, 7, // left
        4, 5, 1, 4, 1, 0, // bottom
        5, 4, 7, 5, 7, 6 // back
    };

    mesh->setVertices(vertices, sizeof(vertices) / sizeof(glm::vec4));
    mesh->setIndices(indices, sizeof(indices) / sizeof(GLuint), GL_TRIANGLES);
    return mesh;
}

CubeInitializationSystem::CubeInitializationSystem()
    : m_cubeMesh{ generateCubeMesh() }
    , m_cubeArchetype{ EntityArchetype::create<Transform, std::shared_ptr<Mesh>, Material, RenderLayer>() }
    , m_childrenArchetype{ EntityArchetype::with<CubeTickInfo, Parent>(m_cubeArchetype) }
    , m_cubeQuery{ m_cubeArchetype }
    , m_childrenQuery{ m_childrenArchetype }
    , m_entityManager{}
    , m_componentManager{}
{
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

        for (auto child{ static_cast<const InnerCube*>(&data.m_config.cubes[i]) }; child != nullptr;
             child = child->innerCube.get()) {
            innerCubes.push_back(child);
            colors.emplace_back(glm::vec3(child->color[0], child->color[1], child->color[2]) / 255.0f, 0.6f);
        }

        transforms.reserve(innerCubes.size());
        tickInfos.reserve(innerCubes.size() - 1);
        for (std::size_t j = 0; j < innerCubes.size(); ++j) {
            if (j == 0) {
                auto& rootCube{ data.m_config.cubes[i] };

                transforms.push_back(Transform{ .rotation = glm::identity<glm::quat>(),
                    .position = { rootCube.position[0], rootCube.position[1], rootCube.position[2] },
                    .scale = { rootCube.tiling[0], rootCube.tiling[1], rootCube.tiling[2] } });
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
            }
        }

        auto rootEntity{ m_entityManager->addEntity(m_cubeArchetype) };
        auto childrenEntities{ m_entityManager->addEntities(innerCubes.size() - 1, m_childrenArchetype) };
        Material cubeMaterial{ ShaderEnvironment{ *data.m_shader, ParameterQualifier::Material }, data.m_shader };

        m_cubeQuery.query(*m_componentManager)
            .filter<RenderLayer>([](const RenderLayer* layer) -> bool { return layer->m_layerMask == 0; })
            .iterate<Transform, std::shared_ptr<Mesh>, Material, RenderLayer>(
                [&, &cubeMesh = m_cubeMesh](std::size_t j, Transform* transform, std::shared_ptr<Mesh>* mesh,
                    Material* material, RenderLayer* layer) {
                    *transform = transforms[j];
                    *mesh = cubeMesh;
                    *material = cubeMaterial;
                    material->m_materialVariables.set("diffuseColor", colors[j]);

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