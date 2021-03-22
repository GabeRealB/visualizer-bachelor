#include "entity_utilities.hpp"

#include "Config.hpp"

namespace Config {

constexpr std::size_t coordinator_camera_switcher_idx = 0;
constexpr std::size_t coordinator_canvas_idx = 1;

constexpr std::size_t canvas_composition_gui_idx = 0;
constexpr std::size_t canvas_legend_gui_idx = 1;
constexpr std::size_t config_dump_gui_idx = 2;

Visconfig::Entity generate_coordinator_entity(std::size_t entity_id)
{
    Visconfig::Entity entity{};
    entity.id = entity_id;

    entity.components.push_back({
        Visconfig::Components::ComponentType::CameraSwitcher,
        std::make_shared<Visconfig::Components::CameraSwitcherComponent>(),
    });
    entity.components.push_back({
        Visconfig::Components::ComponentType::Canvas,
        std::make_shared<Visconfig::Components::CanvasComponent>(),
    });

    auto canvas = std::static_pointer_cast<Visconfig::Components::CanvasComponent>(
        entity.components[coordinator_canvas_idx].data);
    canvas->entries.push_back({
        Visconfig::Components::CanvasEntryType::CompositionGUI,
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },
        Visconfig::Components::CompositionGUI{},
    });
    canvas->entries.push_back({
        Visconfig::Components::CanvasEntryType::LegendGUI,
        { 0.2f, 0.2f },
        { 0.0f, 0.0f },
        Visconfig::Components::LegendGUI{},
    });
    canvas->entries.push_back({
        Visconfig::Components::CanvasEntryType::ConfigDumpGUI,
        { 0.2f, 0.2f },
        { 0.0f, 0.0f },
        Visconfig::Components::ConfigDumpGUI{},
    });

    auto& background_color = ConfigContainer::get_instance().background_color();
    auto& composition_gui
        = std::get<Visconfig::Components::CompositionGUI>(canvas->entries[canvas_composition_gui_idx].gui_data);
    composition_gui.background_color = {
        background_color[0] / 255.0f,
        background_color[1] / 255.0f,
        background_color[2] / 255.0f,
        background_color[3] / 255.0f,
    };

    return entity;
}

Visconfig::Entity generate_view_camera(std::size_t entity_id, std::size_t focus_entity_id, std::size_t view_idx,
    float fov, float aspect, float near, float far, float distance, float orthographic_width, float orthographic_height,
    const std::map<std::string, std::vector<std::string>>& targets, bool fixed, bool perspective, bool active)
{
    Visconfig::Entity entity{};
    entity.id = entity_id;

    entity.components.push_back(
        { Visconfig::Components::ComponentType::Camera, std::make_shared<Visconfig::Components::CameraComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::FreeFlyCamera,
        std::make_shared<Visconfig::Components::FreeFlyCameraComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::FixedCamera,
        std::make_shared<Visconfig::Components::FixedCameraComponent>() });

    auto camera{ std::static_pointer_cast<Visconfig::Components::CameraComponent>(entity.components[0].data) };
    camera->active = active;
    camera->fixed = fixed;
    camera->perspective = perspective;
    camera->fov = fov;
    camera->aspect = aspect;
    camera->near = near;
    camera->far = far;
    camera->orthographicWidth = orthographic_width;
    camera->orthographicHeight = orthographic_height;
    camera->layerMask = 1ull << view_idx;

    for (auto& target : targets) {
        camera->targets.insert_or_assign(target.first, target.second);
    }

    auto transform{ std::static_pointer_cast<Visconfig::Components::TransformComponent>(entity.components[1].data) };
    transform->rotation[0] = 0.0f;
    transform->rotation[1] = 0.0f;
    transform->rotation[2] = 0.0f;

    transform->position[0] = 0.0f;
    transform->position[1] = 0.0f;
    transform->position[2] = distance;

    transform->scale[0] = 1.0f;
    transform->scale[1] = 1.0f;
    transform->scale[2] = 1.0f;

    auto fixed_camera{ std::static_pointer_cast<Visconfig::Components::FixedCameraComponent>(
        entity.components[3].data) };
    fixed_camera->focus = focus_entity_id;
    fixed_camera->distance = distance;
    fixed_camera->horizontalAngle = 0.0f;
    fixed_camera->verticalAngle = 0.0f;

    return entity;
}

