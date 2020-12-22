/// Standard includes
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <thread>

/// External libraries
#include <visconfig/Config.hpp>

/// Internal includes
#include "MDHConfig.hpp"
#include "MDHOps.hpp"
#include "generation.hpp"

constexpr std::string_view usage_str{ "Usage: mdh2vis --model model-path --tps tps-path [--out output-dir]" };

constexpr std::size_t screen_width = 1200;
constexpr std::size_t screen_height = 900;
constexpr std::size_t screen_msaa_samples = 16;
constexpr bool screen_fullscreen = false;

constexpr std::size_t render_resolution_multiplier{ 2 };

constexpr float main_view_texture_border_relative_width{ 0.02f };
constexpr float thread_view_texture_border_relative_width{ 0.05f };
constexpr float sub_view_texture_border_relative_width{ 0.4f };

constexpr float camera_fov{ 70.0f };
constexpr float camera_aspect{ static_cast<float>(screen_width) / static_cast<float>(screen_height) };
constexpr float camera_aspect_small{ camera_aspect / 2 };
constexpr float camera_near{ 0.3f };
constexpr float camera_far{ 10000.0f };

constexpr float min_transparency{ 0.1f };
constexpr float max_transparency{ 0.95f };

constexpr auto assets_directory_path{ "external_assets" };
constexpr auto assets_texture_directory_path{ "external_assets/textures" };

constexpr auto cube_mesh_asset_name{ "cube_mesh" };
constexpr auto cube_texture_asset_name{ "cube_texture" };
constexpr auto output_cube_texture_asset_name{ "output_cube_texture" };
constexpr auto cube_shader_asset_name{ "cube_shader" };
constexpr auto default_framebuffer_asset_name{ "default_framebuffer" };
constexpr auto view_composition_shader_asset_name{ "view_composition_shader" };

constexpr auto cube_shader_vertex_path{ "assets/shaders/cube.vs.glsl" };
constexpr auto cube_shader_fragment_path{ "assets/shaders/cube.fs.glsl" };

constexpr auto view_composition_shader_vertex_path{ "assets/shaders/compositing.vs.glsl" };
constexpr auto view_composition_shader_fragment_path{ "assets/shaders/compositing.fs.glsl" };

void print_config_info(const MDH2Vis::ProcessedConfig& config);

int main(int argc, char* argv[])
{
    if (argc != 5 && argc != 7) {
        std::cerr << usage_str << std::endl;
        return 1;
    }

    auto working_dir{ std::filesystem::current_path() };

    std::filesystem::path model_path{};
    std::filesystem::path tps_path{};

    bool model_set{ false };
    bool tps_set{ false };

    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], "--model") == 0) {
            model_path = argv[i + 1];
            model_set = true;
            ++i;
        } else if (std::strcmp(argv[i], "--tps") == 0) {
            tps_path = argv[i + 1];
            tps_set = true;
            ++i;
        } else if (std::strcmp(argv[i], "--out") == 0) {
            working_dir = argv[i + 1];
            ++i;
        } else {
            std::cerr << usage_str << std::endl;
            return 1;
        }
    }

    if (!model_set || !tps_set) {
        std::cerr << usage_str << std::endl;
        return 1;
    }

    if (!std::filesystem::exists(working_dir)) {
        std::cerr << "Could not find " << working_dir << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(model_path)) {
        std::cerr << "Could not find " << model_path << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(tps_path)) {
        std::cerr << "Could not find " << tps_path << std::endl;
        return 1;
    }

    std::cout << "Paths:" << std::endl;
    std::cout << "\tOutput:" << working_dir << std::endl;
    std::cout << "\tModel:" << model_path << std::endl;
    std::cout << "\tTPS:" << tps_path << std::endl;

    auto mdh_config{ MDH2Vis::loadFromFiles(model_path, tps_path) };
    if (!mdh_config) {
        std::cerr << "Could not load configs" << std::endl;
        return 1;
    }

    auto config{ MDH2Vis::process_config(*mdh_config) };
    print_config_info(config);

    MDH2Vis::GenerationOptions generation_options{};
    generation_options.screen_width = screen_width;
    generation_options.screen_height = screen_height;
    generation_options.screen_msaa_samples = screen_msaa_samples;
    generation_options.screen_fullscreen = screen_fullscreen;

    generation_options.render_resolution_multiplier = render_resolution_multiplier;

    generation_options.main_view_texture_border_relative_width = main_view_texture_border_relative_width;
    generation_options.thread_view_texture_border_relative_width = thread_view_texture_border_relative_width;
    generation_options.sub_view_texture_border_relative_width = sub_view_texture_border_relative_width;

    generation_options.camera_fov = camera_fov;
    generation_options.camera_aspect = camera_aspect;
    generation_options.camera_aspect_small = camera_aspect_small;
    generation_options.camera_near = camera_near;
    generation_options.camera_far = camera_far;

    generation_options.min_transparency = min_transparency;
    generation_options.max_transparency = max_transparency;

    generation_options.working_dir = working_dir;

    generation_options.assets_directory_path = assets_directory_path;
    generation_options.assets_texture_directory_path = assets_texture_directory_path;

    generation_options.cube_shader_vertex_path = cube_shader_vertex_path;
    generation_options.cube_shader_fragment_path = cube_shader_fragment_path;

    generation_options.view_composition_shader_vertex_path = view_composition_shader_vertex_path;
    generation_options.view_composition_shader_fragment_path = view_composition_shader_fragment_path;

    generation_options.cube_mesh_asset_name = cube_mesh_asset_name;
    generation_options.cube_texture_asset_name = cube_texture_asset_name;
    generation_options.output_cube_texture_asset_name = output_cube_texture_asset_name;
    generation_options.cube_shader_asset_name = cube_shader_asset_name;
    generation_options.default_framebuffer_asset_name = default_framebuffer_asset_name;
    generation_options.view_composition_shader_asset_name = view_composition_shader_asset_name;

    MDH2Vis::generate_assets_directory(generation_options);
    auto vis_config{ MDH2Vis::generate_config(config, generation_options) };
    Visconfig::to_file(working_dir / "visconfig.json", vis_config);
}

void print_config_info(const MDH2Vis::ProcessedConfig& config)
{
    std::cout << "Config info:" << std::endl;

    std::cout << "Number of layers: " << config.config.size() << std::endl;
    std::cout << "Dimensions: " << config.config[0].tps.tileSize[0] << ", " << config.config[0].tps.tileSize[1] << ", "
              << config.config[0].tps.tileSize[2] << std::endl;
    std::cout << std::endl;
}
