#include <visualizer/CubeMovementSystem.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <visualizer/Iteration.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

CubeMovementSystem::CubeMovementSystem()
    : m_accumulator{ 0 }
    , m_currentTime{ 0 }
    , m_tick_interval{ 1.0 }
    , m_cubes_query_mesh{ EntityDBQuery{}.with_component<MeshIteration, std::shared_ptr<Mesh>>() }
    , m_cubes_query_activation{ EntityDBQuery{}.with_component<EntityActivation>() }
    , m_cubes_query_homogeneous{ EntityDBQuery{}.with_component<HomogeneousIteration, Transform>() }
    , m_cubes_query_heterogeneous{ EntityDBQuery{}.with_component<HeterogeneousIteration, Transform>() }
    , m_entity_database{}
{
    m_currentTime = glfwGetTime();
}

void CubeMovementSystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void CubeMovementSystem::terminate() { m_entity_database = nullptr; }

void reverse_transform(const HomogeneousIteration& iteration, Transform& transform)
{
    auto position = iteration.positions[iteration.index];

    auto posX{ transform.scale.x * position.x };
    auto posY{ -transform.scale.y * position.y };
    auto posZ{ transform.scale.z * position.z };

    transform.position -= glm::vec3{ posX, posY, posZ };
}

void reverse_transform(const HeterogeneousIteration& iteration, Transform& transform)
{
    auto scale = iteration.scales[iteration.index];
    auto position = iteration.positions[iteration.index];

    auto half_scale{ scale / 2.0f };
    half_scale.y *= -1.0f;

    auto posX{ scale.x * position.x };
    auto posY{ -scale.y * position.y };
    auto posZ{ scale.z * position.z };

    auto offset{ half_scale + glm::vec3{ posX, posY, posZ } };
    transform.position -= offset;
}

void step_iteration(HomogeneousIteration& iteration)
{
    if (++iteration.tick % iteration.ticksPerIteration != 0) {
        return;
    } else {
        iteration.tick = 0;
    }

    if (++iteration.index >= iteration.positions.size()) {
        iteration.index = 0;
    }
}

bool step_iteration(MeshIteration& iteration)
{
    auto initialized{ iteration.initialized };
    iteration.initialized = true;

    if (++iteration.tick % iteration.ticksPerIteration[iteration.index] != 0) {
        return !initialized;
    } else {
        iteration.tick = 0;
    }

    if (++iteration.index >= iteration.positions.size()) {
        iteration.index = 0;

        // iteration.grid.resize(iteration.dimensions[0] * iteration.dimensions[1] * iteration.dimensions[2], false);
        std::fill(iteration.grid.begin(), iteration.grid.end(), false);
    }

    std::array<std::size_t, 3> pos{
        static_cast<std::size_t>(iteration.positions[iteration.index][0]),
        static_cast<std::size_t>(iteration.positions[iteration.index][1]),
        static_cast<std::size_t>(iteration.positions[iteration.index][2]),
    };
    iteration.grid[pos[0] + iteration.dimensions[0] * (pos[1] + iteration.dimensions[1] * pos[2])] = true;
    return true;
}

void step_iteration(EntityActivation& iteration, EntityDatabaseContext& entity_database)
{
    if (++iteration.tick % iteration.ticksPerIteration[iteration.index] != 0) {
        return;
    } else {
        iteration.tick = 0;
    }

    if (++iteration.index >= iteration.entities.size()) {
        iteration.index = 0;

        for (auto entity : iteration.entities) {
            entity_database.write_component(entity, RenderLayer{ 0 });
        }
    }

    entity_database.write_component(iteration.entities[iteration.index], iteration.layer);
}

void step_iteration(HeterogeneousIteration& iteration)
{
    if (++iteration.tick % iteration.ticksPerIteration[iteration.index] != 0) {
        return;
    } else {
        iteration.tick = 0;
    }

    if (++iteration.index >= iteration.positions.size()) {
        iteration.index = 0;
    }
}

void compute_transform(const HomogeneousIteration& iteration, Transform& transform)
{
    auto position = iteration.positions[iteration.index];

    auto posX{ transform.scale.x * position.x };
    auto posY{ -transform.scale.y * position.y };
    auto posZ{ transform.scale.z * position.z };

    transform.position += glm::vec3{ posX, posY, posZ };
}

