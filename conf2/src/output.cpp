#include "output.hpp"

#include <cmath>
#include <filesystem>
#include <map>

#include "Config.hpp"
#include "asset_utilities.hpp"
#include "entity_utilities.hpp"

namespace Config {

Visconfig::World generate_world(const ConfigCommandList& config_command_list,
    const GenerationOptions& generation_options, std::vector<Visconfig::Asset>& assets);
std::vector<std::size_t> populate_view(Visconfig::World& world, const ViewCommandList& view_commands,
    const GenerationOptions& generation_options, std::vector<Visconfig::Asset>& assets, std::size_t view_idx);

void generate_assets_directory(const GenerationOptions& generation_options)
{
    if (std::filesystem::exists(generation_options.working_directory / generation_options.assets_directory_path)) {
        std::filesystem::remove_all(generation_options.working_directory / generation_options.assets_directory_path);
    }

    std::filesystem::create_directories(
        generation_options.working_directory / generation_options.assets_directory_path);
    std::filesystem::create_directory(
        generation_options.working_directory / generation_options.assets_texture_directory_path);
}

Visconfig::Config generate_config(
    const ConfigCommandList& config_command_list, const GenerationOptions& generation_options)
{
    generate_assets_directory(generation_options);

    Visconfig::Config config{};

    config.options.screenHeight = generation_options.screen_height;
    config.options.screenWidth = generation_options.screen_width;
    config.options.screenMSAASamples = generation_options.screen_msaa_samples;
    config.options.screenFullscreen = generation_options.screen_fullscreen;

    config.assets.push_back(create_cuboid_mesh_asset(generation_options.cuboid_mesh_asset_name));
    config.assets.push_back(create_default_framebuffer_asset(generation_options.default_framebuffer_asset_name));
    config.assets.push_back(create_fullscreen_quad_mesh_asset(generation_options.fullscreen_quad_mesh_asset_name));
    config.assets.push_back(create_shader_asset(generation_options.cuboid_diffuse_shader_asset_name,
        generation_options.cuboid_diffuse_shader_vertex_path, generation_options.cuboid_diffuse_shader_fragment_path));
    config.assets.push_back(create_shader_asset(generation_options.cuboid_oit_shader_asset_name,
        generation_options.cuboid_oit_shader_vertex_path, generation_options.cuboid_oit_shader_fragment_path));
    config.assets.push_back(create_shader_asset(generation_options.cuboid_oit_blend_shader_asset_name,
        generation_options.cuboid_oit_blend_shader_vertex_path,
        generation_options.cuboid_oit_blend_shader_fragment_path));
    config.assets.push_back(create_shader_asset(generation_options.view_composition_shader_asset_name,
        generation_options.view_composition_shader_vertex_path,
        generation_options.view_composition_shader_fragment_path));

    config.worlds.push_back(generate_world(config_command_list, generation_options, config.assets));

    return config;
}

Visconfig::World generate_world(const ConfigCommandList& config_command_list,
    const GenerationOptions& generation_options, std::vector<Visconfig::Asset>& assets)
{
    Visconfig::World world{};

    world.entities.push_back(generate_coordinator_entity(0));

    std::map<std::string, std::vector<std::size_t>> view_entity_map{};
    for (std::size_t i = 0; i < config_command_list.view_commands.size(); ++i) {
        view_entity_map[config_command_list.view_commands[i].view_name]
            = populate_view(world, config_command_list.view_commands[i], generation_options, assets, i);
    }

    auto legend_entries = ConfigContainer::get_instance().legend_entries();
    for (auto& legend_entry : legend_entries) {
        if (std::holds_alternative<ColorLegend>(legend_entry)) {
            auto& color = std::get<ColorLegend>(legend_entry);
            auto entity = view_entity_map[color.view_name()][color.cuboid_idx()];
            add_color_legend(
                world.entities.front(), color.label(), color.description(), "active_fill_color", entity, 1);
        } else if (std::holds_alternative<ImageLegend>(legend_entry)) {
            auto& image = std::get<ImageLegend>(legend_entry);
            std::vector<Visconfig::Assets::TextureAttributes> texture_attributes = {
                Visconfig::Assets::TextureAttributes::MagnificationLinear,
                Visconfig::Assets::TextureAttributes::MinificationLinear,
                Visconfig::Assets::TextureAttributes::GenerateMipMaps,
            };

            auto texture_path = generation_options.resource_directory / image.image_path();
            auto asset_texture_path = generation_options.assets_texture_directory_path / image.image_path();
            if (!std::filesystem::exists(texture_path)) {
                std::cerr << "Could not find asset " << texture_path << std::endl;
                std::abort();
            } else {
                std::filesystem::copy(texture_path, asset_texture_path);
            }

            assets.push_back(create_texture_asset(image.image_name(), asset_texture_path, texture_attributes));
            add_image_legend(
                world.entities.front(), image.image_name(), image.description(), image.scaling(), image.absolute());
        }
    }

    auto group_connections = ConfigContainer::get_instance().get_group_connections();
    for (auto& connection : group_connections) {
        add_composition_gui_connection(world.entities.front(), connection[0], connection[1]);
    }

    return world;
}

std::vector<std::size_t> populate_view(Visconfig::World& world, const ViewCommandList& view_commands,
    const GenerationOptions& generation_options, std::vector<Visconfig::Asset>& assets, std::size_t view_idx)
{
    auto render_texture_name = "render_texture_" + std::to_string(view_idx);
    auto depth_buffer_name = "renderbuffer_depth_" + std::to_string(view_idx);
    auto accumulation_texture_name = "accumulation_texture" + std::to_string(view_idx);
    auto revealage_texture_name = "revealage_texture" + std::to_string(view_idx);
    auto framebuffer_name = "framebuffer_" + std::to_string(view_idx);
    auto oit_framebuffer_name = "oit_" + framebuffer_name;

    auto render_resolution_width = static_cast<std::size_t>(
        generation_options.render_resolution_multiplier * generation_options.screen_width * view_commands.size);
    auto render_resolution_height = static_cast<std::size_t>(
        generation_options.render_resolution_multiplier * generation_options.screen_height * view_commands.size);

    assets.push_back(create_render_texture_asset(render_texture_name, render_resolution_width, render_resolution_height,
        Visconfig::Assets::TextureFormat::RGBA));
    assets.push_back(create_renderbuffer_asset(depth_buffer_name, render_resolution_width, render_resolution_height, 0,
        Visconfig::Assets::RenderbufferFormat::Depth24));
    assets.push_back(create_framebuffer_asset(framebuffer_name, 0, 0, render_resolution_width, render_resolution_height,
        { { Visconfig::Assets::FramebufferType::Texture, Visconfig::Assets::FramebufferDestination::Color0,
              render_texture_name },
            { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                depth_buffer_name } }));

    auto render_texture_multisample_name = render_texture_name + "_multisample";
    auto depth_buffer_multisample_name = depth_buffer_name + "_multisample";
    auto accumulation_multisample_texture_name = accumulation_texture_name + "_multisample";
    auto revealage_multisample_texture_name = revealage_texture_name + "_multisample";
    auto framebuffer_multisample_name = framebuffer_name + "_multisample";
    auto oit_framebuffer_multisample_name = oit_framebuffer_name + "_multisample";

    assets.push_back(create_multisample_render_texture_asset(render_texture_multisample_name, render_resolution_width,
        render_resolution_height, generation_options.screen_msaa_samples, Visconfig::Assets::TextureFormat::RGBA));
    assets.push_back(
        create_renderbuffer_asset(depth_buffer_multisample_name, render_resolution_width, render_resolution_height,
            generation_options.screen_msaa_samples, Visconfig::Assets::RenderbufferFormat::Depth24));
    assets.push_back(create_multisample_render_texture_asset(accumulation_multisample_texture_name,
        render_resolution_width, render_resolution_height, generation_options.screen_msaa_samples,
        Visconfig::Assets::TextureFormat::RGBA16F));
    assets.push_back(
        create_multisample_render_texture_asset(revealage_multisample_texture_name, render_resolution_width,
            render_resolution_height, generation_options.screen_msaa_samples, Visconfig::Assets::TextureFormat::R8));
    assets.push_back(
        create_framebuffer_asset(framebuffer_multisample_name, 0, 0, render_resolution_width, render_resolution_height,
            { { Visconfig::Assets::FramebufferType::TextureMultisample,
                  Visconfig::Assets::FramebufferDestination::Color0, render_texture_multisample_name },
                { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                    depth_buffer_multisample_name } }));
    assets.push_back(create_framebuffer_asset(oit_framebuffer_multisample_name, 0, 0, render_resolution_width,
        render_resolution_height,
        { { Visconfig::Assets::FramebufferType::TextureMultisample, Visconfig::Assets::FramebufferDestination::Color0,
              accumulation_multisample_texture_name },
            { Visconfig::Assets::FramebufferType::TextureMultisample, Visconfig::Assets::FramebufferDestination::Color1,
                revealage_multisample_texture_name },
            { Visconfig::Assets::FramebufferType::TextureMultisample, Visconfig::Assets::FramebufferDestination::Color2,
                render_texture_multisample_name },
            { Visconfig::Assets::FramebufferType::Renderbuffer, Visconfig::Assets::FramebufferDestination::Depth,
                depth_buffer_multisample_name } }));

    auto max_cuboid_size = view_commands.cuboids.front().positions.front().size;
    auto texture_border_width = static_cast<std::size_t>(generation_options.cuboid_texture_border_relative_width
        * std::pow(max_cuboid_size[0] * max_cuboid_size[1] * max_cuboid_size[2], 1.0f / 3.0f));
    texture_border_width = texture_border_width == 0 ? 1 : texture_border_width;

    std::vector<std::size_t> generated_entities{};
    auto focus_entity_id = world.entities.size();
    for (auto cuboid = view_commands.cuboids.begin(); cuboid != view_commands.cuboids.end(); ++cuboid) {
        auto index = std::distance(view_commands.cuboids.begin(), cuboid);

        auto front_texture_name = "entity_" + std::to_string(focus_entity_id + index) + "_texture_front";
        auto side_texture_name = "entity_" + std::to_string(focus_entity_id + index) + "_texture_side";
        auto top_texture_name = "entity_" + std::to_string(focus_entity_id + index) + "_texture_top";

        auto front_texture_relative_path = generation_options.assets_texture_directory_path / front_texture_name;
        auto side_texture_relative_path = generation_options.assets_texture_directory_path / side_texture_name;
        auto top_texture_relative_path = generation_options.assets_texture_directory_path / top_texture_name;

        front_texture_relative_path.replace_extension(".png");
        side_texture_relative_path.replace_extension(".png");
        top_texture_relative_path.replace_extension(".png");

        auto front_texture_path = generation_options.working_directory / front_texture_relative_path;
        auto side_texture_path = generation_options.working_directory / side_texture_relative_path;
        auto top_texture_path = generation_options.working_directory / top_texture_relative_path;

        auto cuboid_size = [&]() -> auto
        {
            if (cuboid->commands[0].type == CuboidCommandType::DRAW) {
                auto sub_cuboid_idx = std::get<DrawCommand>(cuboid->commands[0].command).cuboid_idx;
                return cuboid->positions[sub_cuboid_idx].size;
            } else {
                return max_cuboid_size;
            }
        }
        ();
        generate_texture_file(front_texture_path, cuboid_size[0], cuboid_size[1], 1, 1, texture_border_width);
        generate_texture_file(side_texture_path, cuboid_size[2], cuboid_size[1], 1, 1, texture_border_width);
        generate_texture_file(top_texture_path, cuboid_size[0], cuboid_size[2], 1, 1, texture_border_width);

        std::vector<Visconfig::Assets::TextureAttributes> texture_attributes = {
            Visconfig::Assets::TextureAttributes::MagnificationLinear,
            Visconfig::Assets::TextureAttributes::MinificationLinear,
            Visconfig::Assets::TextureAttributes::GenerateMipMaps,
        };

        assets.push_back(create_texture_asset(front_texture_name, front_texture_relative_path, texture_attributes));
        assets.push_back(create_texture_asset(side_texture_name, side_texture_relative_path, texture_attributes));
        assets.push_back(create_texture_asset(top_texture_name, top_texture_relative_path, texture_attributes));

        auto entity = generate_cuboid(world.entities.size(), view_idx, index == 0, *cuboid, front_texture_name,
            side_texture_name, top_texture_name, accumulation_multisample_texture_name,
            revealage_multisample_texture_name, generation_options.cuboid_mesh_asset_name,
            generation_options.cuboid_pipeline_name,
            {
                generation_options.cuboid_diffuse_shader_asset_name,
                generation_options.cuboid_oit_shader_asset_name,
                generation_options.cuboid_oit_blend_shader_asset_name,
            },
            max_cuboid_size);

        generated_entities.push_back(entity.id);
        world.entities.push_back(std::move(entity));
    }

    auto camera_distance = 1.2f
        * std::max({ max_cuboid_size[0] / 2.0f, max_cuboid_size[1] / 2.0f, max_cuboid_size[2] / 2.0f })
        / std::tan(generation_options.camera_fov);

    auto camera_width = 1.2f * std::max({ max_cuboid_size[0], max_cuboid_size[1], max_cuboid_size[2] });
    auto camera_height = camera_width / generation_options.camera_aspect;

    auto camera_fixed = view_idx != 0;
    auto camera_perspective = max_cuboid_size[2] != 0 && max_cuboid_size[2] != 1;
    auto camera_active = view_idx == 0;
    std::map<std::string, std::vector<std::string>> camera_targets = {
        { generation_options.cuboid_pipeline_name,
            {
                framebuffer_multisample_name,
                oit_framebuffer_multisample_name,
                framebuffer_multisample_name,
                framebuffer_name,
            } },
    };

    auto camera_entity_id = world.entities.size();
    world.entities.push_back(generate_view_camera(camera_entity_id, focus_entity_id, view_idx,
        generation_options.camera_fov, generation_options.camera_aspect, generation_options.camera_near,
        generation_options.camera_far, camera_distance, camera_width, camera_height, camera_targets, camera_fixed,
        camera_perspective, camera_active));

    auto& coordinator_entity = world.entities.front();
    std::vector<std::string> composition_src = { render_texture_name };

    auto size = view_commands.size;
    extend_camera_switcher(coordinator_entity, camera_entity_id);
    auto& group_name = ConfigContainer::get_instance().get_group_association(view_commands.view_name);
    add_composition_gui_window(coordinator_entity, group_name, view_commands.view_name, render_texture_name,
        { size, size }, view_commands.position);

    return generated_entities;
}

}
