#include "processing.hpp"
#include "Config.hpp"

#include <algorithm>
#include <thread>

namespace Config {

struct CuboidContainerTupleVec {
    std::vector<std::pair<CuboidContainer, std::size_t>> containers;
};

using ContainerList = std::vector<std::variant<std::monostate, CuboidContainerTupleVec>>;

void generate_cuboid_command_list(std::vector<CuboidCommandList>& command_list, const ContainerList& container_list,
    VariableMap& variable_map, VariableType variable_type, std::size_t end_idx, std::size_t idx)
{
    auto noop_command_lambda = [&](auto&& value) {
        if constexpr (std::is_same_v<decltype(value), const CuboidContainerTupleVec&>) {
            auto& cuboids = value.containers;

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                auto& cuboid_commands = command_list[cuboids[i].second].commands;

                if (cuboid_commands.empty() || cuboid_commands.back().type != CuboidCommandType::NOOP) {
                    cuboid_commands.push_back({ CuboidCommandType::NOOP, NoopCommand{ 1 } });
                } else {
                    std::get<NoopCommand>(cuboid_commands.back().command).counter++;
                }
            }
        }
    };

    auto draw_command_lambda = [&](auto&& value) {
        if constexpr (std::is_same_v<decltype(value), const CuboidContainerTupleVec&>) {
            auto& cuboids = value.containers;

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                auto position_data = cuboids[i].first.pos_size_callable(variable_map);
                std::array<int, 3> start_position = {
                    std::get<0>(position_data[0]),
                    std::get<0>(position_data[1]),
                    std::get<0>(position_data[2]),
                };
                std::array<int, 3> cuboid_size = {
                    1 + std::get<1>(position_data[0]) - start_position[0],
                    1 + std::get<1>(position_data[1]) - start_position[1],
                    1 + std::get<1>(position_data[2]) - start_position[2],
                };

                auto& cuboid_list = command_list[cuboids[i].second];

                std::size_t cuboid_idx;
                if (cuboid_list.position_index_map.contains(std::make_tuple(start_position, cuboid_size))) {
                    cuboid_idx = cuboid_list.position_index_map.at(std::make_tuple(start_position, cuboid_size));
                } else {
                    CuboidInfo cuboid_info = {
                        cuboid_size,
                        start_position,
                    };

                    cuboid_idx = cuboid_list.positions.size();
                    cuboid_list.position_index_map.insert({ std::make_tuple(start_position, cuboid_size), cuboid_idx });
                    cuboid_list.positions.push_back(cuboid_info);
                }

                BoundingBox box = {
                    start_position,
                    {
                        start_position[0] + cuboid_size[0],
                        start_position[1] + cuboid_size[1],
                        start_position[2] + cuboid_size[2],
                    },
                };

                cuboid_list.commands.push_back({ CuboidCommandType::DRAW,
                    DrawCommand{
                        false,
                        cuboid_idx,
                        box,
                    } });
            }
        }
    };

    auto draw_multiple_command_lambda = [&](auto&& value) {
        if constexpr (std::is_same_v<decltype(value), const CuboidContainerTupleVec&>) {
            auto& cuboids = value.containers;

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                auto position_data = cuboids[i].first.pos_size_callable(variable_map);
                std::array<int, 3> start_position = {
                    std::get<0>(position_data[0]),
                    std::get<0>(position_data[1]),
                    std::get<0>(position_data[2]),
                };
                std::array<int, 3> cuboid_size = {
                    1 + std::get<1>(position_data[0]) - start_position[0],
                    1 + std::get<1>(position_data[1]) - start_position[1],
                    1 + std::get<1>(position_data[2]) - start_position[2],
                };

                auto& cuboid_list = command_list[cuboids[i].second];

                std::size_t cuboid_idx;
                if (cuboid_list.position_index_map.contains(std::make_tuple(start_position, cuboid_size))) {
                    cuboid_idx = cuboid_list.position_index_map.at(std::make_tuple(start_position, cuboid_size));
                } else {
                    CuboidInfo cuboid_info = {
                        cuboid_size,
                        start_position,
                    };

                    cuboid_idx = cuboid_list.positions.size();
                    cuboid_list.position_index_map.insert({ std::make_tuple(start_position, cuboid_size), cuboid_idx });
                    cuboid_list.positions.push_back(cuboid_info);
                }

                auto& cuboid_commands = cuboid_list.commands;
                BoundingBox box = {
                    start_position,
                    {
                        start_position[0] + cuboid_size[0],
                        start_position[1] + cuboid_size[1],
                        start_position[2] + cuboid_size[2],
                    },
                };

                if (cuboid_commands.empty() || cuboid_commands.back().type != CuboidCommandType::DRAW_MULTIPLE) {
                    cuboid_commands.push_back({ CuboidCommandType::DRAW_MULTIPLE,
                        DrawMultipleCommand{
                            {},
                            { box },
                            { cuboid_idx },
                            { { cuboid_idx, 1 } },
                        } });
                } else {
                    auto& command = std::get<DrawMultipleCommand>(cuboid_commands.back().command);
                    [[maybe_unused]] auto [it, new_idx] = command.cuboid_indices.insert(cuboid_idx);

                    if (new_idx) {
                        command.cuboid_accesses.insert({ cuboid_idx, 1 });
                    } else {
                        command.cuboid_accesses.at(cuboid_idx)++;
                    }

                    if (new_idx) {
                        bool intersects = false;
                        for (auto& bounding_box : command.bounding_boxes) {
                            if (aabb_intersects(bounding_box, box) && !aabb_contains(bounding_box, box)) {
                                bounding_box = aabb_extend(bounding_box, box);
                                intersects = true;
                                break;
                            }
                        }

                        if (intersects) {
                            for (std::size_t j = 0; j < command.bounding_boxes.size(); ++j) {
                                auto& box_j = command.bounding_boxes[j];
                                for (std::size_t k = j + 1; k < command.bounding_boxes.size(); ++k) {
                                    auto& box_k = command.bounding_boxes[k];
                                    if (aabb_intersects(box_j, box_k)) {
                                        box_j = aabb_extend(box_j, box_k);
                                        command.bounding_boxes.erase(command.bounding_boxes.begin() + k);
                                        j = 0;
                                        break;
                                    }
                                }
                            }
                        } else {
                            command.bounding_boxes.push_back(box);
                        }
                    }
                }
            }
        }
    };

    auto delete_command_lambda = [&](auto&& value) {
        if constexpr (std::is_same_v<decltype(value), const CuboidContainerTupleVec&>) {
            auto& cuboids = value.containers;

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                command_list[cuboids[i].second].commands.push_back({ CuboidCommandType::DELETE, DeleteCommand{} });
            }
        }
    };

    auto delete_multiple_command_lambda = [&](auto&& value) {
        if constexpr (std::is_same_v<decltype(value), const CuboidContainerTupleVec&>) {
            auto& cuboids = value.containers;

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                auto& cuboid_commands = command_list[cuboids[i].second].commands;

                if (cuboid_commands.empty() || cuboid_commands.back().type != CuboidCommandType::DELETE_MULTIPLE) {
                    cuboid_commands.push_back({ CuboidCommandType::DELETE_MULTIPLE,
                        DeleteMultipleCommand{
                            0,
                        } });
                } else {
                    std::get<DeleteMultipleCommand>(cuboid_commands.back().command).counter++;
                }
            }
        }
    };

    const std::string dummy_name = "dummy";
    auto& variable_name = [&]() -> auto&
    {
        if (idx == 0) {
            return dummy_name;
        } else if (variable_type == VariableType::SEQUENTIAL) {
            return variable_map.get_variable_name(variable_type, idx - 1);
        } else {
            auto i = idx - variable_map.get_num_variables(VariableType::SEQUENTIAL);
            return variable_map.get_variable_name(variable_type, i - 1);
        }
    }
    ();

    std::tuple<int, int> iteration_region;
    if (idx == 0) {
        iteration_region = { 0, 0 };
    } else {
        iteration_region = variable_map.get_variable_region(variable_name);
    }

    int dummy_counter;
    int& counter = [&]() -> auto&
    {
        if (idx == 0) {
            return dummy_counter;
        } else {
            return variable_map.get_variable_ref(variable_name);
        }
    }
    ();

    auto num_seq_vars = variable_map.get_num_variables(VariableType::SEQUENTIAL);

    for (counter = std::get<0>(iteration_region); counter <= std::get<1>(iteration_region); counter++) {
        auto& containers = container_list[idx];

        if (idx <= num_seq_vars) {
            std::visit(draw_command_lambda, containers);
        } else {
            std::visit(draw_multiple_command_lambda, containers);
        }

        if (counter != std::get<0>(iteration_region) && variable_type == VariableType::SEQUENTIAL) {
            for (std::size_t i = 0; i < idx; i++) {
                std::visit(noop_command_lambda, container_list[i]);
            }
        }

        if (idx < end_idx) {
            generate_cuboid_command_list(command_list, container_list, variable_map,
                idx + 1 <= num_seq_vars ? VariableType::SEQUENTIAL : VariableType::PARALLEL, end_idx, idx + 1);
        }

        if (idx <= num_seq_vars) {
            if (idx == num_seq_vars) {
                for (std::size_t i = 0; i <= end_idx; i++) {
                    std::visit(noop_command_lambda, container_list[i]);
                    if (i > idx) {
                        std::visit(delete_multiple_command_lambda, container_list[i]);
                    }
                }
            }

            std::visit(delete_command_lambda, containers);
        }

        if (counter != std::get<1>(iteration_region) && variable_type == VariableType::SEQUENTIAL) {
            for (std::size_t i = 0; i < idx; i++) {
                std::visit(noop_command_lambda, container_list[i]);
            }
        }
    }
}

