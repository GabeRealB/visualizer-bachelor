#include <generation.hpp>

#include "asset_creation.hpp"
#include "entity_generation.hpp"

namespace MDH2Vis {

void generate_world(
    Visconfig::Config& config, const ProcessedConfig& mdh_config, const GenerationOptions& generation_options);
void generate_main_view_config(Visconfig::Config& config, const ProcessedConfig& mdh_config, Visconfig::World& world,
    std::size_t& num_entities, const GenerationOptions& generation_options);
void generate_output_view_config(Visconfig::Config& config, const ProcessedConfig& mdh_config, Visconfig::World& world,
    std::size_t& num_entities, std::size_t subview, const GenerationOptions& generation_options);
void generate_sub_view_config(Visconfig::Config& config, const ProcessedConfig& mdh_config, Visconfig::World& world,
    std::size_t& num_entities, std::size_t subview, std::size_t num_sub_views,
    const GenerationOptions& generation_options);

void generate_assets_directory(const GenerationOptions& generation_options)
{
    auto asset_path{ generation_options.working_dir / generation_options.assets_directory_path };
    auto asset_texture_path{ asset_path / generation_options.assets_texture_directory_path };

    if (std::filesystem::exists(asset_path)) {
        std::filesystem::remove_all(asset_path);
    }

    std::filesystem::create_directories(asset_path);
    std::filesystem::create_directory(asset_texture_path);
}

Visconfig::Config generate_config(const ProcessedConfig& config, const GenerationOptions& generation_options)
{
    Visconfig::Config visualizer_config{};

    visualizer_config.options.screenHeight = generation_options.screen_height;
    visualizer_config.options.screenWidth = generation_options.screen_width;
    visualizer_config.options.screenMSAASamples = generation_options.screen_msaa_samples;
    visualizer_config.options.screenFullscreen = generation_options.screen_fullscreen;

    visualizer_config.assets.push_back(create_cube_mesh_asset(generation_options.cube_mesh_asset_name));
    visualizer_config.assets.push_back(create_cube_texture_asset(generation_options.cube_texture_asset_name));
    visualizer_config.assets.push_back(
        create_output_cube_texture_asset(generation_options.output_cube_texture_asset_name));
    visualizer_config.assets.push_back(create_shader_asset(generation_options.cube_shader_vertex_path,
        generation_options.cube_shader_fragment_path, generation_options.cube_shader_asset_name));
    visualizer_config.assets.push_back(create_shader_asset(generation_options.view_composition_shader_vertex_path,
        generation_options.view_composition_shader_fragment_path,
        generation_options.view_composition_shader_asset_name));
    visualizer_config.assets.push_back(
        create_default_framebuffer_asset(generation_options.default_framebuffer_asset_name));

    generate_world(visualizer_config, config, generation_options);

    return visualizer_config;
}

void generate_world(
    Visconfig::Config& config, const ProcessedConfig& mdh_config, const GenerationOptions& generation_options)
{
    Visconfig::World world{};

    std::size_t num_entities{ 0 };
    world.entities.push_back(generate_coordinator_entity(num_entities++));
    generate_main_view_config(config, mdh_config, world, num_entities, generation_options);
    generate_output_view_config(config, mdh_config, world, num_entities, 1, generation_options);

    for (auto pos{ mdh_config.sub_views.begin() }; pos != mdh_config.sub_views.end(); pos++) {
        auto index{ std::distance(mdh_config.sub_views.begin(), pos) };
        generate_sub_view_config(
            config, mdh_config, world, num_entities, index, mdh_config.sub_views.size(), generation_options);
    }
    config.worlds.push_back(world);
}

void generate_main_view_config(Visconfig::Config& config, const ProcessedConfig& mdh_config, Visconfig::World& world,
    std::size_t& num_entities, const GenerationOptions& generation_options)
{
    constexpr auto render_texture_name{ "render_texture_0" };
    constexpr auto depth_buffer_name{ "renderbuffer_depth_0" };
    constexpr auto framebuffer_name{ "framebuffer_0" };

    auto render_resolution_width{ generation_options.render_resolution_multiplier * generation_options.screen_width };
    auto render_resolution_height{ generation_options.render_resolution_multiplier * generation_options.screen_height };

    config.assets.push_back(generate_render_texture_asset(render_texture_name, render_resolution_width / 2,
        render_resolution_height, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generate_renderbuffer_asset(depth_buffer_name, render_resolution_width / 2,
        render_resolution_height, 0, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generate_framebuffer_asset(framebuffer_name, 0, 0, render_resolution_width / 2, render_resolution_height,
            { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
                  render_texture_name },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depth_buffer_name } }));

    constexpr auto render_texture_multisample_name{ "render_texture_0_multisample" };
    constexpr auto depth_buffer_multisample_name{ "renderbuffer_depth_0_multisample" };
    constexpr auto framebuffer_multisample_name{ "framebuffer_0_multisample" };

    config.assets.push_back(
        generate_multisample_render_texture_asset(render_texture_multisample_name, render_resolution_width / 2,
            render_resolution_height, generation_options.screen_msaa_samples, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generate_renderbuffer_asset(depth_buffer_multisample_name, render_resolution_width / 2,
        render_resolution_height, generation_options.screen_msaa_samples,
        Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(generate_framebuffer_asset(framebuffer_multisample_name, 0, 0, render_resolution_width / 2,
        render_resolution_height,
        { { Visconfig::Assets::FramebufferType::TextureMultisample, Visconfig::Assets::FramebufferDestination::Color0,
              render_texture_multisample_name },
            { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                depth_buffer_multisample_name } }));

    auto focus_entity{ num_entities };

    auto thread_texture_border_width
        = static_cast<std::size_t>(generation_options.thread_view_texture_border_relative_width
            * std::pow(mdh_config.main_view.threads.front().absolute_scale[0]
                    * mdh_config.main_view.threads.front().absolute_scale[1]
                    * mdh_config.main_view.threads.front().absolute_scale[2],
                1.0f / 3.0f));

    thread_texture_border_width = thread_texture_border_width == 0 ? 1 : thread_texture_border_width;

    for (auto layer{ mdh_config.main_view.layers.begin() }; layer != mdh_config.main_view.layers.end(); layer++) {
        auto index{ std::distance(mdh_config.main_view.layers.begin(), layer) };

        auto main_texture_border_width = static_cast<std::size_t>(
            generation_options.main_view_texture_border_relative_width
            * std::pow(layer->absolute_scale[0] * layer->absolute_scale[1] * layer->absolute_scale[2], 1.0f / 3.0f));

        main_texture_border_width = main_texture_border_width == 0 ? 1 : main_texture_border_width;

        auto texture_front_name{ "view_0_cube_texture_" + std::to_string(index) + "_front" };
        auto texture_side_name{ "view_0_cube_texture_" + std::to_string(index) + "_side" };
        auto texture_top_name{ "view_0_cube_texture_" + std::to_string(index) + "_top" };

        auto texture_front_path{ (generation_options.working_dir
            / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/"
                + texture_front_name + ".png" })
                                     .string() };
        auto texture_side_path{ (generation_options.working_dir
            / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + texture_side_name
                + ".png" })
                                    .string() };
        auto texture_top_path{ (generation_options.working_dir
            / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + texture_top_name
                + ".png" })
                                   .string() };

        if (static_cast<std::size_t>(index) == mdh_config.main_view.layers.size() - 1) {
            generate_texture_file(texture_front_path, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[1]), 1, 1, main_texture_border_width);
            generate_texture_file(texture_side_path, static_cast<std::size_t>(layer->absolute_scale[2]),
                static_cast<std::size_t>(layer->absolute_scale[1]), 1, 1, main_texture_border_width);
            generate_texture_file(texture_top_path, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[2]), 1, 1, main_texture_border_width);
        } else {
            auto current_layer_scale{ layer->absolute_scale };
            auto next_layer_scale{ (layer + 1)->absolute_scale };

            std::array<std::size_t, 3> subdivisions{
                static_cast<std::size_t>(current_layer_scale[0] / next_layer_scale[0]),
                static_cast<std::size_t>(current_layer_scale[1] / next_layer_scale[1]),
                static_cast<std::size_t>(current_layer_scale[2] / next_layer_scale[2]),
            };

            generate_texture_file(texture_front_path, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[1]), subdivisions[0], subdivisions[1],
                main_texture_border_width);
            generate_texture_file(texture_side_path, static_cast<std::size_t>(layer->absolute_scale[2]),
                static_cast<std::size_t>(layer->absolute_scale[1]), subdivisions[2], subdivisions[1],
                main_texture_border_width);
            generate_texture_file(texture_top_path, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[2]), subdivisions[0], subdivisions[2],
                main_texture_border_width);
        }

        config.assets.push_back(create_texture_asset(texture_front_name, texture_front_path,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(create_texture_asset(texture_side_name, texture_side_path,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(create_texture_asset(texture_top_name, texture_top_path,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));

        world.entities.push_back(generate_main_view_cube(mdh_config, mdh_config.main_view, num_entities,
            num_entities - 1, index, texture_front_name, texture_side_name, texture_top_name,
            generation_options.cube_mesh_asset_name, generation_options.cube_shader_asset_name,
            generation_options.min_transparency, generation_options.max_transparency));
        num_entities++;
    }

    std::vector<std::size_t> thread_parents{ num_entities - 1 };

    for (auto layer{ mdh_config.main_view.threads.begin() }; layer != mdh_config.main_view.threads.end(); layer++) {
        std::vector<std::size_t> new_parents{};
        auto index{ std::distance(mdh_config.main_view.threads.begin(), layer) };

        auto texture_front_name{ "view_0_cube_texture_" + std::to_string(index + mdh_config.main_view.layers.size())
            + "_front" };
        auto texture_side_name{ "view_0_cube_texture_" + std::to_string(index + mdh_config.main_view.layers.size())
            + "_side" };
        auto texture_top_name{ "view_0_cube_texture_" + std::to_string(index + mdh_config.main_view.layers.size())
            + "_top" };

        auto texture_front_path{ (generation_options.working_dir
            / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/"
                + texture_front_name + ".png" })
                                     .string() };
        auto texture_side_path{ (generation_options.working_dir
            / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + texture_side_name
                + ".png" })
                                    .string() };
        auto texture_top_path{ (generation_options.working_dir
            / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + texture_top_name
                + ".png" })
                                   .string() };

        if (static_cast<std::size_t>(index) == mdh_config.main_view.threads.size() - 1) {
            generate_texture_file(texture_front_path, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[1]), 1, 1, thread_texture_border_width);
            generate_texture_file(texture_side_path, static_cast<std::size_t>(layer->absolute_scale[2]),
                static_cast<std::size_t>(layer->absolute_scale[1]), 1, 1, thread_texture_border_width);
            generate_texture_file(texture_top_path, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[2]), 1, 1, thread_texture_border_width);
        } else {
            auto current_layer_scale{ layer->absolute_scale };
            auto next_layer_scale{ (layer + 1)->absolute_scale };

            std::array<std::size_t, 3> subdivisions{
                static_cast<std::size_t>(current_layer_scale[0] / next_layer_scale[0]),
                static_cast<std::size_t>(current_layer_scale[1] / next_layer_scale[1]),
                static_cast<std::size_t>(current_layer_scale[2] / next_layer_scale[2]),
            };

            generate_texture_file(texture_front_path, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[1]), subdivisions[0], subdivisions[1],
                thread_texture_border_width);
            generate_texture_file(texture_side_path, static_cast<std::size_t>(layer->absolute_scale[2]),
                static_cast<std::size_t>(layer->absolute_scale[1]), subdivisions[2], subdivisions[1],
                thread_texture_border_width);
            generate_texture_file(texture_top_path, static_cast<std::size_t>(layer->absolute_scale[0]),
                static_cast<std::size_t>(layer->absolute_scale[2]), subdivisions[0], subdivisions[2],
                thread_texture_border_width);
        }

        config.assets.push_back(create_texture_asset(texture_front_name, texture_front_path,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(create_texture_asset(texture_side_name, texture_side_path,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(create_texture_asset(texture_top_name, texture_top_path,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));

        for (auto& parent : thread_parents) {
            for (std::size_t x{ 0 }; x < layer->num_threads[0]; x++) {
                for (std::size_t y{ 0 }; y < layer->num_threads[1]; y++) {
                    for (std::size_t z{ 0 }; z < layer->num_threads[2]; z++) {
                        world.entities.push_back(
                            generate_main_view_thread_cube(mdh_config, mdh_config.main_view, num_entities, parent,
                                index, { x, y, z }, texture_front_name, texture_side_name, texture_top_name,
                                generation_options.cube_mesh_asset_name, generation_options.cube_shader_asset_name,
                                generation_options.min_transparency, generation_options.max_transparency));
                        new_parents.push_back(num_entities++);
                    }
                }
            }
        }
        thread_parents = std::move(new_parents);
    }

    auto camera_distance{ 2.0f
        * std::max({ mdh_config.main_view.layers[0].absolute_scale[0], mdh_config.main_view.layers[0].absolute_scale[1],
            mdh_config.main_view.layers[0].absolute_scale[2] }) };

    auto camera_width{ 1.2f
        * std::max(
            { mdh_config.main_view.layers[0].absolute_scale[0], mdh_config.main_view.layers[0].absolute_scale[1] }) };
    auto camera_height{ camera_width / generation_options.camera_aspect_small };

    auto camera_entity{ num_entities++ };
    world.entities.push_back(generate_view_camera(camera_entity, focus_entity, 0, framebuffer_multisample_name,
        generation_options.camera_fov, generation_options.camera_aspect_small, generation_options.camera_near,
        generation_options.camera_far, camera_distance, camera_width, camera_height, true, false,
        mdh_config.main_view.layers[0].absolute_scale[2] != 1.0f));
    extend_copy(world, framebuffer_multisample_name, framebuffer_name,
        { Visconfig::Components::CopyOperationFlag::Color, Visconfig::Components::CopyOperationFlag::Depth },
        Visconfig::Components::CopyOperationFilter::Nearest);
    extend_composition(world, { 0.5f, 1.0f }, { -0.5f, 0.0f }, { render_texture_name },
        generation_options.default_framebuffer_asset_name, generation_options.view_composition_shader_asset_name, 0,
        false);
    extend_camera_switcher(world, camera_entity);
}

void generate_output_view_config(Visconfig::Config& config, const ProcessedConfig& mdh_config, Visconfig::World& world,
    std::size_t& num_entities, std::size_t subview, const GenerationOptions& generation_options)
{
    auto render_texture_name{ "render_texture_" + std::to_string(subview) };
    auto depth_buffer_name{ "renderbuffer_depth_" + std::to_string(subview) };
    auto framebuffer_name{ "framebuffer_" + std::to_string(subview) };

    auto render_resolution_width{ generation_options.render_resolution_multiplier * generation_options.screen_width };
    auto render_resolution_height{ generation_options.render_resolution_multiplier * generation_options.screen_height };

    config.assets.push_back(generate_render_texture_asset(render_texture_name, render_resolution_width / 2,
        render_resolution_height, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generate_renderbuffer_asset(depth_buffer_name, render_resolution_width / 2,
        render_resolution_height, 0, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generate_framebuffer_asset(framebuffer_name, 0, 0, render_resolution_width / 2, render_resolution_height,
            { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
                  render_texture_name },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depth_buffer_name } }));

    auto render_texture_multisample_name{ render_texture_name + "_multisample" };
    auto depth_buffer_multisample_name{ depth_buffer_name + "_multisample" };
    auto framebuffer_multisample_name{ framebuffer_name + "_multisample" };

    config.assets.push_back(
        generate_multisample_render_texture_asset(render_texture_multisample_name, render_resolution_width / 2,
            render_resolution_height, generation_options.screen_msaa_samples, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generate_renderbuffer_asset(depth_buffer_multisample_name, render_resolution_width / 2,
        render_resolution_height, generation_options.screen_msaa_samples,
        Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(generate_framebuffer_asset(framebuffer_multisample_name, 0, 0, render_resolution_width / 2,
        render_resolution_height,
        { { Visconfig::Assets::FramebufferType::TextureMultisample, Visconfig::Assets::FramebufferDestination::Color0,
              render_texture_multisample_name },
            { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                depth_buffer_multisample_name } }));

    auto texture_border_width = static_cast<std::size_t>(generation_options.main_view_texture_border_relative_width
        * std::pow(mdh_config.main_view.layers[0].absolute_scale[0] * mdh_config.main_view.layers[0].absolute_scale[1]
                * mdh_config.main_view.layers[0].absolute_scale[2],
            1.0f / 3.0f));

    texture_border_width = texture_border_width == 0 ? 1 : texture_border_width;

    auto texture_front_name{ "view_" + std::to_string(subview) + "_cube_texture_0_front" };
    auto texture_side_name{ "view_" + std::to_string(subview) + "_cube_texture_0_side" };
    auto texture_top_name{ "view_" + std::to_string(subview) + "_cube_texture_0_top" };
    auto inner_layer_texture_name{ "view_" + std::to_string(subview) + "_cube_texture_1" };

    auto texture_front_path{ (generation_options.working_dir
        / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + texture_front_name
            + ".png" })
                                 .string() };
    auto texture_side_path{ (generation_options.working_dir
        / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + texture_side_name
            + ".png" })
                                .string() };
    auto texture_top_path{ (generation_options.working_dir
        / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + texture_top_name
            + ".png" })
                               .string() };
    auto inner_layer_texture_path{ (generation_options.working_dir
        / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/"
            + inner_layer_texture_name + ".png" })
                                       .string() };

    generate_texture_file(texture_front_path, static_cast<std::size_t>(mdh_config.output_view.size[0]),
        static_cast<std::size_t>(mdh_config.output_view.size[1]), 1, 1, texture_border_width);
    generate_texture_file(texture_side_path, static_cast<std::size_t>(mdh_config.output_view.size[2]),
        static_cast<std::size_t>(mdh_config.output_view.size[1]), 1, 1, texture_border_width);
    generate_texture_file(texture_top_path, static_cast<std::size_t>(mdh_config.output_view.size[0]),
        static_cast<std::size_t>(mdh_config.output_view.size[2]), 1, 1, texture_border_width);
    generate_texture_file(inner_layer_texture_path, static_cast<std::size_t>(mdh_config.output_view.size[0]),
        static_cast<std::size_t>(mdh_config.output_view.size[1]), 1, 1, 0);

    config.assets.push_back(create_texture_asset(texture_front_name, texture_front_path,
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
    config.assets.push_back(create_texture_asset(texture_side_name, texture_side_path,
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
    config.assets.push_back(create_texture_asset(texture_top_name, texture_top_path,
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
    config.assets.push_back(create_texture_asset(inner_layer_texture_name, inner_layer_texture_path,
        { Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps }));

    auto focus_entity{ num_entities };
    world.entities.push_back(generate_cube({ 0, 0, 0 }, mdh_config.output_view.size, { 0.0f, 0.0f, 0.0f, 0.0f },
        num_entities++, 0, 1llu << subview, false, texture_front_name, texture_side_name, texture_top_name,
        generation_options.cube_mesh_asset_name, generation_options.cube_shader_asset_name));

    auto parent{ focus_entity };
    for (auto layer{ mdh_config.output_view.layers.begin() }; layer != mdh_config.output_view.layers.end(); layer++) {
        auto index{ std::distance(mdh_config.output_view.layers.begin(), layer) };

        auto mesh_name{ "view_" + std::to_string(subview) + "_mesh_" + std::to_string(index) };
        config.assets.push_back(create_simple_cube_mesh_asset(mesh_name));

        auto new_parent{ num_entities };
        world.entities.push_back(generate_output_view_cube(world, mdh_config, mdh_config.output_view, num_entities,
            parent, subview, index, inner_layer_texture_name, mesh_name, generation_options.cube_shader_asset_name,
            generation_options.min_transparency, generation_options.max_transparency));
        parent = new_parent;
        num_entities++;
    }

    auto camera_distance{ 2.0f
        * std::max(
            { mdh_config.output_view.size[0], mdh_config.output_view.size[1], mdh_config.output_view.size[2] }) };

    auto camera_width{ 1.2f * std::max({ mdh_config.output_view.size[0], mdh_config.output_view.size[1] }) };
    auto camera_height{ camera_width / generation_options.camera_aspect_small };

    auto camera_entity{ num_entities++ };
    world.entities.push_back(generate_view_camera(camera_entity, focus_entity, subview, framebuffer_multisample_name,
        generation_options.camera_fov, generation_options.camera_aspect_small, generation_options.camera_near,
        generation_options.camera_far, camera_distance, camera_width, camera_height, false, false,
        mdh_config.output_view.size[2] != 1.0f));
    extend_copy(world, framebuffer_multisample_name, framebuffer_name,
        { Visconfig::Components::CopyOperationFlag::Color, Visconfig::Components::CopyOperationFlag::Depth },
        Visconfig::Components::CopyOperationFilter::Nearest);
    extend_composition(world, { 0.5f, 1.0f }, { 0.5f, 0.0f }, { render_texture_name },
        generation_options.default_framebuffer_asset_name, generation_options.view_composition_shader_asset_name,
        subview, false);
    extend_camera_switcher(world, camera_entity);
}

void generate_sub_view_config(Visconfig::Config& config, const ProcessedConfig& mdh_config, Visconfig::World& world,
    std::size_t& num_entities, std::size_t subview, std::size_t num_sub_views,
    const GenerationOptions& generation_options)
{
    auto renderTextureName{ "render_texture_" + std::to_string(subview + 2) };
    auto depthBufferName{ "renderbuffer_depth_" + std::to_string(subview + 2) };
    auto framebufferName{ "framebuffer_" + std::to_string(subview + 2) };

    auto render_resolution_width{ generation_options.render_resolution_multiplier * generation_options.screen_width };
    auto render_resolution_height{ generation_options.render_resolution_multiplier * generation_options.screen_height };

    config.assets.push_back(generate_render_texture_asset(
        renderTextureName, render_resolution_width, render_resolution_height, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(generate_renderbuffer_asset(depthBufferName, render_resolution_width,
        render_resolution_height, 0, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generate_framebuffer_asset(framebufferName, 0, 0, render_resolution_width, render_resolution_height,
            { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
                  renderTextureName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferName } }));

    auto renderTextureMultisampleName{ renderTextureName + "_multisample" };
    auto depthBufferMultisampleName{ depthBufferName + "_multisample" };
    auto framebufferMultisampleName{ framebufferName + "_multisample" };

    config.assets.push_back(
        generate_multisample_render_texture_asset(renderTextureMultisampleName, render_resolution_width,
            render_resolution_height, generation_options.screen_msaa_samples, Visconfig::Assets::TextureFormat::RGBA));
    config.assets.push_back(
        generate_renderbuffer_asset(depthBufferMultisampleName, render_resolution_width, render_resolution_height,
            generation_options.screen_msaa_samples, Visconfig::Assets::RenderbufferFormat::Depth24));
    config.assets.push_back(
        generate_framebuffer_asset(framebufferMultisampleName, 0, 0, render_resolution_width, render_resolution_height,
            { { Visconfig::Assets::FramebufferType::TextureMultisample,
                  Visconfig::Assets::FramebufferDestination::Color0, renderTextureMultisampleName },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depthBufferMultisampleName } }));

    auto textureBorderWidth = static_cast<std::size_t>(generation_options.sub_view_texture_border_relative_width
        * std::pow(mdh_config.sub_views[subview].layers.front().absolute_scale[0]
                * mdh_config.sub_views[subview].layers.front().absolute_scale[1]
                * mdh_config.sub_views[subview].layers.front().absolute_scale[2],
            1.0f / 3.0f));

    textureBorderWidth = textureBorderWidth == 0 ? 1 : textureBorderWidth;

    auto focusEntity{ num_entities };
    for (auto layer{ mdh_config.config.begin() }; layer != mdh_config.config.end(); layer++) {
        auto index{ std::distance(mdh_config.config.begin(), layer) };

        auto textureFrontName{ "view_" + std::to_string(subview + 2) + "_cube_texture_" + std::to_string(index)
            + "_front" };
        auto textureSideName{ "view_" + std::to_string(subview + 2) + "_cube_texture_" + std::to_string(index)
            + "_side" };
        auto textureTopName{ "view_" + std::to_string(subview + 2) + "_cube_texture_" + std::to_string(index)
            + "_top" };

        auto textureFrontPath{ (generation_options.working_dir
            / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + textureFrontName
                + ".png" })
                                   .string() };
        auto textureSidePath{ (generation_options.working_dir
            / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + textureSideName
                + ".png" })
                                  .string() };
        auto textureTopPath{ (generation_options.working_dir
            / std::filesystem::path{ generation_options.assets_texture_directory_path.string() + "/" + textureTopName
                + ".png" })
                                 .string() };

        generate_texture_file(textureFrontPath,
            static_cast<std::size_t>(mdh_config.sub_views[subview].layers[index].absolute_scale[0]),
            static_cast<std::size_t>(mdh_config.sub_views[subview].layers[index].absolute_scale[1]), 1, 1,
            textureBorderWidth);
        generate_texture_file(textureSidePath,
            static_cast<std::size_t>(mdh_config.sub_views[subview].layers[index].absolute_scale[2]),
            static_cast<std::size_t>(mdh_config.sub_views[subview].layers[index].absolute_scale[1]), 1, 1,
            textureBorderWidth);
        generate_texture_file(textureTopPath,
            static_cast<std::size_t>(mdh_config.sub_views[subview].layers[index].absolute_scale[0]),
            static_cast<std::size_t>(mdh_config.sub_views[subview].layers[index].absolute_scale[2]), 1, 1,
            textureBorderWidth);

        config.assets.push_back(create_texture_asset(textureFrontName, textureFrontPath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(create_texture_asset(textureSideName, textureSidePath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));
        config.assets.push_back(create_texture_asset(textureTopName, textureTopPath,
            { Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps }));

        world.entities.push_back(generate_sub_view_cube(mdh_config, mdh_config.sub_views[subview], num_entities,
            num_entities - 1, subview, index, textureFrontName, textureSideName, textureTopName,
            generation_options.cube_mesh_asset_name, generation_options.cube_shader_asset_name,
            generation_options.min_transparency, generation_options.max_transparency));
        num_entities++;
    }

    auto cameraDistance{ 1.5f
        * std::max({ mdh_config.sub_views[subview].layers[0].absolute_scale[0],
            mdh_config.sub_views[subview].layers[0].absolute_scale[1],
            mdh_config.sub_views[subview].layers[0].absolute_scale[2] }) };

    auto cameraWidth{ 1.2f
        * std::max({ mdh_config.sub_views[subview].layers[0].absolute_scale[0],
            mdh_config.sub_views[subview].layers[0].absolute_scale[1] }) };
    auto cameraHeight{ cameraWidth / generation_options.camera_aspect };

    auto cameraEntity{ num_entities++ };
    world.entities.push_back(generate_view_camera(cameraEntity, focusEntity, subview + 2, framebufferMultisampleName,
        generation_options.camera_fov, generation_options.camera_aspect, generation_options.camera_near,
        generation_options.camera_far, cameraDistance, cameraWidth, cameraHeight, false, true,
        mdh_config.sub_views[subview].layers[0].absolute_scale[2] != 1.0f));
    extend_copy(world, framebufferMultisampleName, framebufferName,
        { Visconfig::Components::CopyOperationFlag::Color, Visconfig::Components::CopyOperationFlag::Depth },
        Visconfig::Components::CopyOperationFilter::Nearest);
    extend_composition(world, { 0.2f, 0.2f }, { 0.7f, (-0.7f + ((num_sub_views - 1) * 0.5f)) - (subview * 0.5f) },
        { renderTextureName }, generation_options.default_framebuffer_asset_name,
        generation_options.view_composition_shader_asset_name, subview + 2, true);
    extend_camera_switcher(world, cameraEntity);
}

}