void compute_transform(const HeterogeneousIteration& iteration, Transform& transform)
{
    auto scale = iteration.scales[iteration.index];
    auto position = iteration.positions[iteration.index];

    auto half_scale{ scale / 2.0f };
    half_scale.y *= -1.0f;

    auto posX{ scale.x * position.x };
    auto posY{ -scale.y * position.y };
    auto posZ{ scale.z * position.z };

    auto offset{ half_scale + glm::vec3{ posX, posY, posZ } };

    transform.scale = scale;
    transform.position += offset;
}

void compute_mesh(const MeshIteration& iteration, Mesh& mesh)
{
    /// Adapted from:
    /// https://github.com/mikolalysenko/mikolalysenko.github.com/blob/gh-pages/MinecraftMeshes/js/greedy.js

    std::vector<GLuint> indices{};
    std::vector<glm::vec4> vertices{};
    std::vector<glm::vec4> tex_coords{};

    std::array<std::size_t, 3> dimensions{
        iteration.dimensions[0],
        iteration.dimensions[1],
        iteration.dimensions[2],
    };

    std::vector<bool> mask{};

    auto getGridValue{ [&](std::size_t i, std::size_t j, std::size_t k) {
        return iteration.grid[i + iteration.dimensions[0] * (j + iteration.dimensions[1] * k)];
    } };

    // Sweep over 3-axes
    for (std::size_t dimension{ 0 }; dimension < 3; ++dimension) {
        glm::i64vec3 x{ 0, 0, 0 };
        glm::i64vec3 q{ 0, 0, 0 };
        std::size_t u{ (dimension + 1) % 3ull };
        std::size_t v{ (dimension + 2) % 3ull };

        auto elementsU{ dimensions[u] };
        auto elementsV{ dimensions[v] };
        auto elements{ elementsU * elementsV };
        mask.reserve(elements);
        mask.resize(elements, false);
        std::fill(mask.begin(), mask.end(), false);

        q[dimension] = 1;

        for (x[dimension] = -1; x[dimension] < static_cast<std::int64_t>(iteration.dimensions[dimension]);) {
            // Compute mask
            std::size_t n{ 0 };
            for (x[v] = 0; x[v] < static_cast<std::int64_t>(iteration.dimensions[v]); ++x[v]) {
                for (x[u] = 0; x[u] < static_cast<std::int64_t>(iteration.dimensions[u]); ++x[u]) {
                    mask[n++] = (0 <= x[dimension]
                                    && getGridValue(static_cast<std::size_t>(x[0]), static_cast<std::size_t>(x[1]),
                                        static_cast<std::size_t>(x[2])))
                        != (x[dimension] < static_cast<std::int64_t>(iteration.dimensions[dimension]) - 1
                            && getGridValue(static_cast<std::size_t>(x[0] + q[0]),
                                static_cast<std::size_t>(x[1] + q[1]), static_cast<std::size_t>(x[2] + q[2])));
                }
            }

            // Increment x[dimension]
            ++x[dimension];

            // Generate mesh for mask using lexicographic ordering
            n = 0;
            for (std::size_t j{ 0 }; j < iteration.dimensions[v]; ++j) {
                for (std::size_t i{ 0 }; i < iteration.dimensions[u];) {
                    if (mask[n]) {
                        // Compute width
                        std::size_t w;
                        for (w = 1; n + w < mask.size() && mask[n + w] && i + w < iteration.dimensions[u]; ++w) {
                        }

                        // Compute height (this is slightly awkward)
                        bool done{ false };
                        std::size_t h;
                        for (h = 1; j + h < iteration.dimensions[v]; ++h) {
                            for (std::size_t k{ 0 }; k < w; ++k) {
                                if (!mask[n + k + h * iteration.dimensions[u]]) {
                                    done = true;
                                    break;
                                }
                            }
                            if (done) {
                                break;
                            }
                        }

                        // Add quad
                        x[u] = static_cast<std::int64_t>(i);
                        x[v] = static_cast<std::int64_t>(j);
                        glm::vec<3, std::size_t, glm::defaultp> du{ 0, 0, 0 };
                        glm::vec<3, std::size_t, glm::defaultp> dv{ 0, 0, 0 };
                        du[u] = w;
                        dv[v] = h;

                        std::size_t numVertices{ vertices.size() };
                        vertices.push_back({ x[0], -1.0f * x[1], x[2], 1.0f });
                        vertices.push_back({ x[0] + du[0], -1.0f * (x[1] + du[1]), x[2] + du[2], 1.0f });
                        vertices.push_back(
                            { x[0] + du[0] + dv[0], -1.0f * (x[1] + du[1] + dv[1]), x[2] + du[2] + dv[2], 1.0f });
                        vertices.push_back({ x[0] + dv[0], -1.0f * (x[1] + dv[1]), x[2] + dv[2], 1.0f });

                        tex_coords.push_back({ 0.0f, 0.0f, 0.0f, 0.0f });
                        tex_coords.push_back({ 0.0f, 0.0f, 0.0f, 0.0f });
                        tex_coords.push_back({ 0.0f, 0.0f, 0.0f, 0.0f });
                        tex_coords.push_back({ 0.0f, 0.0f, 0.0f, 0.0f });

                        indices.push_back(numVertices + 3);
                        indices.push_back(numVertices + 2);
                        indices.push_back(numVertices + 1);
                        indices.push_back(numVertices + 3);
                        indices.push_back(numVertices + 1);
                        indices.push_back(numVertices + 0);

                        // Zero-out mask
                        for (std::size_t l{ 0 }; l < h; ++l) {
                            for (std::size_t k{ 0 }; k < w; ++k) {
                                mask[n + k + l * iteration.dimensions[u]] = false;
                            }
                        }

                        // Increment counters and continue
                        i += w;
                        n += w;
                    } else {
                        ++i;
                        ++n;
                    }
                }
            }
        }
    }

    mesh.setVertices(vertices.data(), vertices.size());
    mesh.setIndices(indices.data(), indices.size(), GL_TRIANGLES);
    mesh.setTextureCoordinates0(tex_coords.data(), tex_coords.size());
}