void generate_bounds_information(const CuboidCommandList& parent_layer, CuboidCommandList& layer)
{
    std::size_t parent_idx = 0;
    std::size_t parent_counter = 0;

    auto step_parent = [&](std::size_t ticks) {
        if (std::holds_alternative<NoopCommand>(parent_layer.commands[parent_idx].command)) {
            parent_counter += ticks;
            auto& noop_command = std::get<NoopCommand>(parent_layer.commands[parent_idx].command);
            if (parent_counter >= noop_command.counter) {
                parent_idx++;
                parent_counter = 0;
            }
        } else {
            parent_idx++;
            parent_counter = 0;
        }
    };

    for (auto& command : layer.commands) {
        auto& parent_command = [&]() -> auto&
        {
            auto reverse_idx = parent_layer.commands.size() - parent_idx - 1;
            for (auto parent_command = parent_layer.commands.rbegin() + reverse_idx;
                 parent_command != parent_layer.commands.rend(); parent_command++) {
                if (std::holds_alternative<DrawCommand>(parent_command->command)
                    || std::holds_alternative<DrawMultipleCommand>(parent_command->command)) {
                    return parent_command->command;
                }
            }
            std::abort();
        }
        ();

        if (std::holds_alternative<DrawCommand>(command.command)) {
            auto& draw_command = std::get<DrawCommand>(command.command);

            if (std::holds_alternative<DrawCommand>(parent_command)) {
                auto& parent_draw_command = std::get<DrawCommand>(parent_command);

                if (!aabb_contains(parent_draw_command.bounding_box, draw_command.bounding_box)) {
                    draw_command.out_of_bounds = true;
                }
            } else {
                bool inside_bounds = false;
                auto& parent_draw_command = std::get<DrawMultipleCommand>(parent_command);

                for (auto& box : parent_draw_command.bounding_boxes) {
                    if (aabb_contains(box, draw_command.bounding_box)) {
                        inside_bounds = true;
                        break;
                    }
                }
                draw_command.out_of_bounds = !inside_bounds;
            }
        } else if (std::holds_alternative<DrawMultipleCommand>(command.command)) {
            auto& draw_command = std::get<DrawMultipleCommand>(command.command);
            for (auto idx : draw_command.cuboid_indices) {
                auto& draw_info = layer.positions[idx];
                BoundingBox bounding_box = {
                    draw_info.position,
                    draw_info.size,
                };
                bounding_box.end[0] += bounding_box.start[0];
                bounding_box.end[1] += bounding_box.start[1];
                bounding_box.end[2] += bounding_box.start[2];

                if (std::holds_alternative<DrawCommand>(parent_command)) {
                    auto& parent_draw_command = std::get<DrawCommand>(parent_command);

                    if (!aabb_contains(parent_draw_command.bounding_box, bounding_box)) {
                        draw_command.out_of_bounds.push_back(idx);
                    }
                } else {
                    bool inside_bounds = false;
                    auto& parent_draw_command = std::get<DrawMultipleCommand>(parent_command);
                    for (auto& parent_box : parent_draw_command.bounding_boxes) {
                        if (aabb_contains(parent_box, bounding_box)) {
                            inside_bounds = true;
                            break;
                        }
                    }
                    if (!inside_bounds) {
                        draw_command.out_of_bounds.push_back(idx);
                    }
                }
            }
            for (auto idx : draw_command.out_of_bounds) {
                draw_command.cuboid_indices.erase(idx);
            }
        }

        if (std::holds_alternative<NoopCommand>(command.command)) {
            auto& noop_command = std::get<NoopCommand>(command.command);
            step_parent(noop_command.counter);
        } else {
            step_parent(1);
        }
    }
}

