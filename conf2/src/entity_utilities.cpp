#include "entity_utilities.hpp"

namespace Config {

Visconfig::Entity generate_coordinator_entity(std::size_t entity_id)
{
    Visconfig::Entity entity{};
    entity.id = entity_id;

    entity.components.push_back({ Visconfig::Components::ComponentType::Composition,
        std::make_shared<Visconfig::Components::CompositionComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::CameraSwitcher,
        std::make_shared<Visconfig::Components::CameraSwitcherComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Copy, std::make_shared<Visconfig::Components::CopyComponent>() });

    return entity;
}

Visconfig::Entity generate_view_camera(std::size_t entity_id, std::size_t focus_entity_id, std::size_t view_idx,
    float fov, float aspect, float near, float far, float distance, float orthographic_width, float orthographic_height,
    const std::map<std::string, std::string>& targets, bool fixed, bool perspective, bool active)
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
    const CuboidCommandList& command_list, const std::string& front_texture, const std::string& side_texture,
    const std::string& top_texture, const std::string& mesh_name, const std::string& shader_asset_name,
    std::array<int, 3> global_size)
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

    auto texture_front_attribute{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_side_attribute{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_top_attribute{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto active_fill_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto inactive_fill_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto active_border_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };
    auto inactive_border_color_attribute{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    texture_front_attribute->asset = front_texture;
    texture_front_attribute->slot = 0;

    texture_side_attribute->asset = side_texture;
    texture_side_attribute->slot = 1;

    texture_top_attribute->asset = top_texture;
    texture_top_attribute->slot = 2;

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

    material.asset = shader_asset_name;
    material.attributes.insert_or_assign("grid_texture_front",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_front_attribute, false });
    material.attributes.insert_or_assign("grid_texture_side",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_side_attribute, false });
    material.attributes.insert_or_assign("grid_texture_top",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_top_attribute, false });
    material.attributes.insert_or_assign("active_fill_color",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, active_fill_color_attribute, false });
    material.attributes.insert_or_assign("inactive_fill_color",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, inactive_fill_color_attribute, false });
    material.attributes.insert_or_assign("active_border_color",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, active_border_color_attribute, false });
    material.attributes.insert_or_assign("inactive_border_color",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, inactive_border_color_attribute, false });

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
                    command.cuboid_idx,
                };
            }
            (std::get<DrawCommand>(command.command));
            break;
        case CuboidCommandType::DRAW_MULTIPLE:
            visconfig_command.type = Visconfig::Components::CuboidCommandType::DRAW_MULTIPLE;
            visconfig_command.command = [=](auto&& command) -> auto
            {
                return Visconfig::Components::DrawMultipleCommand{
                    std::vector<std::size_t>{ command.cuboid_indices.begin(), command.cuboid_indices.end() },
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
        cuboid_command_list.positions.push_back(position);
    }

    return entity;
}

void extend_copy(Visconfig::Entity& coordinator_entity, const std::string& src_name, const std::string& dst_name,
    Visconfig::Components::CopyOperationFilter filter,
    const std::vector<Visconfig::Components::CopyOperationFlag>& flags)
{
    auto copy_component{
        std::static_pointer_cast<Visconfig::Components::CopyComponent>(coordinator_entity.components[2].data),
    };

    copy_component->operations.push_back(Visconfig::Components::CopyOperation{ src_name, dst_name, flags, filter });
}

void extend_camera_switcher(Visconfig::Entity& coordinator_entity, std::size_t camera_entity_id)
{
    auto camera_switcher{
        std::static_pointer_cast<Visconfig::Components::CameraSwitcherComponent>(coordinator_entity.components[1].data),
    };

    camera_switcher->cameras.push_back(camera_entity_id);
}

void extend_composition(Visconfig::Entity& coordinator_entity, std::array<float, 2> scale,
    std::array<float, 2> position, const std::vector<std::string>& src, const std::string& target,
    const std::string& shader, std::size_t id, [[maybe_unused]] bool draggable)
{
    auto composition{
        std::static_pointer_cast<Visconfig::Components::CompositionComponent>(coordinator_entity.components[0].data),
    };

    composition->operations.push_back(Visconfig::Components::CompositionOperation{
        { scale[0], scale[1] },
        { position[0], position[1] },
        src,
        target,
        shader,
        id,
        draggable,
    });
}

}