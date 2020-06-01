#include <visualizer/CubeInitializationSystem.hpp>

#include <visualizer/Camera.hpp>
#include <visualizer/CubeTickInfo.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

CubeInitializationSystem::CubeInitializationSystem(
    Resolution resolution, OuterCube outerCube, std::shared_ptr<Mesh> cubeMesh)
    : m_resolution{ resolution }
    , m_outerCube{ std::move(outerCube) }
    , m_cubeMesh{ std::move(cubeMesh) }
    , m_cubeArchetype{ EntityArchetype::create<Transform, CubeTickInfo, std::shared_ptr<Mesh>, Material>() }
    , m_cameraArchetype{ EntityArchetype::create<Camera, Transform>() }
    , m_cubeQuery{ m_cubeArchetype }
    , m_cameraQuery{ m_cameraArchetype }
    , m_entityManager{}
    , m_componentManager{}
{
}

void CubeInitializationSystem::run(CubeInitializationSystem::Data& data)
{
    std::vector<const InnerCube*> innerCubes{};
    std::vector<glm::vec3> positions{};
    std::vector<CubeTickInfo> tickInfos{};
    std::vector<glm::vec4> colors{};
    innerCubes.push_back(static_cast<const InnerCube*>(&m_outerCube));
    positions.emplace_back();
    tickInfos.emplace_back();
    colors.emplace_back(static_cast<float>(m_outerCube.color[0]) / 255.0f,
        static_cast<float>(m_outerCube.color[1]) / 255.0f, static_cast<float>(m_outerCube.color[2]) / 255.0f, 0.6f);

    for (auto child{ m_outerCube.innerCube.get() }; child != nullptr; child = child->innerCube.get()) {
        innerCubes.push_back(child);
        positions.emplace_back();
        tickInfos.emplace_back();
        colors.emplace_back(static_cast<float>(child->color[0]) / 255.0f, static_cast<float>(child->color[1]) / 255.0f,
            static_cast<float>(child->color[2]) / 255.0f, 0.6f);
    }

    auto entities{ m_entityManager->addEntities(innerCubes.size(), m_cubeArchetype) };
    if (entities.size() == 0) {
        return;
    }

    for (std::size_t i = 0; i < innerCubes.size(); ++i) {
        if (i > 0) {
            auto current{ innerCubes[i] };
            auto previous{ innerCubes[i - 1] };
            if ((current->tiling[0] > previous->tiling[0]) || (current->tiling[1] > previous->tiling[1])
                || (current->tiling[2] > previous->tiling[2])) {
                return;
            }

            positions[i] = positions[i - 1]
                + glm::vec3{ -(float)m_outerCube.tiling[0] / 2, (float)m_outerCube.tiling[1] / 2,
                      -(float)m_outerCube.tiling[2] / 2 }
                + glm::vec3{ (float)current->tiling[0] / 2, -(float)current->tiling[1] / 2,
                      (float)current->tiling[2] / 2 };
        } else {
            positions[i] = { m_outerCube.position[0], m_outerCube.position[1], m_outerCube.position[2] };
        }
    }

    for (std::size_t i = innerCubes.size() - 1; i + 1 != 0; --i) {
        auto current{ innerCubes[i] };
        auto& currentTickInfo{ tickInfos[i] };
        if (i != innerCubes.size() - 1) {
            currentTickInfo.tickRate = tickInfos[i + 1].tickRate
                + (tickInfos[i + 1].limits[0] * tickInfos[i + 1].limits[1] * tickInfos[i + 1].limits[2]);
        } else {
            currentTickInfo.tickRate = 1;
        }

        if (i > 0) {
            auto previous{ innerCubes[i - 1] };
            currentTickInfo.limits = { (previous->tiling[0] / current->tiling[0]) - 1,
                (previous->tiling[1] / current->tiling[1]) - 1, (previous->tiling[2] / current->tiling[2]) - 1 };
        } else {
            currentTickInfo.limits = { 0, 0, 0 };
        }

        currentTickInfo.currentTick = 0;
        currentTickInfo.currentIter = { 0, 0, 0 };
        currentTickInfo.canTick = i != 0;

        switch (current->traversalOrder) {
        case TraversalOrder::XYZ:
            currentTickInfo.order = { 0, 1, 2 };
            break;
        case TraversalOrder::XZY:
            currentTickInfo.order = { 0, 2, 1 };
            break;
        case TraversalOrder::YXZ:
            currentTickInfo.order = { 1, 0, 2 };
            break;
        case TraversalOrder::YZX:
            currentTickInfo.order = { 2, 0, 1 };
            break;
        case TraversalOrder::ZXY:
            currentTickInfo.order = { 1, 2, 0 };
            break;
        case TraversalOrder::ZYX:
            currentTickInfo.order = { 2, 1, 0 };
            break;
        }

        if (i == 0) {
            break;
        }
    }

    Material cubeMaterial{ ShaderEnvironment{ *data.shader, ParameterQualifier::Material }, std::move(data.shader) };
    Transform startTransform{ .rotation = { 1.0, 0.0, 0.0, 0.0 }, .position = { 0, 0, 0 }, .scale = { 1, 1, 1 } };

    auto cubeQueryResult{ m_cubeQuery.query(*m_componentManager) };
    cubeQueryResult.forEach<Transform, CubeTickInfo, std::shared_ptr<Mesh>, Material>([&, &cubeMesh = m_cubeMesh](
                                                                                          Transform* transform,
                                                                                          CubeTickInfo* tickInfo,
                                                                                          std::shared_ptr<Mesh>* mesh,
                                                                                          Material* material) {
        *transform = startTransform;
        transform->position = positions.back();
        transform->scale = { innerCubes.back()->tiling[0], innerCubes.back()->tiling[1], innerCubes.back()->tiling[2] };
        *tickInfo = tickInfos.back();
        tickInfo->startPos = transform->position;
        *mesh = cubeMesh;
        *material = cubeMaterial;
        material->m_materialVariables.set("diffuseColor", colors.back());
        innerCubes.pop_back();
        positions.pop_back();
        tickInfos.pop_back();
        colors.pop_back();
    });

    auto projectionMatrix{ glm::perspective(
        64.0f, static_cast<float>(m_resolution[0]) / static_cast<float>(m_resolution[1]), 0.1f, 1000.0f) };

    m_entityManager->addEntity(m_cameraArchetype);
    auto cameraQueryResult{ m_cameraQuery.query(*m_componentManager) };
    cameraQueryResult.forEach<Camera, Transform>([&projectionMatrix](Camera* camera, Transform* transform) {
        *camera = { projectionMatrix };
        *transform = { .rotation = { 1.0, 0.0, 0.0, 0.0 }, .position = { 0, 0, 20 }, .scale = { 1, 1, 1 } };
    });
}

void CubeInitializationSystem::run(void* data)
{
    if (data) {
        run(*static_cast<Data*>(data));
    }
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

}