Visconfig::Entity generate_cuboid(std::size_t entity_id, std::size_t view_idx, bool global,
    const CuboidCommandList& command_list, const std::string& accum_texture, const std::string& revealage_texture,
    const std::string& mesh_name, const std::string& pipeline_name, const std::vector<std::string>& shader_asset_names,
    const std::array<int, 3>& global_size, const std::array<int, 3>& size, float line_width)
{
    Visconfig::Entity entity{};
    entity.id = entity_id;

    entity.components.push_back(
        { Visconfig::Components::ComponentType::Cube, std::make_shared<Visconfig::Components::CubeComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Mesh, std::make_shared<Visconfig::Components::MeshComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Material,
        std::make_shared<Visconfig::Components::MaterialComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Layer, std::make_shared<Visconfig::Components::LayerComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::CuboidCommandList,
        std::make_shared<Visconfig::Components::CuboidCommandListComponent>() });

    [[maybe_unused]] constexpr auto cube_index{ 0 };
    constexpr auto mesh_index{ 1 };
    constexpr auto material_index{ 2 };
    constexpr auto layer_index{ 3 };
    constexpr auto transform_index{ 4 };
    constexpr auto cuboid_command_list_index{ 5 };

    constexpr unsigned int HEATMAP_COUNTER_BITS = 16;
    constexpr unsigned int HEATMAP_COUNTER_MAX = (1u << HEATMAP_COUNTER_BITS) - 1;
    std::size_t heatmap_stepping
        = command_list.max_accesses <= HEATMAP_COUNTER_MAX ? 1 : command_list.max_accesses / HEATMAP_COUNTER_MAX;
    unsigned int heatmap_max
        = std::min(static_cast<unsigned int>(command_list.max_accesses / heatmap_stepping), HEATMAP_COUNTER_MAX);

    auto& mesh{ *std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[mesh_index].data) };
    auto& material{ *std::static_pointer_cast<Visconfig::Components::MaterialComponent>(
        entity.components[material_index].data) };
    auto& layer{ *std::static_pointer_cast<Visconfig::Components::LayerComponent>(
        entity.components[layer_index].data) };
    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transform_index].data) };
    auto& cuboid_command_list{ *std::static_pointer_cast<Visconfig::Components::CuboidCommandListComponent>(
        entity.components[cuboid_command_list_index].data) };

    mesh.asset = mesh_name;

    auto accum_texture_attribute{ std::make_shared<Visconfig::Components::Sampler2DMSMaterialAttribute>() };
    auto revealage_texture_attribute{ std::make_shared<Visconfig::Components::Sampler2DMSMaterialAttribute>() };
    auto active_fill_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto inactive_fill_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto active_border_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto inactive_border_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto oob_active_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto oob_inactive_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    auto max_access_count_attribute{ std::make_shared<Visconfig::Components::UIntMaterialAttribute>() };
    auto heatmap_color_count_attribute{ std::make_shared<Visconfig::Components::UIntMaterialAttribute>() };
    auto heatmap_color_start_attribute{ std::make_shared<Visconfig::Components::FloatArrayMaterialAttribute>() };
    auto heatmap_fill_colors_attribute{ std::make_shared<Visconfig::Components::Vec4ArrayMaterialAttribute>() };

    auto line_width_attribute{ std::make_shared<Visconfig::Components::FloatMaterialAttribute>() };
    auto cuboid_size_attribute{ std::make_shared<Visconfig::Components::Vec3MaterialAttribute>() };

    line_width_attribute->value = line_width;

    cuboid_size_attribute->value = {
        static_cast<float>(size[0]),
        static_cast<float>(size[1]),
        static_cast<float>(size[2]),
    };

    accum_texture_attribute->asset = accum_texture;
    accum_texture_attribute->slot = 0;

    revealage_texture_attribute->asset = revealage_texture;
    revealage_texture_attribute->slot = 1;

    active_fill_color_attribute->value = {
        command_list.active_fill_color[0] / 255.0f,
        command_list.active_fill_color[1] / 255.0f,
        command_list.active_fill_color[2] / 255.0f,
        command_list.active_fill_color[3] / 255.0f,
    };
    inactive_fill_color_attribute->value = {
        command_list.inactive_fill_color[0] / 255.0f,
        command_list.inactive_fill_color[1] / 255.0f,
        command_list.inactive_fill_color[2] / 255.0f,
        command_list.inactive_fill_color[3] / 255.0f,
    };
    active_border_color_attribute->value = {
        command_list.active_border_color[0] / 255.0f,
        command_list.active_border_color[1] / 255.0f,
        command_list.active_border_color[2] / 255.0f,
        command_list.active_border_color[3] / 255.0f,
    };
    inactive_border_color_attribute->value = {
        command_list.inactive_border_color[0] / 255.0f,
        command_list.inactive_border_color[1] / 255.0f,
        command_list.inactive_border_color[2] / 255.0f,
        command_list.inactive_border_color[3] / 255.0f,
    };
    oob_active_color_attribute->value = {
        command_list.oob_active_color[0] / 255.0f,
        command_list.oob_active_color[1] / 255.0f,
        command_list.oob_active_color[2] / 255.0f,
        command_list.oob_active_color[3] / 255.0f,
    };
    oob_inactive_color_attribute->value = {
        command_list.oob_inactive_color[0] / 255.0f,
        command_list.oob_inactive_color[1] / 255.0f,
        command_list.oob_inactive_color[2] / 255.0f,
        command_list.oob_inactive_color[3] / 255.0f,
    };

    max_access_count_attribute->value = heatmap_max;
    heatmap_color_count_attribute->value = 0;
    heatmap_color_start_attribute->value = std::vector(10, 0.0f);
    heatmap_fill_colors_attribute->value = { 10, { 0.0f, 0.0f, 0.0f, 0.0f } };

    if (command_list.heatmap.has_value()) {
        auto& heatmap = command_list.heatmap.value();
        heatmap_color_count_attribute->value = static_cast<std::uint32_t>(heatmap.colors.size());

        for (std::size_t i = 0; i < heatmap.colors.size(); i++) {
            heatmap_color_start_attribute->value[i] = heatmap.colors_start[i];
            heatmap_fill_colors_attribute->value[i] = {
                heatmap.colors[i][0] / 255.0f,
                heatmap.colors[i][1] / 255.0f,
                heatmap.colors[i][2] / 255.0f,
                heatmap.colors[i][3] / 255.0f,
            };
        }
    }

    std::vector<Visconfig::Components::MaterialPass> material_passes{};

    // diffuse pass
    material_passes.push_back(Visconfig::Components::MaterialPass{ shader_asset_names[0],
        {
            {
                "line_width",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Float, line_width_attribute, false },
            },
            {
                "cuboid_size",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Vec3, cuboid_size_attribute, false },
            },
            {
                "active_border_color",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Vec4, active_border_color_attribute, false },
            },
            {
                "inactive_border_color",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Vec4, inactive_border_color_attribute, false },
            },
        } });

    // transparent pass
    material_passes.push_back(Visconfig::Components::MaterialPass{ shader_asset_names[1],
        {
            {
                "heatmap_color_start",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Float, heatmap_color_start_attribute, true },
            },
            {
                "heatmap_fill_colors",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Vec4, heatmap_fill_colors_attribute, true },
            },
            {
                "max_access_count",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::UInt, max_access_count_attribute, false },
            },
            {
                "heatmap_color_count",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::UInt, heatmap_color_count_attribute, false },
            },
            {
                "line_width",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Float, line_width_attribute, false },
            },
            {
                "cuboid_size",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Vec3, cuboid_size_attribute, false },
            },
            {
                "active_fill_color",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Vec4, active_fill_color_attribute, false },
            },
            {
                "inactive_fill_color",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Vec4, inactive_fill_color_attribute, false },
            },
            {
                "oob_active_color",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Vec4, oob_active_color_attribute, false },
            },
            {
                "oob_inactive_color",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Vec4, oob_inactive_color_attribute, false },
            },
        } });

    // blend pass
    material_passes.push_back(Visconfig::Components::MaterialPass{ shader_asset_names[2],
        {
            {
                "accum_texture",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Sampler2DMS, accum_texture_attribute, false },
            },
            {
                "revealage_texture",
                Visconfig::Components::MaterialAttribute{
                    Visconfig::Components::MaterialAttributeType::Sampler2DMS, revealage_texture_attribute, false },
            },
        } });

    material.pipeline = pipeline_name;
    material.passes = material_passes;

    layer.mask = 1llu << view_idx;

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;

    transform.position[0] = 0;
    transform.position[1] = 0;
    transform.position[2] = 0;

    cuboid_command_list.global = global;
    cuboid_command_list.draw_heatmap = command_list.heatmap.has_value();
    cuboid_command_list.heatmap_stepping = heatmap_stepping;
    cuboid_command_list.global_size = global_size;
    cuboid_command_list.commands.reserve(command_list.commands.size());
    cuboid_command_list.positions.reserve(command_list.positions.size());

    for (auto& command : command_list.commands) {
        auto visconfig_command = Visconfig::Components::CuboidCommand{};

        switch (command.type) {
        case CuboidCommandType::NOOP:
            visconfig_command.type = Visconfig::Components::CuboidCommandType::NOOP;
            visconfig_command.command = [](auto&& command) -> auto
            {
                return Visconfig::Components::NoopCommand{
                    command.counter,
                };
            }
            (std::get<NoopCommand>(command.command));
            break;
        case CuboidCommandType::DRAW:
            visconfig_command.type = Visconfig::Components::CuboidCommandType::DRAW;
            visconfig_command.command = [=](auto&& command) -> auto
            {
                return Visconfig::Components::DrawCommand{
                    command.out_of_bounds,
                    command.cuboid_idx,
                };
            }
            (std::get<DrawCommand>(command.command));
            break;
        case CuboidCommandType::DRAW_MULTIPLE:
            visconfig_command.type = Visconfig::Components::CuboidCommandType::DRAW_MULTIPLE;
            visconfig_command.command = [=](auto&& command) -> auto
            {
                std::vector<std::tuple<std::size_t, std::size_t>> cuboid_indices;
                std::vector<std::tuple<std::size_t, std::size_t>> out_of_bounds_indices;

                cuboid_indices.reserve(command.cuboid_indices.size());
                out_of_bounds_indices.reserve(command.out_of_bounds.size());

                for (auto idx : command.cuboid_indices) {
                    cuboid_indices.push_back({ idx, command.cuboid_accesses.at(idx) });
                }
                for (auto idx : command.out_of_bounds) {
                    out_of_bounds_indices.push_back({ idx, command.cuboid_accesses.at(idx) });
                }

                return Visconfig::Components::DrawMultipleCommand{
                    std::move(cuboid_indices),
                    std::move(out_of_bounds_indices),
                };
            }
            (std::get<DrawMultipleCommand>(command.command));
            break;
        case CuboidCommandType::DELETE:
            visconfig_command.type = Visconfig::Components::CuboidCommandType::DELETE;
            visconfig_command.command = []([[maybe_unused]] auto&& command) -> auto
            {
                return Visconfig::Components::DeleteCommand{};
            }
            (std::get<DeleteCommand>(command.command));
            break;
        case CuboidCommandType::DELETE_MULTIPLE:
            visconfig_command.type = Visconfig::Components::CuboidCommandType::DELETE_MULTIPLE;
            visconfig_command.command = [](auto&& command) -> auto
            {
                return Visconfig::Components::DeleteMultipleCommand{
                    command.counter,
                };
            }
            (std::get<DeleteMultipleCommand>(command.command));
            break;
        }

        cuboid_command_list.commands.push_back(visconfig_command);
    }

    for (auto& position : command_list.positions) {
        cuboid_command_list.positions.push_back(std::make_tuple(position.position, position.size));
    }

    return entity;
}