void count_accesses(CuboidCommandList& command_list)
{
    command_list.access_counters = std::vector<std::size_t>(command_list.positions.size(), 0);

    for (auto& command : command_list.commands) {
        switch (command.type) {
        case CuboidCommandType::DRAW: {
            auto& draw_command = std::get<DrawCommand>(command.command);
            command_list.access_counters[draw_command.cuboid_idx]++;
            break;
        }
        case CuboidCommandType::DRAW_MULTIPLE: {
            auto& draw_command = std::get<DrawMultipleCommand>(command.command);
            for (auto idx : draw_command.cuboid_indices) {
                command_list.access_counters[idx] += draw_command.cuboid_accesses[idx];
            }
            break;
        }
        default:
            break;
        }
    }
}

void generate_heatmap(const CuboidCommandList& access_layer, CuboidCommandList& grid_layer)
{
    for (auto& info : access_layer.position_index_map) {
        auto& position = std::get<0>(info.first);
        auto& size = std::get<1>(info.first);

        for (auto x = position[0]; x < position[0] + size[0]; ++x) {
            for (auto y = position[1]; y < position[1] + size[1]; ++y) {
                for (auto z = position[2]; z < position[2] + size[2]; ++z) {
                    CuboidPosition cuboid_position = { x, y, z };
                    CuboidSize cuboid_size = { 1, 1, 1 };

                    if (!grid_layer.position_index_map.contains(std::make_tuple(cuboid_position, cuboid_size))) {
                        CuboidInfo cuboid_info{ cuboid_size, cuboid_position };
                        auto insertion_idx = grid_layer.positions.size();
                        grid_layer.positions.push_back(cuboid_info);
                        grid_layer.position_index_map.insert(
                            { std::make_tuple(cuboid_position, cuboid_size), insertion_idx });
                    }
                }
            }
        }
    }

    for (auto& command : access_layer.commands) {
        switch (command.type) {
        case CuboidCommandType::DRAW: {
            auto& access_draw_command = std::get<DrawCommand>(command.command);
            auto& access_cuboid_info = access_layer.positions[access_draw_command.cuboid_idx];

            auto& position = access_cuboid_info.position;
            auto& size = access_cuboid_info.size;

            if (size[0] > 1 || size[1] > 1 || size[2] > 1) {
                DrawMultipleCommand draw_command = {};
                draw_command.bounding_boxes.push_back(access_draw_command.bounding_box);

                for (auto x = position[0]; x < position[0] + size[0]; ++x) {
                    for (auto y = position[1]; y < position[1] + size[1]; ++y) {
                        for (auto z = position[2]; z < position[2] + size[2]; ++z) {
                            CuboidPosition cuboid_position = { x, y, z };
                            CuboidSize cuboid_size = { 1, 1, 1 };

                            auto idx = grid_layer.position_index_map.at(std::make_tuple(cuboid_position, cuboid_size));

                            if (!draw_command.cuboid_accesses.contains(idx)) {
                                draw_command.cuboid_accesses.insert({ idx, 1 });

                                if (access_draw_command.out_of_bounds) {
                                    draw_command.out_of_bounds.push_back(idx);
                                } else {
                                    draw_command.cuboid_indices.insert(idx);
                                }
                            } else {
                                draw_command.cuboid_accesses[idx]++;
                            }
                        }
                    }
                }

                grid_layer.commands.push_back(CuboidCommand{ CuboidCommandType::DRAW_MULTIPLE, draw_command });
            } else {
                DrawCommand draw_command = {};
                CuboidPosition cuboid_position = access_draw_command.bounding_box.start;
                CuboidSize cuboid_size = { 1, 1, 1 };

                draw_command.out_of_bounds = access_draw_command.out_of_bounds;
                draw_command.bounding_box = access_draw_command.bounding_box;
                draw_command.cuboid_idx
                    = grid_layer.position_index_map.at(std::make_tuple(cuboid_position, cuboid_size));

                grid_layer.commands.push_back(CuboidCommand{ CuboidCommandType::DRAW, draw_command });
            }

            break;
        }
        case CuboidCommandType::DRAW_MULTIPLE: {
            DrawMultipleCommand draw_command = {};
            auto& access_draw_command = std::get<DrawMultipleCommand>(command.command);
            draw_command.bounding_boxes = access_draw_command.bounding_boxes;

            for (auto access_idx : access_draw_command.cuboid_indices) {
                auto& access_cuboid_info = access_layer.positions[access_idx];
                auto& position = access_cuboid_info.position;
                auto& size = access_cuboid_info.size;
                auto accesses = access_draw_command.cuboid_accesses.at(access_idx);

                for (auto x = position[0]; x < position[0] + size[0]; ++x) {
                    for (auto y = position[1]; y < position[1] + size[1]; ++y) {
                        for (auto z = position[2]; z < position[2] + size[2]; ++z) {
                            CuboidPosition cuboid_position = { x, y, z };
                            CuboidSize cuboid_size = { 1, 1, 1 };

                            auto idx = grid_layer.position_index_map.at(std::make_tuple(cuboid_position, cuboid_size));
                            if (!draw_command.cuboid_accesses.contains(idx)) {
                                draw_command.cuboid_accesses.insert({ idx, accesses });
                                draw_command.cuboid_indices.insert(idx);
                            } else {
                                draw_command.cuboid_accesses[idx] += accesses;
                            }
                        }
                    }
                }
            }

            for (auto access_idx : access_draw_command.out_of_bounds) {
                auto& access_cuboid_info = access_layer.positions[access_idx];
                auto& position = access_cuboid_info.position;
                auto& size = access_cuboid_info.size;
                auto accesses = access_draw_command.cuboid_accesses.at(access_idx);

                for (auto x = position[0]; x < position[0] + size[0]; ++x) {
                    for (auto y = position[1]; y < position[1] + size[1]; ++y) {
                        for (auto z = position[2]; z < position[2] + size[2]; ++z) {
                            CuboidPosition cuboid_position = { x, y, z };
                            CuboidSize cuboid_size = { 1, 1, 1 };

                            auto idx = grid_layer.position_index_map.at(std::make_tuple(cuboid_position, cuboid_size));
                            if (!draw_command.cuboid_accesses.contains(idx)) {
                                draw_command.cuboid_accesses.insert({ idx, accesses });
                                draw_command.out_of_bounds.push_back(idx);
                            } else {
                                draw_command.cuboid_accesses[idx] += accesses;
                            }
                        }
                    }
                }
            }

            grid_layer.commands.push_back(CuboidCommand{ CuboidCommandType::DRAW_MULTIPLE, draw_command });
            break;
        }
        default:
            grid_layer.commands.push_back(command);
            break;
        }
    }
}

