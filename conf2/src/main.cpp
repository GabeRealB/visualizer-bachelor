#include "output.hpp"

#include <filesystem>
#include <iostream>
#include <variant>

struct Print {
};

struct Generate {
    bool screen_fullscreen;
    std::size_t screen_width;
    std::size_t screen_height;
    std::size_t screen_msaa_samples;
    std::size_t render_resolution_multiplier;
    std::filesystem::path output_path;
    std::filesystem::path resource_path;
};

constexpr std::string_view usage_string
    = "Usage: conf2 [print | generate [-o output_path] [-r resource_path] [-sw screen_width] [-sh screen_height] "
      "[-msaa msaa_samples] [-rmul resolution_multiplier] [-fs]]";

using OperationVariant = std::variant<std::monostate, Print, Generate>;

void print_handler(const Config::ConfigCommandList& command_list);
void generate_handler(const Generate& operation, const Config::ConfigCommandList& command_list);

int main(int argc, char* argv[])
{
    if (argc == 1) {
        std::cerr << usage_string << std::endl;
        return 1;
    }

    const OperationVariant operation_variant = [&]() -> OperationVariant {
        if (strcmp(argv[1], "print") == 0) {
            return Print{};
        } else if (strcmp(argv[1], "generate") == 0) {
            Generate options{};

            bool screen_width = false;
            bool screen_height = false;
            bool screen_msaa_samples = false;
            bool render_resolution_multiplier = false;
            bool output_path = false;
            bool resource_path = false;

            for (int i = 2; i < argc; i++) {
                if (strcmp(argv[i], "-o") == 0) {
                    if (i + 1 >= argc) {
                        std::cerr << "Too few arguments after -o, expected 1, got 0." << std::endl;
                        return std::monostate{};
                    }

                    i++;
                    output_path = true;
                    options.output_path = std::filesystem::path{ argv[i] };
                } else if (strcmp(argv[i], "-r") == 0) {
                    if (i + 1 >= argc) {
                        std::cerr << "Too few arguments after -r, expected 1, got 0." << std::endl;
                        return std::monostate{};
                    }

                    i++;
                    resource_path = true;
                    options.resource_path = std::filesystem::path{ argv[i] };
                } else if (strcmp(argv[i], "-fs") == 0) {
                    options.screen_fullscreen = true;
                } else if (strcmp(argv[i], "-sw") == 0) {
                    if (i + 1 >= argc) {
                        std::cerr << "Too few arguments after -sw, expected 1, got 0." << std::endl;
                        return std::monostate{};
                    }

                    i++;
                    screen_width = true;
                    options.screen_width = std::atoi(argv[i]);
                } else if (strcmp(argv[i], "-sh") == 0) {
                    if (i + 1 >= argc) {
                        std::cerr << "Too few arguments after -sh, expected 1, got 0." << std::endl;
                        return std::monostate{};
                    }

                    i++;
                    screen_height = true;
                    options.screen_height = std::atoi(argv[i]);
                } else if (strcmp(argv[i], "-msaa") == 0) {
                    if (i + 1 >= argc) {
                        std::cerr << "Too few arguments after -msaa, expected 1, got 0." << std::endl;
                        return std::monostate{};
                    }

                    i++;
                    screen_msaa_samples = true;
                    options.screen_msaa_samples = std::atoi(argv[i]);
                } else if (strcmp(argv[i], "-rmul") == 0) {
                    if (i + 1 >= argc) {
                        std::cerr << "Too few arguments after -rmul, expected 1, got 0." << std::endl;
                        return std::monostate{};
                    }

                    i++;
                    render_resolution_multiplier = true;
                    options.render_resolution_multiplier = std::atoi(argv[i]);
                }
            }

            constexpr double aspect_ration = 4.0 / 3.0;

            if (!screen_width) {
                options.screen_width = 1200;
            } else {
                options.screen_height
                    = static_cast<std::size_t>(static_cast<double>(options.screen_width) / aspect_ration);
                screen_height = true;
            }
            if (!screen_height) {
                options.screen_height = 900;
                options.screen_width
                    = static_cast<std::size_t>(static_cast<double>(options.screen_height) * aspect_ration);
            }
            if (!screen_msaa_samples) {
                options.screen_msaa_samples = 32;
            }
            if (!render_resolution_multiplier) {
                options.render_resolution_multiplier = 2;
            }
            if (!output_path) {
                options.output_path = std::filesystem::current_path();
            }
            if (!resource_path) {
                options.resource_path = std::filesystem::current_path();
            }

            return options;
        }

        return std::monostate{};
    }();

    if (std::holds_alternative<std::monostate>(operation_variant)) {
        std::cerr << usage_string << std::endl;
        return 1;
    }

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
    generation_options.screen_width = operation.screen_width;
    generation_options.screen_height = operation.screen_height;
    generation_options.screen_msaa_samples = operation.screen_msaa_samples;
    generation_options.screen_fullscreen = operation.screen_fullscreen;

    generation_options.render_resolution_multiplier = operation.render_resolution_multiplier;

    generation_options.cuboid_border_width = 0.08f;

    generation_options.camera_fov = 70.0f;
    generation_options.camera_aspect
        = static_cast<float>(generation_options.screen_width) / static_cast<float>(generation_options.screen_height);
    generation_options.camera_near = 0.3f;
    generation_options.camera_far = 10000.0f;

    generation_options.min_transparency = 0.1f;
    generation_options.max_transparency = 0.95f;

    generation_options.working_directory = operation.output_path;
    generation_options.resource_directory = operation.resource_path;

    generation_options.assets_directory_path = "external_assets";
    generation_options.assets_texture_directory_path = generation_options.assets_directory_path / "textures";

    generation_options.cuboid_diffuse_shader_vertex_path = "assets/shaders/cuboid.vs.glsl";
    generation_options.cuboid_diffuse_shader_fragment_path = "assets/shaders/cuboid_diffuse.fs.glsl";

    generation_options.cuboid_oit_shader_vertex_path = "assets/shaders/cuboid.vs.glsl";
    generation_options.cuboid_oit_shader_fragment_path = "assets/shaders/cuboid_oit.fs.glsl";

    generation_options.cuboid_oit_blend_shader_vertex_path = "assets/shaders/cuboid_oit_blend.vs.glsl";
    generation_options.cuboid_oit_blend_shader_fragment_path = "assets/shaders/cuboid_oit_blend.fs.glsl";

    generation_options.view_composition_shader_vertex_path = "assets/shaders/compositing.vs.glsl";
    generation_options.view_composition_shader_fragment_path = "assets/shaders/compositing.fs.glsl";

    generation_options.cuboid_pipeline_name = "cuboid";
    generation_options.cuboid_mesh_asset_name = "cuboid_mesh";
    generation_options.cuboid_oit_shader_asset_name = "cuboid_oit_shader";
    generation_options.cuboid_diffuse_shader_asset_name = "cuboid_diffuse_shader";
    generation_options.default_framebuffer_asset_name = "default_framebuffer";
    generation_options.fullscreen_quad_mesh_asset_name = "fullscreen_quad_mesh";
    generation_options.cuboid_oit_blend_shader_asset_name = "cuboid_oit_blend_shader";
    generation_options.view_composition_shader_asset_name = "view_composition_shader";

    auto config = Config::generate_config(command_list, generation_options);
    Visconfig::to_file(operation.output_path / "visconfig", config);
}
