#include "output.hpp"

#include <filesystem>
#include <iostream>
#include <variant>

struct Print {
};

struct Generate {
    std::filesystem::path output_path;
};

using OperationVariant = std::variant<std::monostate, Print, Generate>;

void print_handler(const Config::ConfigCommandList& command_list);
void generate_handler(const Generate& operation, const Config::ConfigCommandList& command_list);

int main(int argc, char* argv[])
{
    if (argc != 2 && argc != 4) {
        std::cerr << "Usage: conf2 [print | generate [-o output_path]]" << std::endl;
        return 1;
    }

    const OperationVariant operation_variant = [&]() -> OperationVariant {
        if (strcmp(argv[1], "print") == 0) {
            return Print{};
        } else if (strcmp(argv[1], "generate") == 0) {
            if (argc == 2) {
                return Generate{ std::filesystem::current_path() };
            } else if (strcmp(argv[2], "-o") == 0) {
                return Generate{ std::filesystem::path{ argv[3] } };
            }
        }

        return std::monostate{};
    }();

    auto command_list = Config::generate_config_command_list();

    std::visit(
        [&](auto&& operation) {
            if constexpr (std::is_same_v<decltype(operation), const Print&>) {
                print_handler(command_list);
            } else if constexpr (std::is_same_v<decltype(operation), const Generate&>) {
                generate_handler(operation, command_list);
            }
        },
        operation_variant);
    return 0;
}

void print_handler(const Config::ConfigCommandList& command_list) { Config::print_config_command_list(command_list); }

void generate_handler(const Generate& operation, const Config::ConfigCommandList& command_list)
{
    Config::GenerationOptions generation_options{};
    generation_options.screen_width = 1200;
    generation_options.screen_height = 900;
    generation_options.screen_msaa_samples = 32;
    generation_options.screen_fullscreen = false;

    generation_options.render_resolution_multiplier = 2;

    generation_options.cuboid_texture_border_relative_width = 0.04f;

    generation_options.camera_fov = 70.0f;
    generation_options.camera_aspect
        = static_cast<float>(generation_options.screen_width) / static_cast<float>(generation_options.screen_height);
    generation_options.camera_near = 0.3f;
    generation_options.camera_far = 10000.0f;

    generation_options.min_transparency = 0.1f;
    generation_options.max_transparency = 0.95f;

    generation_options.assets_directory_path = operation.output_path / "external_assets";
    generation_options.assets_texture_directory_path = generation_options.assets_directory_path / "textures";

    generation_options.cuboid_shader_vertex_path = "assets/shaders/cuboid.vs.glsl";
    generation_options.cuboid_shader_fragment_path = "assets/shaders/cuboid.fs.glsl";

    generation_options.cuboid_oit_blend_shader_vertex_path = "assets/shaders/cuboid_oit_blend.vs.glsl";
    generation_options.cuboid_oit_blend_shader_fragment_path = "assets/shaders/cuboid_oit_blend.fs.glsl";

    generation_options.view_composition_shader_vertex_path = "assets/shaders/compositing.vs.glsl";
    generation_options.view_composition_shader_fragment_path = "assets/shaders/compositing.fs.glsl";

    generation_options.cuboid_mesh_asset_name = "cuboid_mesh";
    generation_options.cuboid_shader_asset_name = "cuboid_shader";
    generation_options.default_framebuffer_asset_name = "default_framebuffer";
    generation_options.fullscreen_quad_mesh_asset_name = "fullscreen_quad_mesh";
    generation_options.cuboid_oit_blend_shader_asset_name = "cuboid_oit_blend_shader";
    generation_options.view_composition_shader_asset_name = "view_composition_shader";

    auto config = Config::generate_config(command_list, generation_options);
    Visconfig::to_file(operation.output_path / "visconfig", config);
}