void find_max_accesses(CuboidCommandList& command_list)
{
    command_list.max_accesses = *std::max_element(command_list.access_counters.begin(),
        command_list.access_counters.end(), [](auto&& p1, auto&& p2) { return p1 < p2; });
}

ViewCommandList generate_view_command_list(const ViewContainer& view_container, VariableMap variable_map)
{
    ViewCommandList command_list{};

    command_list.id = view_container.id();
    command_list.size = view_container.size();
    command_list.invert_x = view_container.invert_x();
    command_list.invert_y = view_container.invert_y();
    command_list.invert_z = view_container.invert_z();
    command_list.border_width = view_container.border_width();
    command_list.view_name = view_container.name();
    command_list.position = view_container.position();
    command_list.border_color = view_container.border_color();
    command_list.caption_color = view_container.caption_color();
    command_list.cuboids.reserve(view_container.get_num_cuboids());

    if (view_container.camera().has_value()) {
        auto camera = view_container.camera().value();

        CameraData data = {};
        data.fixed = camera.fixed;
        data.active = camera.active;
        data.perspective = camera.perspective;
        data.fov = camera.fov;
        data.aspect = camera.aspect;
        data.near = camera.near;
        data.far = camera.far;
        data.distance = camera.distance;
        data.orthographic_width = camera.orthographic_width;
        data.orthographic_height = camera.orthographic_height;
        data.horizontal_angle = camera.horizontal_angle;
        data.vertical_angle = camera.vertical_angle;
        data.position = camera.position;
        data.rotation = camera.rotation;

        command_list.camera = data;
    }

    for (auto& cuboid_container : view_container.get_cuboids()) {
        CuboidCommandList cuboid_command_list{};
        cuboid_command_list.line_width = cuboid_container.line_width;
        cuboid_command_list.active_fill_color = cuboid_container.fill_active;
        cuboid_command_list.inactive_fill_color = cuboid_container.fill_inactive;
        cuboid_command_list.active_border_color = cuboid_container.border_active;
        cuboid_command_list.inactive_border_color = cuboid_container.border_inactive;
        cuboid_command_list.oob_active_color = cuboid_container.oob_active;
        cuboid_command_list.oob_inactive_color = cuboid_container.oob_inactive;
        command_list.cuboids.push_back(std::move(cuboid_command_list));
    }

    ContainerList container_list{};
    auto num_variables = variable_map.get_num_variables(VariableType::SEQUENTIAL)
        + variable_map.get_num_variables(VariableType::PARALLEL);

    for (std::size_t i = 0; i <= num_variables; ++i) {
        auto matches = view_container.find_matching(i);
        if (matches.empty()) {
            container_list.push_back(std::monostate{});
        } else {
            container_list.push_back(CuboidContainerTupleVec{ std::move(matches) });
        }
    }

    generate_cuboid_command_list(
        command_list.cuboids, container_list, variable_map, VariableType::SEQUENTIAL, num_variables, 0);

    for (auto layer = command_list.cuboids.begin(); layer != command_list.cuboids.end(); layer++) {
        count_accesses(*layer);
        find_max_accesses(*layer);

        for (auto sub_layer = layer + 1; sub_layer != command_list.cuboids.end(); sub_layer++) {
            generate_bounds_information(*layer, *sub_layer);
        }
    }

    auto& heatmap_opt = view_container.heatmap();
    if (heatmap_opt.has_value()) {
        auto& heatmap = heatmap_opt.value();

        command_list.heatmap = true;
        command_list.heatmap_idx = heatmap.idx;

        CuboidCommandList cuboid_command_list{};
        cuboid_command_list.line_width = 1.0f;
        cuboid_command_list.active_fill_color = { 0, 0, 0, 0 };
        cuboid_command_list.inactive_fill_color = { 0, 0, 0, 0 };
        cuboid_command_list.active_border_color = { 0, 0, 0, 0 };
        cuboid_command_list.inactive_border_color = { 0, 0, 0, 0 };
        cuboid_command_list.oob_active_color = { 0, 0, 0, 0 };
        cuboid_command_list.oob_inactive_color = { 0, 0, 0, 0 };
        command_list.cuboids.push_back(cuboid_command_list);

        command_list.cuboids.back().heatmap = { heatmap.colors_start, heatmap.colors };
        generate_heatmap(command_list.cuboids[heatmap.idx], command_list.cuboids.back());
        count_accesses(command_list.cuboids.back());
        find_max_accesses(command_list.cuboids.back());
    }

    return command_list;
}