void extend_camera_switcher(Visconfig::Entity& coordinator_entity, std::size_t camera_entity_id)
{
    auto camera_switcher{
        std::static_pointer_cast<Visconfig::Components::CameraSwitcherComponent>(
            coordinator_entity.components[coordinator_camera_switcher_idx].data),
    };

    camera_switcher->cameras.push_back(camera_entity_id);
}

void add_color_legend(Visconfig::Entity& coordinator_entity, const std::string& label, const std::string& caption,
    const std::array<std::size_t, 4>& color)
{
    auto canvas = std::static_pointer_cast<Visconfig::Components::CanvasComponent>(
        coordinator_entity.components[coordinator_canvas_idx].data);
    auto& legend_gui = std::get<Visconfig::Components::LegendGUI>(canvas->entries[canvas_legend_gui_idx].gui_data);
    legend_gui.entries.push_back({
        Visconfig::Components::LegendGUIEntryType::ColorEntry,
        Visconfig::Components::LegendGUIColorEntry{
            label,
            caption,
            {
                color[0] / 255.0f,
                color[1] / 255.0f,
                color[2] / 255.0f,
                color[3] / 255.0f,
            },
        },
    });
}

void add_image_legend(Visconfig::Entity& coordinator_entity, const std::string& image, const std::string& description,
    const std::array<float, 2>& scaling, bool absolute)
{
    auto canvas = std::static_pointer_cast<Visconfig::Components::CanvasComponent>(
        coordinator_entity.components[coordinator_canvas_idx].data);
    auto& legend_gui = std::get<Visconfig::Components::LegendGUI>(canvas->entries[canvas_legend_gui_idx].gui_data);
    legend_gui.entries.push_back({
        Visconfig::Components::LegendGUIEntryType::ImageEntry,
        Visconfig::Components::LegendGUIImageEntry{ absolute, image, description, scaling },
    });
}

