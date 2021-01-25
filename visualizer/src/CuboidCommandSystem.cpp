#include <visualizer/CuboidCommandSystem.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <visualizer/Cube.hpp>
#include <visualizer/CuboidCommandList.hpp>
#include <visualizer/Mesh.hpp>
#include <visualizer/Parent.hpp>
#include <visualizer/Shader.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

constexpr double TICK_INTERVAL_0 = 1.0;
constexpr double TICK_INTERVAL_1 = 0.1;
constexpr double TICK_INTERVAL_2 = 0.01;
constexpr double TICK_INTERVAL_3 = 0.001;
constexpr double TICK_INTERVAL_4 = 0.0001;
constexpr double TICK_INTERVAL_STOPPED = std::numeric_limits<double>::max();

void step_iteration(
    CuboidCommandList& command_list, std::shared_ptr<Mesh>& mesh, Material& material, Transform& transform);

CuboidCommandSystem::CuboidCommandSystem()
    : m_current_time{ glfwGetTime() }
    , m_tick_interval{ TICK_INTERVAL_0 }
    , m_time_accumulator{ TICK_INTERVAL_0 }
    , m_cuboid_query{ EntityDBQuery{}
                          .with_component<Cube, CuboidCommandList, std::shared_ptr<Mesh>, Material, Transform>()
                          .without_component<Parent>() }
    , m_entity_database{}
{
}

void CuboidCommandSystem::initialize() { m_entity_database = m_world->getManager<EntityDatabase>(); }

void CuboidCommandSystem::terminate() { m_entity_database = nullptr; }

void CuboidCommandSystem::run(void*)
{
    auto current_time = glfwGetTime();
    auto delta_time = current_time - m_current_time;
    m_current_time = current_time;

    auto window{ glfwGetCurrentContext() };
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        m_time_accumulator = 0.0;
        m_tick_interval = TICK_INTERVAL_0;
    } else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        m_time_accumulator = 0.0;
        m_tick_interval = TICK_INTERVAL_1;
    } else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        m_time_accumulator = 0.0;
        m_tick_interval = TICK_INTERVAL_2;
    } else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        m_time_accumulator = 0.0;
        m_tick_interval = TICK_INTERVAL_3;
    } else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        m_time_accumulator = 0.0;
        m_tick_interval = TICK_INTERVAL_4;
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        m_time_accumulator = 0.0;
        m_tick_interval = TICK_INTERVAL_STOPPED;
    }

    m_time_accumulator += delta_time;
    if (m_time_accumulator >= m_tick_interval) {
        m_time_accumulator -= m_tick_interval;

        m_entity_database->enter_secure_context([&](EntityDatabaseContext& entity_database) {
            m_cuboid_query.query_db_window(entity_database)
                .for_each<CuboidCommandList, std::shared_ptr<Mesh>, Material, Transform>(
                    [](CuboidCommandList* command_list, std::shared_ptr<Mesh>* mesh, Material* material,
                        Transform* transform) { step_iteration(*command_list, *mesh, *material, *transform); });
        });
    }
}

bool step_iteration(
    const NoopCommand& command, std::shared_ptr<Mesh>&, Material&, Transform&, std::size_t command_counter)
{
    return command_counter + 1 >= command.counter;
}

bool step_iteration(
    const DrawCommand& command, std::shared_ptr<Mesh>& mesh, Material& material, Transform& transform, std::size_t)
{
    material.m_materialVariables.set("fill_color", command.fill_color);
    material.m_materialVariables.set("border_color", command.border_color);

    transform.position = command.start_position;
    transform.scale = command.cuboid_size;

    auto model_matrix = getModelMatrix(transform);

    const std::array<VertexAttributeDesc, 4> model_matrix_buffer{
        VertexAttributeDesc{ 2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0, 1 },
        VertexAttributeDesc{ 3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(1 * sizeof(glm::vec4)), 1 },
        VertexAttributeDesc{ 4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)), 1 },
        VertexAttributeDesc{ 5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)), 1 },
    };

    mesh->set_num_instances(1);
    mesh->set_complex_attribute(
        "model_matrix", model_matrix_buffer, sizeof(glm::mat4), GL_STATIC_DRAW, glm::value_ptr(model_matrix));

    return true;
}

bool step_iteration(
    const DrawMultipleCommand& command, std::shared_ptr<Mesh>& mesh, Material& material, Transform&, std::size_t)
{
    if (command.cuboid_sizes.size() == 0) {
        return true;
    }

    material.m_materialVariables.set("fill_color", command.fill_color);
    material.m_materialVariables.set("border_color", command.border_color);

    std::vector<glm::mat4> model_matrices{};
    model_matrices.reserve(command.start_positions.size());

    for (std::size_t i = 0; i < command.start_positions.size(); ++i) {
        model_matrices.push_back(getModelMatrix({
            glm::identity<glm::quat>(),
            command.start_positions[i],
            command.cuboid_sizes[i],
        }));
    }

    const std::array<VertexAttributeDesc, 4> model_matrix_buffer{
        VertexAttributeDesc{ 2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0, 1 },
        VertexAttributeDesc{ 3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(1 * sizeof(glm::vec4)), 1 },
        VertexAttributeDesc{ 4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)), 1 },
        VertexAttributeDesc{ 5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)), 1 },
    };

    mesh->set_num_instances(command.cuboid_sizes.size());
    mesh->set_complex_attribute("model_matrix", model_matrix_buffer, command.cuboid_sizes.size() * sizeof(glm::mat4),
        GL_STATIC_DRAW, glm::value_ptr(model_matrices.front()));

    return true;
}

bool step_iteration(const DeleteCommand& command, std::shared_ptr<Mesh>&, Material& material, Transform&, std::size_t)
{
    material.m_materialVariables.set("fill_color", command.fill_color);
    material.m_materialVariables.set("border_color", command.border_color);

    return true;
}

bool step_iteration(const DeleteMultipleCommand& command, std::shared_ptr<Mesh>&, Material& material, Transform&,
    std::size_t command_counter)
{
    material.m_materialVariables.set("fill_color", command.fill_color);
    material.m_materialVariables.set("border_color", command.border_color);

    return command_counter + 1 >= command.counter;
}

void step_iteration(
    CuboidCommandList& command_list, std::shared_ptr<Mesh>& mesh, Material& material, Transform& transform)
{
    if (command_list.commands.size() == 0) {
        return;
    }

    bool step_command = false;
    const auto& command = command_list.commands[command_list.current_index];

    step_command = std::visit(
        [&](auto&& command) -> auto {
            return step_iteration(command, mesh, material, transform, command_list.command_counter);
        },
        command.command);

    ++command_list.command_counter;
    if (step_command) {
        ++command_list.current_index;
        command_list.command_counter = 0;

        if (command_list.current_index == command_list.commands.size()) {
            command_list.current_index = 0;
        }
    }
}

}