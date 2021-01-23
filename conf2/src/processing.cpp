#include "processing.hpp"
#include "Config.hpp"

#include <thread>

namespace Config {

std::vector<std::set<std::string>> get_variable_combinations(
    const VariableMap& variable_map, VariableType variable_type, std::size_t index)
{
    if (index == 0) {
        return std::vector<std::set<std::string>>{ {} };
    } else {
        auto& variable_name = [&]() -> auto&
        {
            if (variable_type == VariableType::SEQUENTIAL) {
                return variable_map.get_variable_name(VariableType::SEQUENTIAL, index - 1);
            } else {
                auto i = index - variable_map.get_num_variables(VariableType::SEQUENTIAL);
                return variable_map.get_variable_name(VariableType::PARALLEL, i - 1);
            }
        }
        ();

        auto num_sequential_vars = variable_map.get_num_variables(VariableType::SEQUENTIAL);

        std::set<std::set<std::string>> variable_combinations{};
        for (std::size_t i = 0; i < index; i++) {
            auto combinations = get_variable_combinations(
                variable_map, i <= num_sequential_vars ? VariableType::SEQUENTIAL : VariableType::PARALLEL, i);
            for (auto& combination : combinations) {
                variable_combinations.insert(combination);
            }
        }

        std::vector<std::set<std::string>> combinations_vec{};
        combinations_vec.reserve(variable_combinations.size());

        for (auto& combination : variable_combinations) {
            combinations_vec.push_back(combination);
        }

        for (auto& combination : combinations_vec) {
            combination.insert(variable_name);
        }

        return combinations_vec;
    }
}

using CuboidContainerTupleVec = std::tuple<std::size_t, std::vector<CuboidContainer>>;
using ContainerList = std::vector<std::variant<std::monostate, CuboidContainerTupleVec>>;

void generate_cuboid_command_list(std::vector<CuboidCommandList>& command_list, const ContainerList& container_list,
    VariableMap& variable_map, [[maybe_unused]] VariableType variable_type, [[maybe_unused]] std::size_t end_idx,
    std::size_t idx)
{
    auto noop_command_lambda = [&](auto&& value) {
        if constexpr (std::is_same_v<decltype(value), const CuboidContainerTupleVec&>) {
            auto startIdx = std::get<0>(value);
            auto& cuboids = std::get<1>(value);

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                auto& cuboid_commands = command_list[startIdx + i].commands;

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
            auto startIdx = std::get<0>(value);
            auto& cuboids = std::get<1>(value);

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                auto position_data = cuboids[i].pos_size_callable(variable_map);
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
                command_list[startIdx + i].commands.push_back({ CuboidCommandType::DRAW,
                    DrawCommand{
                        cuboid_size,
                        start_position,
                        cuboids[i].fill_color,
                        cuboids[i].active_color,
                    } });
            }
        }
    };