void add_composition_gui_image(Visconfig::Entity& coordinator_entity, const std::string& group,
    const std::string& group_caption, const std::string& group_id, const std::array<float, 2>& group_position,
    const std::string& id, const std::string& name, const std::string& texture, const std::array<float, 2>& scaling,
    const std::array<float, 2>& position)
{
    auto canvas = std::static_pointer_cast<Visconfig::Components::CanvasComponent>(
        coordinator_entity.components[coordinator_canvas_idx].data);
    auto& composition_gui
        = std::get<Visconfig::Components::CompositionGUI>(canvas->entries[canvas_composition_gui_idx].gui_data);

    Visconfig::Components::CompositionGUIWindow gui_window{ id, name, false, texture, scaling, position };

    if (composition_gui.groups.contains(group)) {
        composition_gui.groups[group].windows.push_back(std::move(gui_window));
    } else {
        composition_gui.groups.insert(
            { group, { false, group_id, group_caption, group_position, { std::move(gui_window) } } });
    }
}

void add_composition_gui_window(Visconfig::Entity& coordinator_entity, const std::string& group,
    const std::string& group_caption, const std::string& group_id, const std::array<float, 2>& group_position,
    const std::string& id, const std::string& window, const std::string& texture, const std::array<float, 2>& scaling,
    const std::array<float, 2>& position)
{
    auto canvas = std::static_pointer_cast<Visconfig::Components::CanvasComponent>(
        coordinator_entity.components[coordinator_canvas_idx].data);
    auto& composition_gui
        = std::get<Visconfig::Components::CompositionGUI>(canvas->entries[canvas_composition_gui_idx].gui_data);

    Visconfig::Components::CompositionGUIWindow gui_window{ id, window, true, texture, scaling, position };

    if (composition_gui.groups.contains(group)) {
        composition_gui.groups[group].windows.push_back(std::move(gui_window));
    } else {
        composition_gui.groups.insert(
            { group, { false, group_id, group_caption, group_position, { std::move(gui_window) } } });
    }
}

