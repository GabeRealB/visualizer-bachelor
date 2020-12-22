#pragma once

#include <MDHConfig.hpp>

namespace MDH2Vis {

namespace detail {
    struct SequentialLayer {
        MDH2Vis::Model::Layer model;
        MDH2Vis::TPS::Layer tps;
    };

    struct MainLayerInfo {
        std::array<float, 3> scale;
        std::array<float, 3> absolute_scale;
        std::array<std::size_t, 3> num_iterations;
    };

    struct ThreadLayerInfo {
        std::array<float, 3> scale;
        std::array<float, 3> absolute_scale;
        std::array<std::size_t, 3> num_threads;
        std::array<std::size_t, 3> num_iterations;
    };

    struct SubViewLayerInfo {
        std::size_t iteration_rate;
        std::array<float, 3> scale;
        std::array<float, 3> absolute_scale;
        std::vector<std::array<float, 3>> positions;
    };

    struct OutputLayerInfo {
        std::size_t iteration_rate;
        std::array<float, 3> size;
        std::array<float, 3> absolute_size;
        std::array<std::size_t, 3> subdivisions;
        std::vector<std::size_t> iteration_rates;
        std::array<std::size_t, 3> num_iterations;
        std::vector<std::array<float, 3>> positions;
        std::vector<std::array<float, 3>> grid_positions;
        std::vector<std::array<float, 3>> absolute_positions;
    };

    struct MainViewInfo {
        std::vector<MainLayerInfo> layers;
        std::vector<ThreadLayerInfo> threads;
    };

    struct SubViewInfo {
        std::string name;
        std::vector<SubViewLayerInfo> layers;
    };

    struct OutputViewInfo {
        std::array<float, 3> size;
        std::vector<OutputLayerInfo> layers;
    };
}

struct ProcessedConfig {
    detail::MainViewInfo main_view;
    detail::OutputViewInfo output_view;
    std::vector<detail::SubViewInfo> sub_views;
    std::vector<detail::SequentialLayer> config;
};

ProcessedConfig process_config(const MDHConfig& mdh_config);

}