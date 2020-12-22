#include <MDHOps.hpp>

#include <string>
#include <thread>

#include <processing.hpp>

namespace MDH2Vis {

void process_main_view(const MDHConfig& mdh_config, ProcessedConfig& config);
void process_output_view(ProcessedConfig& previous_layer);
void process_sub_view(ProcessedConfig& operation_container, const std::string& step_x, const OperationContainer& step_y);

ProcessedConfig process_config(const MDHConfig& mdh_config)
{
    ProcessedConfig config{};

    for (auto& tps : mdh_config.tps) {
        config.config.push_back(detail::SequentialLayer{ mdh_config.model.at(tps.first), tps.second });
    }

    process_main_view(mdh_config, config);
    process_output_view(config);

    for (auto& operation : OperationMap::operations()) {
        process_sub_view(config, operation.first, operation.second);
    }

    return config;
}

template <class... Fs> struct Overload : Fs... {
    template <class... Ts>
    Overload(Ts&&... ts)
        : Fs{ std::forward<Ts>(ts) }...
    {
    }

    using Fs::operator()...;
};

template <class... Ts> Overload(Ts&&...) -> Overload<std::remove_reference_t<Ts>...>;

void process_main_view(const MDHConfig& mdh_config, ProcessedConfig& config)
{
    auto adjust_layer{ Overload(
        [](auto& current, const auto& previous) {
            current.scale = {
                current.absolute_scale[0] / previous.absolute_scale[0],
                current.absolute_scale[1] / previous.absolute_scale[1],
                current.absolute_scale[2] / previous.absolute_scale[2],
            };
        },
        [](detail::MainLayerInfo& current, const detail::MainLayerInfo& previous) {
            current.scale = {
                current.absolute_scale[0] / previous.absolute_scale[0],
                current.absolute_scale[1] / previous.absolute_scale[1],
                current.absolute_scale[2] / previous.absolute_scale[2],
            };

            current.num_iterations = {
                static_cast<std::size_t>(previous.absolute_scale[0] / current.absolute_scale[0]) - 1,
                static_cast<std::size_t>(previous.absolute_scale[1] / current.absolute_scale[1]) - 1,
                static_cast<std::size_t>(previous.absolute_scale[2] / current.absolute_scale[2]) - 1,
            };

            if (current.absolute_scale[0] == previous.absolute_scale[0]) {
                current.num_iterations[0] = 0;
            }
            if (current.absolute_scale[1] == previous.absolute_scale[1]) {
                current.num_iterations[1] = 0;
            }
            if (current.absolute_scale[2] == previous.absolute_scale[2]) {
                current.num_iterations[2] = 0;
            }
        },
        [](detail::ThreadLayerInfo& current, const detail::MainLayerInfo& previous) {
            current.scale = {
                current.absolute_scale[0] / previous.absolute_scale[0],
                current.absolute_scale[1] / previous.absolute_scale[1],
                current.absolute_scale[2] / previous.absolute_scale[2],
            };

            current.num_iterations = {
                static_cast<std::size_t>(previous.absolute_scale[0] / current.absolute_scale[0]) - 1,
                static_cast<std::size_t>(previous.absolute_scale[1] / current.absolute_scale[1]) - 1,
                static_cast<std::size_t>(previous.absolute_scale[2] / current.absolute_scale[2]) - 1,
            };

            if (current.absolute_scale[0] == previous.absolute_scale[0]) {
                current.num_iterations[0] = 0;
            }
            if (current.absolute_scale[1] == previous.absolute_scale[1]) {
                current.num_iterations[1] = 0;
            }
            if (current.absolute_scale[2] == previous.absolute_scale[2]) {
                current.num_iterations[2] = 0;
            }
        }) };

    auto adjust_layer_vector{ Overload(
        [&](std::vector<detail::MainLayerInfo>& layers) {
            for (auto pos{ layers.begin() + 1 }; pos != layers.end(); pos++) {
                adjust_layer.operator()(*pos, *(pos - 1));
            }
        },
        [&](std::vector<detail::ThreadLayerInfo>& layers) {
            for (auto pos{ layers.begin() + 1 }; pos != layers.end(); pos++) {
                adjust_layer.operator()(*pos, *(pos - 1));
            }
        }) };

    std::array<float, 3> thread_scale{ 1.0f, 1.0f, 1.0f };

    for (auto& tps : mdh_config.tps) {
        thread_scale[0] *= tps.second.numThreads[0];
        thread_scale[1] *= tps.second.numThreads[1];
        thread_scale[2] *= tps.second.numThreads[2];

        auto absolute_scale{ thread_scale };
        absolute_scale[0] *= tps.second.tileSize[0];
        absolute_scale[1] *= tps.second.tileSize[1];
        absolute_scale[2] *= tps.second.tileSize[2];

        config.main_view.layers.push_back(detail::MainLayerInfo{ absolute_scale, absolute_scale, { 0, 0, 0 } });
    }

    for (std::size_t i{ 0 }; i < config.config.size(); i++) {
        std::array<std::size_t, 3> num_threads{
            config.config[i].tps.numThreads[0],
            config.config[i].tps.numThreads[1],
            config.config[i].tps.numThreads[2],
        };
        std::array<float, 3> absolute_scale{ 1, 1, 1 };
        for (std::size_t j{ i + 1 }; j < config.config.size(); ++j) {
            absolute_scale[0] *= config.config[j].tps.numThreads[0];
            absolute_scale[1] *= config.config[j].tps.numThreads[1];
            absolute_scale[2] *= config.config[j].tps.numThreads[2];
        }

        config.main_view.threads.push_back(
            detail::ThreadLayerInfo{ absolute_scale, absolute_scale, num_threads, { 0, 0, 0 } });
    }

    adjust_layer_vector.operator()(config.main_view.layers);
    adjust_layer.operator()(config.main_view.threads.front(), config.main_view.layers.back());
    adjust_layer_vector.operator()(config.main_view.threads);
}

void process_output_view(ProcessedConfig& config)
{
    enum class DimensionType { CC, CB };
    auto combine_operations{ MDH2Vis::CombineOperations::operations() };

    std::array<DimensionType, 3> dimension_types{};

    dimension_types[0] = combine_operations.size() >= 1 && combine_operations[0].compare("CB") == 0 ? DimensionType::CB
                                                                                                    : DimensionType::CC;
    dimension_types[1] = combine_operations.size() >= 2 && combine_operations[1].compare("CB") == 0 ? DimensionType::CB
                                                                                                    : DimensionType::CC;
    dimension_types[2] = combine_operations.size() >= 3 && combine_operations[2].compare("CB") == 0 ? DimensionType::CB
                                                                                                    : DimensionType::CC;
    std::array<float, 3> thread_dimensions{ config.main_view.threads[0].absolute_scale };

    auto generate_layer{ [](std::array<DimensionType, 3> dimension_types, std::array<float, 3> thread_dimensions,
                             std::array<float, 3> layer_dimensions) -> detail::OutputLayerInfo {
        detail::OutputLayerInfo layer_info{};

        std::array<std::size_t, 3> subdivisions{ 0, 0, 0 };
        std::array<float, 3> output_dimensions{ layer_dimensions };

        if (dimension_types[0] == DimensionType::CB) {
            subdivisions[0] = 1;
            output_dimensions[0] = thread_dimensions[0];
        } else {
            subdivisions[0] = static_cast<std::size_t>(output_dimensions[0] / thread_dimensions[0]);
        }
        if (dimension_types[1] == DimensionType::CB) {
            subdivisions[1] = 1;
            output_dimensions[1] = thread_dimensions[2];
        } else {
            subdivisions[1] = static_cast<std::size_t>(output_dimensions[1] / thread_dimensions[1]);
        }
        if (dimension_types[2] == DimensionType::CB) {
            subdivisions[2] = 1;
            output_dimensions[2] = thread_dimensions[2];
        } else {
            subdivisions[2] = static_cast<std::size_t>(output_dimensions[2] / thread_dimensions[2]);
        }

        layer_info.subdivisions = subdivisions;
        layer_info.absolute_size = output_dimensions;

        return layer_info;
    } };

    auto compute_positions{ Overload{ [](detail::OutputLayerInfo& layer, std::array<float, 3> thread_dimensions,
                                          std::array<std::size_t, 3> iterations, std::size_t iteration_rate) {
                                         iterations[0]++;
                                         iterations[1]++;
                                         iterations[2]++;

                                         for (std::size_t z{ 0 }; z < iterations[2]; z++) {
                                             for (std::size_t y{ 0 }; y < iterations[1]; y++) {
                                                 for (std::size_t x{ 0 }; x < iterations[0]; x++) {
                                                     if (x >= layer.subdivisions[0] || y >= layer.subdivisions[1]
                                                         || z >= layer.subdivisions[2]) {
                                                         layer.iteration_rates.back() += iteration_rate;
                                                     } else {
                                                         std::array<float, 3> position{
                                                             x * thread_dimensions[0],
                                                             -1.0f * y * thread_dimensions[1],
                                                             z * thread_dimensions[2],
                                                         };

                                                         layer.absolute_positions.push_back(position);
                                                         layer.iteration_rates.push_back(iteration_rate);
                                                     }
                                                 }
                                             }
                                         }

                                         layer.iteration_rate = 0;
                                         for (auto rate : layer.iteration_rates) {
                                             layer.iteration_rate += rate;
                                         }
                                     },
        [](detail::OutputLayerInfo& layer, const detail::OutputLayerInfo& previousLayer,
            std::array<std::size_t, 3> iterations) {
            iterations[0]++;
            iterations[1]++;
            iterations[2]++;

            for (std::size_t z{ 0 }; z < iterations[2]; z++) {
                for (std::size_t y{ 0 }; y < iterations[1]; y++) {
                    for (std::size_t x{ 0 }; x < iterations[0]; x++) {
                        std::array<float, 3> offset{
                            x * previousLayer.absolute_size[0],
                            y * previousLayer.absolute_size[1],
                            z * previousLayer.absolute_size[2],
                        };
                        if (offset[0] >= layer.absolute_size[0] || offset[1] >= layer.absolute_size[1]
                            || offset[2] >= layer.absolute_size[2]) {
                            layer.iteration_rates.back() += previousLayer.iteration_rate;
                            continue;
                        } else {

                            offset[1] *= -1.0f;

                            for (std::size_t i{ 0 }; i < previousLayer.absolute_positions.size(); i++) {
                                std::array<float, 3> position{
                                    previousLayer.absolute_positions[i][0] + offset[0],
                                    previousLayer.absolute_positions[i][1] + offset[1],
                                    previousLayer.absolute_positions[i][2] + offset[2],
                                };

                                layer.absolute_positions.push_back(position);
                                layer.iteration_rates.push_back(previousLayer.iteration_rates[i]);
                            }
                        }
                    }
                }
            }

            layer.iteration_rate = 0;
            for (auto rate : layer.iteration_rates) {
                layer.iteration_rate += rate;
            }
        } } };

    auto compute_relative_position_and_size{ Overload{
        [](detail::OutputLayerInfo& layer, std::array<float, 3> thread_size) {
            layer.size = { 1.0f, 1.0f, 1.0f };
            layer.num_iterations = { 0, 0, 0 };
            for (auto position : layer.absolute_positions) {
                position[0] /= layer.absolute_size[0];
                position[1] /= layer.absolute_size[1];
                position[2] /= layer.absolute_size[2];
                layer.positions.push_back(position);
            }
            for (auto position : layer.absolute_positions) {
                position[0] /= thread_size[0];
                position[1] /= thread_size[1];
                position[2] /= thread_size[2];
                layer.grid_positions.push_back(position);
            }
        },
        [](detail::OutputLayerInfo& layer, const detail::OutputLayerInfo& previous_layer,
            std::array<float, 3> thread_size) {
            layer.size = {
                layer.absolute_size[0] / previous_layer.absolute_size[0],
                layer.absolute_size[1] / previous_layer.absolute_size[1],
                layer.absolute_size[2] / previous_layer.absolute_size[2],
            };
            layer.num_iterations = {
                static_cast<std::size_t>(previous_layer.absolute_size[0] / layer.absolute_size[0]) - 1,
                static_cast<std::size_t>(previous_layer.absolute_size[1] / layer.absolute_size[1]) - 1,
                static_cast<std::size_t>(previous_layer.absolute_size[2] / layer.absolute_size[2]) - 1,
            };
            for (auto position : layer.absolute_positions) {
                position[0] /= layer.absolute_size[0];
                position[1] /= layer.absolute_size[1];
                position[2] /= layer.absolute_size[2];
                layer.positions.push_back(position);
            }
            for (auto position : layer.absolute_positions) {
                position[0] /= thread_size[0];
                position[1] /= thread_size[1];
                position[2] /= thread_size[2];
                layer.grid_positions.push_back(position);
            }
        } } };

    for (const auto& layer : config.main_view.layers) {
        config.output_view.layers.push_back(generate_layer(dimension_types, thread_dimensions, layer.absolute_scale));
    }

    for (auto pos{ config.output_view.layers.rbegin() }; pos != config.output_view.layers.rend(); pos++) {
        if (pos == config.output_view.layers.rbegin()) {
            const auto& thread_layer{ config.main_view.threads.front() };
            /*
            auto iteration_rate{ static_cast<std::size_t>(thread_dimensions[0])
                * static_cast<std::size_t>(thread_dimensions[1]) * static_cast<std::size_t>(thread_dimensions[2]) };
            */
            compute_positions.operator()(*pos, thread_dimensions, thread_layer.num_iterations, 1);
        } else {
            auto& layer{ *pos };
            const auto& previous_layer{ *(pos - 1) };
            auto previous_index{ config.output_view.layers.size()
                - std::distance(config.output_view.layers.rbegin(), pos) };

            auto& previous_main_layer{ config.main_view.layers[previous_index] };
            compute_positions.operator()(layer, previous_layer, previous_main_layer.num_iterations);
        }
    }

    for (auto pos{ config.output_view.layers.begin() }; pos != config.output_view.layers.end(); pos++) {
        if (pos == config.output_view.layers.begin()) {
            compute_relative_position_and_size.operator()(*pos, thread_dimensions);
        } else {
            compute_relative_position_and_size.operator()(*pos, *(pos - 1), thread_dimensions);
        }
    }

    config.output_view.size = config.output_view.layers.front().absolute_size;
}

void process_sub_view(ProcessedConfig& config, const std::string& name, const OperationContainer& operation)
{
    auto compute_bounds_volume{ [](const MDH2Vis::OperationContainer& operation_container, std::size_t max_x,
                                    std::size_t max_y, std::size_t max_z) -> std::array<float, 3> {
        struct BufferBounds {
            std::size_t min;
            std::size_t max;
        };

        constexpr BufferBounds start_bounds{ std::numeric_limits<std::size_t>::max(),
            std::numeric_limits<std::size_t>::min() };

        auto compute_bounds_sub_volume{ [](std::array<BufferBounds, 3>& bounds,
                                            const MDH2Vis::OperationContainer& operation_container, std::size_t min_x,
                                            std::size_t max_x, std::size_t max_y, std::size_t max_z) {
            auto compute_bounds_single{ [](BufferBounds& bounds, const MDH2Vis::Operation& operation, std::size_t i1,
                                            std::size_t i2, std::size_t i3) {
                auto value{ operation(i1, i2, i3) };

                if (bounds.min > value) {
                    bounds.min = value;
                }

                if (bounds.max < value) {
                    bounds.max = value;
                }
            } };

            for (std::size_t i1 = min_x; i1 < max_x; ++i1) {
                for (std::size_t i2 = 0; i2 < max_y; ++i2) {
                    for (std::size_t i3 = 0; i3 < max_z; ++i3) {
                        for (std::size_t i = 0; i < operation_container.x.size(); ++i) {
                            compute_bounds_single(bounds[0], operation_container.x[i], i1, i2, i3);
                        }
                        for (std::size_t i = 0; i < operation_container.y.size(); ++i) {
                            compute_bounds_single(bounds[1], operation_container.y[i], i1, i2, i3);
                        }
                        for (std::size_t i = 0; i < operation_container.z.size(); ++i) {
                            compute_bounds_single(bounds[2], operation_container.z[i], i1, i2, i3);
                        }
                    }
                }
            }
        } };

        auto num_threads{ std::thread::hardware_concurrency() };

        auto x_step{ max_x };
        if (num_threads != 1) {
            x_step = max_x / num_threads;
        }

        std::vector<std::array<BufferBounds, 3>> buffer{};
        buffer.resize(num_threads, { start_bounds, start_bounds, start_bounds });

        std::vector<std::thread> threads{};
        threads.reserve(num_threads);

        auto x_min_cmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[0].min < rhs[0].min;
        } };
        auto x_max_cmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[0].max < rhs[0].max;
        } };

        auto y_min_cmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[1].min < rhs[1].min;
        } };
        auto y_max_cmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[1].max < rhs[1].max;
        } };

        auto z_min_cmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[2].min < rhs[2].min;
        } };
        auto z_max_cmp{ [](const std::array<BufferBounds, 3>& lhs, const std::array<BufferBounds, 3>& rhs) {
            return lhs[2].max < rhs[2].max;
        } };

        for (std::size_t i = 0; i < num_threads; i++) {
            if (i == num_threads - 1) {
                threads.push_back(std::thread(compute_bounds_sub_volume, std::ref(buffer[i]),
                    std::ref(operation_container), i * x_step, max_x, max_y, max_z));
            } else {
                threads.push_back(std::thread(compute_bounds_sub_volume, std::ref(buffer[i]),
                    std::ref(operation_container), i * x_step, (i + 1) * x_step, max_y, max_z));
            }
        }

        for (auto& thread : threads) {
            thread.join();
        }

        std::array<BufferBounds, 3> bounds{};

        bounds[0].min = std::min(bounds[0].min, std::min_element(buffer.begin(), buffer.end(), x_min_cmp)->at(0).min);
        bounds[0].max = std::max(bounds[0].max, std::max_element(buffer.begin(), buffer.end(), x_max_cmp)->at(0).max);

        bounds[1].min = std::min(bounds[1].min, std::min_element(buffer.begin(), buffer.end(), y_min_cmp)->at(1).min);
        bounds[1].max = std::max(bounds[1].max, std::max_element(buffer.begin(), buffer.end(), y_max_cmp)->at(1).max);

        bounds[2].min = std::min(bounds[2].min, std::min_element(buffer.begin(), buffer.end(), z_min_cmp)->at(2).min);
        bounds[2].max = std::max(bounds[2].max, std::max_element(buffer.begin(), buffer.end(), z_max_cmp)->at(2).max);

        return {
            static_cast<float>(bounds[0].max - bounds[0].min + 1),
            static_cast<float>(bounds[1].max - bounds[1].min + 1),
            static_cast<float>(bounds[2].max - bounds[2].min + 1),
        };
    } };

    auto compute_iteration_positions{ [](const MDH2Vis::OperationContainer& operation_container, std::uint32_t step_x,
                                        std::uint32_t step_y, std::uint32_t step_z, std::uint32_t max_x,
                                        std::uint32_t max_y, std::uint32_t max_z, float size_x, float size_y,
                                        float size_z) -> std::vector<std::array<float, 3>> {
        auto compute_min{ [](float& min, const MDH2Vis::Operation& operation, std::size_t i1, std::size_t i2,
                             std::size_t i3) {
            auto value{ static_cast<float>(operation(i1, i2, i3)) };

            if (min > value) {
                min = value;
            }
        } };

        std::vector<std::array<float, 3>> positions{};

        for (std::size_t i3 = 0; i3 < max_z; i3 += step_z) {
            for (std::size_t i2 = 0; i2 < max_y; i2 += step_y) {
                for (std::size_t i1 = 0; i1 < max_x; i1 += step_x) {
                    std::array<float, 3> position{};
                    position.fill(std::numeric_limits<float>::max());

                    for (std::size_t i = 0; i < operation_container.x.size(); ++i) {
                        compute_min(position[0], operation_container.x[i], i1, i2, i3);
                    }
                    for (std::size_t i = 0; i < operation_container.y.size(); ++i) {
                        compute_min(position[1], operation_container.y[i], i1, i2, i3);
                    }
                    for (std::size_t i = 0; i < operation_container.z.size(); ++i) {
                        compute_min(position[2], operation_container.z[i], i1, i2, i3);
                    }

                    position[0] /= size_x;
                    position[1] /= size_y;
                    position[2] /= size_z;

                    positions.push_back(position);
                }
            }
        }

        return positions;
    } };

    detail::SubViewInfo view{};
    view.name = name;

    for (const auto& layer : config.main_view.layers) {
        auto scale{ compute_bounds_volume(
            operation, layer.absolute_scale[0], layer.absolute_scale[1], layer.absolute_scale[2]) };
        view.layers.push_back(detail::SubViewLayerInfo{ 0, scale, scale, {} });
    }

    std::size_t thread_layer_size = config.main_view.threads[0].absolute_scale[0]
        * config.main_view.threads[0].absolute_scale[1] * config.main_view.threads[0].absolute_scale[2];

    for (auto pos{ view.layers.begin() }; pos != view.layers.end(); pos++) {

        auto index{ std::distance(view.layers.begin(), pos) };

        const auto& current_layer{ *pos };
        const auto& main_view_layer{ config.main_view.layers[index] };

        pos->iteration_rate
            = main_view_layer.absolute_scale[0] * main_view_layer.absolute_scale[1] * main_view_layer.absolute_scale[2];
        pos->iteration_rate /= thread_layer_size;

        if (pos == view.layers.begin()) {
            pos->positions = { { 0.0f, 0.0f, 0.0f } };
        } else {
            const auto& previous_layer{ *(pos - 1) };
            const auto& previous_main_view_layer{ config.main_view.layers[index - 1] };

            pos->scale = {
                current_layer.absolute_scale[0] / previous_layer.absolute_scale[0],
                current_layer.absolute_scale[1] / previous_layer.absolute_scale[1],
                current_layer.absolute_scale[2] / previous_layer.absolute_scale[2],
            };

            pos->positions = compute_iteration_positions(operation, main_view_layer.absolute_scale[0],
                main_view_layer.absolute_scale[1], main_view_layer.absolute_scale[2],
                previous_main_view_layer.absolute_scale[0], previous_main_view_layer.absolute_scale[1],
                previous_main_view_layer.absolute_scale[2],
                current_layer.absolute_scale[0], current_layer.absolute_scale[1], current_layer.absolute_scale[2]);
        }
    }

    config.sub_views.push_back(view);
}

}