void add_composition_gui_connection(
    Visconfig::Entity& coordinator_entity, const std::string& group_source, const std::string& group_destination)
{
    auto canvas = std::static_pointer_cast<Visconfig::Components::CanvasComponent>(
        coordinator_entity.components[coordinator_canvas_idx].data);
    auto& composition_gui
        = std::get<Visconfig::Components::CompositionGUI>(canvas->entries[canvas_composition_gui_idx].gui_data);

    composition_gui.group_connections.push_back({ group_source, group_destination });
}

void add_config_dump_gui_template(Visconfig::Entity& coordinator_entity, const std::string& config_template)
{
    auto canvas = std::static_pointer_cast<Visconfig::Components::CanvasComponent>(
        coordinator_entity.components[coordinator_canvas_idx].data);
    auto& config_dump_gui
        = std::get<Visconfig::Components::ConfigDumpGUI>(canvas->entries[config_dump_gui_idx].gui_data);

    config_dump_gui.config_template = config_template;
}

void add_config_dump_gui_texture(Visconfig::Entity& coordinator_entity, const std::string& id)
{
    auto canvas = std::static_pointer_cast<Visconfig::Components::CanvasComponent>(
        coordinator_entity.components[coordinator_canvas_idx].data);
    auto& config_dump_gui
        = std::get<Visconfig::Components::ConfigDumpGUI>(canvas->entries[config_dump_gui_idx].gui_data);

    config_dump_gui.texture_ids.push_back(id);
}

void add_config_dump_gui_window(Visconfig::Entity& coordinator_entity, bool heatmap, std::size_t heatmap_idx,
    const std::string& id, const std::vector<std::size_t>& entities)
{
    auto canvas = std::static_pointer_cast<Visconfig::Components::CanvasComponent>(
        coordinator_entity.components[coordinator_canvas_idx].data);
    auto& config_dump_gui
        = std::get<Visconfig::Components::ConfigDumpGUI>(canvas->entries[config_dump_gui_idx].gui_data);

    config_dump_gui.windows.push_back({ heatmap, id, heatmap_idx, entities });
}

}