ConfigCommandList generate_config_command_list()
{
    auto& config_instance = ConfigContainer::get_instance();
    auto variable_map = config_instance.construct_variable_map();

    ConfigCommandList command_list{};
    std::vector<std::thread> threads{};

    auto view_ids = config_instance.get_view_ids();
    for (std::size_t i = 0; i < view_ids.size(); ++i) {
        command_list.view_commands.emplace_back();
    }

    for (std::size_t i = 0; i < view_ids.size(); ++i) {
        threads.emplace_back(
            [](const ConfigContainer& config_instance, ConfigCommandList& command_list, VariableMap variable_map,
                const std::string& view_id, std::size_t idx) {
                auto& view_container = config_instance.get_view_container(view_id);
                command_list.view_commands[idx] = generate_view_command_list(view_container, std::move(variable_map));
            },
            std::cref(config_instance), std::ref(command_list), variable_map, std::cref(view_ids[i]), i);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    threads.clear();

    return command_list;
}

void print_cuboid_command_list(const std::vector<CuboidCommandList>& command_list, std::vector<std::size_t>& indices,
    std::size_t cuboid, std::vector<std::size_t>& noop_counts)
{
    std::string indentation = "";

    for (std::size_t i = 0; i < cuboid; ++i) {
        indentation += "\t";
    }

    std::cout << indentation << "CUBOID: " << cuboid << std::endl;

    while (indices[cuboid] < command_list[cuboid].commands.size()) {
        auto& command = command_list[cuboid].commands[indices[cuboid]];
        ++indices[cuboid];

        if (command.type == CuboidCommandType::NOOP) {
            if (cuboid == indices.size() - 1) {
                std::cout << indentation << "\t\tPAUSE" << std::endl;
            } else {
                auto& noop_command = std::get<NoopCommand>(command.command);
                for (std::size_t i = 0; i < noop_command.counter && noop_counts[cuboid] < noop_command.counter; ++i) {
                    for (std::size_t j = 0; j < cuboid; ++j) {
                        ++noop_counts[j];
                    }
                    print_cuboid_command_list(command_list, indices, cuboid + 1, noop_counts);
                }
                noop_counts[cuboid] = 0;
            }
        } else if (command.type == CuboidCommandType::DRAW) {
            std::cout << indentation << "\tDRAW" << std::endl;
        } else if (command.type == CuboidCommandType::DRAW_MULTIPLE) {
            std::cout << indentation << "\tDRAW_PARALLEL" << std::endl;
        } else if (command.type == CuboidCommandType::DELETE) {
            std::cout << indentation << "\tDELETE" << std::endl;
            break;
        } else if (command.type == CuboidCommandType::DELETE_MULTIPLE) {
            std::cout << indentation << "\tDELETE_PARALLEL" << std::endl;
            break;
        }
    }
}

bool aabb_contains(const BoundingBox& lhs, const BoundingBox& rhs)
{
    auto contains_x = lhs.start[0] <= rhs.start[0] && rhs.end[0] <= lhs.end[0];
    auto contains_y = lhs.start[1] <= rhs.start[1] && rhs.end[1] <= lhs.end[1];
    auto contains_z = lhs.start[2] <= rhs.start[2] && rhs.end[2] <= lhs.end[2];
    return contains_x && contains_y && contains_z;
}

bool aabb_intersects(const BoundingBox& lhs, const BoundingBox& rhs)
{
    auto intersects_x = lhs.start[0] <= rhs.end[0] && rhs.start[0] <= lhs.end[0];
    auto intersects_y = lhs.start[1] <= rhs.end[1] && rhs.start[1] <= lhs.end[1];
    auto intersects_z = lhs.start[2] <= rhs.end[2] && rhs.start[2] <= lhs.end[2];
    return intersects_x && intersects_y && intersects_z;
}

BoundingBox aabb_extend(const BoundingBox& lhs, const BoundingBox& rhs)
{
    auto box = rhs;

    if (lhs.start[0] < box.start[0]) {
        box.start[0] = lhs.start[0];
    }
    if (lhs.start[1] < box.start[1]) {
        box.start[1] = lhs.start[1];
    }
    if (lhs.start[2] < box.start[2]) {
        box.start[2] = lhs.start[2];
    }

    if (lhs.end[0] > box.end[0]) {
        box.end[0] = lhs.end[0];
    }
    if (lhs.end[1] > box.end[1]) {
        box.end[1] = lhs.end[1];
    }
    if (lhs.end[2] > box.end[2]) {
        box.end[2] = lhs.end[2];
    }

    return box;
}

void print_view_command_list(const ViewCommandList& command_list)
{
    std::cout << "VIEW START: " << command_list.view_name << std::endl;

    std::vector<std::size_t> indices{};
    std::vector<std::size_t> noop_counts = {};
    for (std::size_t i = 0; i < command_list.cuboids.size(); i++) {
        indices.push_back(0);
        noop_counts.push_back(0);
    }

    print_cuboid_command_list(command_list.cuboids, indices, 0, noop_counts);

    std::cout << "VIEW END" << std::endl << std::endl;
}

void print_config_command_list(const ConfigCommandList& config)
{
    std::cout << "DRAW COMMANDS START" << std::endl << std::endl;

    for (auto& view : config.view_commands) {
        print_view_command_list(view);
    }

    std::cout << "DRAW COMMANDS END" << std::endl << std::endl;
}
}