void CubeMovementSystem::run(void*)
{
    auto currentTime{ glfwGetTime() };
    auto deltaTime{ currentTime - m_currentTime };
    m_currentTime = currentTime;

    auto window{ glfwGetCurrentContext() };
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        m_tick_interval = 1.0f;
    } else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        m_tick_interval = 0.1f;
    } else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        m_tick_interval = 0.01f;
    } else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        m_tick_interval = 0.001f;
    } else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        m_tick_interval = 0.0001f;
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        m_tick_interval = std::numeric_limits<double>::max();
    }

    m_accumulator += deltaTime;
    if (m_accumulator >= m_tick_interval) {
        m_accumulator = 0;

        m_entity_database->enter_secure_context([&](EntityDatabaseContext& entity_database) {
            m_cubes_query_mesh.query_db_window(entity_database)
                .for_each<MeshIteration, std::shared_ptr<Mesh>>(
                    [](MeshIteration* meshIteration, std::shared_ptr<Mesh>* mesh) {
                        if (step_iteration(*meshIteration)) {
                            compute_mesh(*meshIteration, *mesh->get());
                        }
                    });

            m_cubes_query_activation.query_db_window(entity_database)
                .for_each<EntityActivation>(
                    [&](EntityActivation* iteration) { step_iteration(*iteration, entity_database); });

            m_cubes_query_homogeneous.query_db_window(entity_database)
                .for_each<HomogeneousIteration, Transform>([](HomogeneousIteration* iteration, Transform* transform) {
                    reverse_transform(*iteration, *transform);
                    step_iteration(*iteration);
                    compute_transform(*iteration, *transform);
                });

            m_cubes_query_heterogeneous.query_db_window(entity_database)
                .for_each<HeterogeneousIteration, Transform>(
                    [](HeterogeneousIteration* iteration, Transform* transform) {
                        reverse_transform(*iteration, *transform);
                        step_iteration(*iteration);
                        compute_transform(*iteration, *transform);
                    });
        });
    }
}

}