    auto draw_multiple_command_lambda = [&](auto&& value) {
        if constexpr (std::is_same_v<decltype(value), const CuboidContainerTupleVec&>) {
            auto startIdx = std::get<0>(value);
            auto& cuboids = std::get<1>(value);

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                auto position_data = cuboids[i].pos_size_callable(variable_map);
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

                auto& cuboid_commands = command_list[startIdx + i].commands;

                if (cuboid_commands.empty() || cuboid_commands.back().type != CuboidCommandType::DRAW_MULTIPLE) {
                    cuboid_commands.push_back({ CuboidCommandType::DRAW_MULTIPLE,
                        DrawMultipleCommand{
                            cuboids[i].fill_color,
                            cuboids[i].active_color,
                            { cuboid_size },
                            { start_position },
                        } });
                } else {
                    auto& command = std::get<DrawMultipleCommand>(cuboid_commands.back().command);
                    command.cuboid_sizes.push_back(cuboid_size);
                    command.start_positions.push_back(start_position);
                }
            }
        }
    };

    auto delete_command_lambda = [&](auto&& value) {
        if constexpr (std::is_same_v<decltype(value), const CuboidContainerTupleVec&>) {
            auto startIdx = std::get<0>(value);
            auto& cuboids = std::get<1>(value);

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                command_list[startIdx + i].commands.push_back({ CuboidCommandType::DELETE,
                    DeleteCommand{
                        cuboids[i].fill_color,
                        cuboids[i].unused_color,
                    } });
            }
        }
    };

    auto delete_multiple_command_lambda = [&](auto&& value) {
        if constexpr (std::is_same_v<decltype(value), const CuboidContainerTupleVec&>) {
            auto startIdx = std::get<0>(value);
            auto& cuboids = std::get<1>(value);

            for (std::size_t i = 0; i < cuboids.size(); i++) {
                auto& cuboid_commands = command_list[startIdx + i].commands;

                if (cuboid_commands.empty() || cuboid_commands.back().type != CuboidCommandType::DELETE_MULTIPLE) {
                    command_list[startIdx + i].commands.push_back({ CuboidCommandType::DELETE_MULTIPLE,
                        DeleteMultipleCommand{
                            0,
                            cuboids[i].fill_color,
                            cuboids[i].unused_color,
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

        if (idx < end_idx) {
            generate_cuboid_command_list(command_list, container_list, variable_map,
                idx + 1 <= num_seq_vars ? VariableType::SEQUENTIAL : VariableType::PARALLEL, end_idx, idx + 1);
        }

        if (idx <= num_seq_vars) {
            std::visit(delete_command_lambda, containers);

            if (idx == num_seq_vars) {
                for (std::size_t i = 0; i <= end_idx; i++) {
                    std::visit(noop_command_lambda, container_list[i]);
                    if (i > idx) {
                        std::visit(delete_multiple_command_lambda, container_list[i]);
                    }
                }
            }
        }
    }
}

using VariablePowerSet = std::vector<std::vector<std::set<std::string>>>;

ViewCommandList generate_view_command_list(const std::string& view_name, const ViewContainer& view_container,
    const VariablePowerSet& variable_power_set, VariableMap variable_map)
{
    ViewCommandList command_list{};

    command_list.view_name = view_name;
    command_list.cuboids.reserve(view_container.get_num_cuboids());

    for (std::size_t i = 0; i < view_container.get_num_cuboids(); i++) {
        command_list.cuboids.push_back({});
    }

    ContainerList container_list{};
    for (std::size_t i = 0, containers = 0; i < variable_power_set.size(); i++) {
        std::vector<CuboidContainer> cuboid_containers{};
        for (const auto& variables : variable_power_set[i]) {
            auto matching_containers = view_container.find_matching(variables);
            for (auto& container : matching_containers) {
                cuboid_containers.push_back(container);
            }
        }

        if (cuboid_containers.empty()) {
            container_list.push_back(std::monostate{});
        } else {
            auto container_start_index = containers;
            containers += cuboid_containers.size();
            container_list.push_back(std::tuple{ container_start_index, std::move(cuboid_containers) });
        }
    }

    generate_cuboid_command_list(
        command_list.cuboids, container_list, variable_map, VariableType::SEQUENTIAL, variable_power_set.size() - 1, 0);

    return command_list;
}

ConfigCommandList generate_config_command_list()
{
    auto& config_instance = ConfigContainer::get_instance();
    auto variable_map = config_instance.construct_variable_map();

    ConfigCommandList command_list{};
    std::vector<std::thread> threads{};
    VariablePowerSet variable_power_set{};

    auto view_names = config_instance.get_view_names();
    for (std::size_t i = 0; i < view_names.size(); ++i) {
        command_list.view_commands.emplace_back();
    }

    for (std::size_t i = 0; i <= variable_map.get_num_variables(VariableType::SEQUENTIAL)
             + variable_map.get_num_variables(VariableType::PARALLEL);
         ++i) {
        variable_power_set.emplace_back();
    }

    for (std::size_t i = 0; i <= variable_map.get_num_variables(VariableType::SEQUENTIAL)
             + variable_map.get_num_variables(VariableType::PARALLEL);
         ++i) {
        threads.emplace_back(
            [](VariablePowerSet& variable_power_set, const VariableMap& variable_map, std::size_t idx) {
                if (idx <= variable_map.get_num_variables(VariableType::SEQUENTIAL)) {
                    variable_power_set[idx] = get_variable_combinations(variable_map, VariableType::SEQUENTIAL, idx);
                } else {
                    variable_power_set[idx] = get_variable_combinations(variable_map, VariableType::PARALLEL, idx);
                }
            },
            std::ref(variable_power_set), std::cref(variable_map), i);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    threads.clear();

    for (std::size_t i = 0; i < view_names.size(); ++i) {
        threads.emplace_back(
            [](const ConfigContainer& config_instance, ConfigCommandList& command_list,
                const VariablePowerSet& variable_power_set, VariableMap variable_map, const std::string& view_name,
                std::size_t idx) {
                auto& view_container = config_instance.get_view_container(view_name);
                command_list.view_commands[idx] = generate_view_command_list(
                    view_name, view_container, variable_power_set, std::move(variable_map));
            },
            std::cref(config_instance), std::ref(command_list), std::cref(variable_power_set), variable_map,
            std::cref(view_names[i]), i);
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