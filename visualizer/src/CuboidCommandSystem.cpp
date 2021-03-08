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

constexpr unsigned int ACTIVE_FLAG = 1u << 31;
constexpr unsigned int OUT_OF_BOUNDS_FLAG = 1u << 30;
constexpr unsigned int HEATMAP_FLAG = 1u << 29;

constexpr unsigned int HEATMAP_COUNTER_BITS = 16;
constexpr unsigned int HEATMAP_COUNTER_MASK = (1u << HEATMAP_COUNTER_BITS) - 1;

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

bool step_iteration(CuboidCommandList&, const NoopCommand& command, const CuboidCommand&, std::shared_ptr<Mesh>&,
    Material&, Transform&, std::size_t command_counter)
{
    return command_counter + 1 >= command.counter;
}

bool step_iteration(CuboidCommandList& command_list, const DrawCommand& command, const CuboidCommand& previous,
    std::shared_ptr<Mesh>& mesh, Material&, Transform&, std::size_t)
{
    auto& enabled_buffer
        = std::get<std::shared_ptr<ComplexVertexAttributeBuffer>>(mesh->get_vertex_buffer("enabled_cuboids"));
    auto buffer_ptr = static_cast<GLuint*>(enabled_buffer->map(GL_READ_WRITE));
    auto& previous_command = std::get<DrawCommand>(previous.command);
    auto accesses = ++command_list.cuboid_accesses[command.cuboid_idx];

    buffer_ptr[previous_command.cuboid_idx]
        &= previous_command.out_of_bounds ? ~(ACTIVE_FLAG | OUT_OF_BOUNDS_FLAG) : ~ACTIVE_FLAG;
    buffer_ptr[command.cuboid_idx] |= command.out_of_bounds ? (ACTIVE_FLAG | OUT_OF_BOUNDS_FLAG) : ACTIVE_FLAG;

    if (command_list.heat_map) {
        buffer_ptr[command.cuboid_idx] &= ~HEATMAP_COUNTER_MASK;
        buffer_ptr[command.cuboid_idx] |= (HEATMAP_FLAG | (accesses / command_list.access_stepping));
    }

    enabled_buffer->unmap();
    return true;
}

bool step_iteration(CuboidCommandList& command_list, const DrawMultipleCommand& command, const CuboidCommand& previous,
    std::shared_ptr<Mesh>& mesh, Material&, Transform&, std::size_t)
{
    auto& enabled_buffer
        = std::get<std::shared_ptr<ComplexVertexAttributeBuffer>>(mesh->get_vertex_buffer("enabled_cuboids"));
    auto buffer_ptr = static_cast<GLuint*>(enabled_buffer->map(GL_READ_WRITE));
    auto& previous_command = std::get<DrawMultipleCommand>(previous.command);

    for (auto idx : previous_command.cuboid_indices) {
        buffer_ptr[idx] &= ~ACTIVE_FLAG;
    }
    for (auto idx : previous_command.out_of_bounds) {
        buffer_ptr[idx] &= ~(ACTIVE_FLAG | OUT_OF_BOUNDS_FLAG);
    }

    for (auto idx : command.cuboid_indices) {
        buffer_ptr[idx] |= ACTIVE_FLAG;
        auto accesses = ++command_list.cuboid_accesses[idx];

        if (command_list.heat_map) {
            buffer_ptr[idx] &= ~HEATMAP_COUNTER_MASK;
            buffer_ptr[idx] |= (HEATMAP_FLAG | (accesses / command_list.access_stepping));
        }
    }
    for (auto idx : command.out_of_bounds) {
        buffer_ptr[idx] |= (ACTIVE_FLAG | OUT_OF_BOUNDS_FLAG);
        auto accesses = ++command_list.cuboid_accesses[idx];

        if (command_list.heat_map) {
            buffer_ptr[idx] &= ~HEATMAP_COUNTER_MASK;
            buffer_ptr[idx] |= (HEATMAP_FLAG | (accesses / command_list.access_stepping));
        }
    }

    enabled_buffer->unmap();

    return true;
}

bool step_iteration(CuboidCommandList&, const DeleteCommand&, const CuboidCommand&, std::shared_ptr<Mesh>&, Material&,
    Transform&, std::size_t)
{
    return true;
}

bool step_iteration(CuboidCommandList&, const DeleteMultipleCommand&, const CuboidCommand&, std::shared_ptr<Mesh>&,
    Material&, Transform&, std::size_t)
{
    return true;
}

void step_iteration(
    CuboidCommandList& command_list, std::shared_ptr<Mesh>& mesh, Material& material, Transform& transform)
{
    if (command_list.commands.size() == 0) {
        return;
    }

    bool step_command = false;
    const auto& command = command_list.commands[command_list.current_index];
    const auto& previous_command = [&]() -> auto&
    {
        if (command_list.current_index == 0) {
            return command_list.commands[command_list.commands.size() - 3];
        } else if (command_list.current_index % 3 == 0) {
            return command_list.commands[command_list.current_index - 3];
        } else {
            auto offset = command_list.current_index % 3;
            return command_list.commands[command_list.current_index - offset];
        }
    }
    ();

    step_command = std::visit(
        [&](auto&& command) -> auto {
            return step_iteration(
                command_list, command, previous_command, mesh, material, transform, command_list.command_counter);
        },
        command.command);

    ++command_list.command_counter;
    if (step_command) {
        ++command_list.current_index;
        command_list.command_counter = 0;

        if (command_list.current_index == command_list.commands.size()) {
            command_list.current_index = 0;
            std::fill(command_list.cuboid_accesses.begin(), command_list.cuboid_accesses.end(), 0);

            auto& enabled_buffer
                = std::get<std::shared_ptr<ComplexVertexAttributeBuffer>>(mesh->get_vertex_buffer("enabled_cuboids"));
            auto buffer_ptr = static_cast<GLuint*>(enabled_buffer->map(GL_READ_WRITE));
            std::memset(buffer_ptr, 0, enabled_buffer->size());
            enabled_buffer->unmap();
        }
    }
}

}