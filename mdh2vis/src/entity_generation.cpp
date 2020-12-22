#include <entity_generation.hpp>

#include "templates.hpp"

namespace MDH2Vis {

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

Visconfig::Entity generate_cube(std::array<float, 3> position, std::array<float, 3> scale, std::array<float, 4> color,
    std::size_t entity_id, std::size_t parent_id, std::size_t layer_id, bool has_parent,
    const std::string& front_texture, const std::string& side_texture, const std::string& top_texture,
    const std::string& mesh_name, const std::string& shader_asset_name)
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

    if (has_parent) {
        entity.components.push_back({ Visconfig::Components::ComponentType::Parent,
            std::make_shared<Visconfig::Components::ParentComponent>() });
    }

    [[maybe_unused]] constexpr auto cube_index{ 0 };
    constexpr auto mesh_index{ 1 };
    constexpr auto material_index{ 2 };
    constexpr auto layer_index{ 3 };
    constexpr auto transform_index{ 4 };
    constexpr auto parent_index{ 5 };

    auto& mesh{ *std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[mesh_index].data) };
    auto& material{ *std::static_pointer_cast<Visconfig::Components::MaterialComponent>(
        entity.components[material_index].data) };
    auto& layer{ *std::static_pointer_cast<Visconfig::Components::LayerComponent>(
        entity.components[layer_index].data) };
    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transform_index].data) };

    mesh.asset = mesh_name;

    auto texture_front{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_side{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_top{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuse_color{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    texture_front->asset = front_texture;
    texture_front->slot = 0;

    texture_side->asset = side_texture;
    texture_side->slot = 1;

    texture_top->asset = top_texture;
    texture_top->slot = 2;

    diffuse_color->value[0] = color[0];
    diffuse_color->value[1] = color[1];
    diffuse_color->value[2] = color[2];
    diffuse_color->value[3] = color[3];

    material.asset = shader_asset_name;
    material.attributes.insert_or_assign("gridTextureFront",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_front, false });
    material.attributes.insert_or_assign("gridTextureSide",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_side, false });
    material.attributes.insert_or_assign("gridTextureTop",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_top, false });
    material.attributes.insert_or_assign("diffuse_color",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuse_color, false });

    layer.mask = layer_id;

    transform.position[0] = position[0];
    transform.position[1] = position[1];
    transform.position[2] = position[2];

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = scale[0];
    transform.scale[1] = scale[1];
    transform.scale[2] = scale[2];

    if (has_parent) {
        auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
            entity.components[parent_index].data) };
        parent.id = parent_id;
    }

    return entity;
}

Visconfig::Entity generate_main_view_cube(const ProcessedConfig& mdh_config, const detail::MainViewInfo& view,
    std::size_t entity_id, std::size_t parent_id, std::size_t layer_number, const std::string& front_texture,
    const std::string& side_texture, const std::string& top_texture, const std::string& mesh_name,
    const std::string& shader_asset_name, float min_transparency, float max_transparency)
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

    if (layer_number > 0) {
        entity.components.push_back({ Visconfig::Components::ComponentType::Parent,
            std::make_shared<Visconfig::Components::ParentComponent>() });
        entity.components.push_back({ Visconfig::Components::ComponentType::ImplicitIteration,
            std::make_shared<Visconfig::Components::ImplicitIterationComponent>() });
    }

    [[maybe_unused]] constexpr auto cube_index{ 0 };
    constexpr auto mesh_index{ 1 };
    constexpr auto material_index{ 2 };
    constexpr auto layer_index{ 3 };
    constexpr auto transform_index{ 4 };
    constexpr auto parent_index{ 5 };
    constexpr auto iteration_index{ 6 };

    auto& mesh{ *std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[mesh_index].data) };
    auto& material{ *std::static_pointer_cast<Visconfig::Components::MaterialComponent>(
        entity.components[material_index].data) };
    auto& layer{ *std::static_pointer_cast<Visconfig::Components::LayerComponent>(
        entity.components[layer_index].data) };
    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transform_index].data) };

    mesh.asset = mesh_name;

    auto texture_front{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_side{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_top{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuse_color{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    texture_front->asset = front_texture;
    texture_front->slot = 0;

    texture_side->asset = side_texture;
    texture_side->slot = 1;

    texture_top->asset = top_texture;
    texture_top->slot = 2;

    diffuse_color->value[0] = static_cast<float>(mdh_config.config[layer_number].model.colors.tile[0]) / 255.0f;
    diffuse_color->value[1] = static_cast<float>(mdh_config.config[layer_number].model.colors.tile[1]) / 255.0f;
    diffuse_color->value[2] = static_cast<float>(mdh_config.config[layer_number].model.colors.tile[2]) / 255.0f;
    diffuse_color->value[3] = static_cast<float>(mdh_config.config[layer_number].model.colors.tile[3]) / 255.0f;
    diffuse_color->value[3] = interpolate_linear<std::size_t>(
        min_transparency, max_transparency, 0ull, view.layers.size() + view.threads.size() - 1, layer_number);

    material.asset = shader_asset_name;
    material.attributes.insert_or_assign("gridTextureFront",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_front, false });
    material.attributes.insert_or_assign("gridTextureSide",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_side, false });
    material.attributes.insert_or_assign("gridTextureTop",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_top, false });
    material.attributes.insert_or_assign("diffuse_color",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuse_color, false });

    layer.mask = 1llu;

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = view.layers[layer_number].scale[0];
    transform.scale[1] = view.layers[layer_number].scale[1];
    transform.scale[2] = view.layers[layer_number].scale[2];

    if (layer_number != 0) {
        std::array<float, 3> half_scale{
            view.layers[layer_number].scale[0] / 2.0f,
            view.layers[layer_number].scale[1] / 2.0f,
            view.layers[layer_number].scale[2] / 2.0f,
        };

        transform.position[0] = -0.5f + half_scale[0];
        transform.position[1] = 0.5f - half_scale[1];
        transform.position[2] = -0.5f + half_scale[2];
    } else {
        transform.position[0] = 0;
        transform.position[1] = 0;
        transform.position[2] = 0;
    }

    if (layer_number != 0) {
        auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
            entity.components[parent_index].data) };
        auto& iteration{ *std::static_pointer_cast<Visconfig::Components::ImplicitIterationComponent>(
            entity.components[iteration_index].data) };

        parent.id = parent_id;

        iteration.order = Visconfig::Components::IterationOrder::XYZ;
        iteration.numIterations[0] = view.layers[layer_number].num_iterations[0];
        iteration.numIterations[1] = view.layers[layer_number].num_iterations[1];
        iteration.numIterations[2] = view.layers[layer_number].num_iterations[2];

        iteration.ticksPerIteration = view.layers[layer_number].absolute_scale[0]
            * view.layers[layer_number].absolute_scale[1] * view.layers[layer_number].absolute_scale[2];
        iteration.ticksPerIteration /= view.threads[0].absolute_scale[0] * view.threads[0].absolute_scale[1]
            * view.threads[0].absolute_scale[2];
    }

    return entity;
}

Visconfig::Entity generate_main_view_thread_cube(const ProcessedConfig& mdh_config, const detail::MainViewInfo& view,
    std::size_t entity_id, std::size_t parent_id, std::size_t layer_number, std::array<std::size_t, 3> block_number,
    const std::string& front_texture, const std::string& side_texture, const std::string& top_texture,
    const std::string& mesh_name, const std::string& shader_asset_name, float min_transparency, float max_transparency)
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
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Parent, std::make_shared<Visconfig::Components::ParentComponent>() });

    if (layer_number == 0) {
        entity.components.push_back({ Visconfig::Components::ComponentType::ImplicitIteration,
            std::make_shared<Visconfig::Components::ImplicitIterationComponent>() });
    }

    [[maybe_unused]] constexpr auto cube_index{ 0 };
    constexpr auto mesh_index{ 1 };
    constexpr auto material_index{ 2 };
    constexpr auto layer_index{ 3 };
    constexpr auto transform_index{ 4 };
    constexpr auto parent_index{ 5 };
    constexpr auto iteration_index{ 6 };

    auto& mesh{ *std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[mesh_index].data) };
    auto& material{ *std::static_pointer_cast<Visconfig::Components::MaterialComponent>(
        entity.components[material_index].data) };
    auto& layer{ *std::static_pointer_cast<Visconfig::Components::LayerComponent>(
        entity.components[layer_index].data) };
    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transform_index].data) };
    auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
        entity.components[parent_index].data) };

    mesh.asset = mesh_name;

    auto texture_front{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_side{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_top{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuse_color{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    texture_front->asset = front_texture;
    texture_front->slot = 0;

    texture_side->asset = side_texture;
    texture_side->slot = 1;

    texture_top->asset = top_texture;
    texture_top->slot = 2;

    diffuse_color->value[0] = static_cast<float>(mdh_config.config[layer_number].model.colors.thread[0]) / 255.0f;
    diffuse_color->value[1] = static_cast<float>(mdh_config.config[layer_number].model.colors.thread[1]) / 255.0f;
    diffuse_color->value[2] = static_cast<float>(mdh_config.config[layer_number].model.colors.thread[2]) / 255.0f;
    diffuse_color->value[3] = static_cast<float>(mdh_config.config[layer_number].model.colors.thread[3]) / 255.0f;
    diffuse_color->value[3] = interpolate_linear<std::size_t>(min_transparency, max_transparency, 0ull,
        view.layers.size() + view.threads.size() - 1, view.layers.size() + layer_number);

    material.asset = shader_asset_name;
    material.attributes.insert_or_assign("gridTextureFront",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_front, false });
    material.attributes.insert_or_assign("gridTextureSide",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_side, false });
    material.attributes.insert_or_assign("gridTextureTop",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_top, false });
    material.attributes.insert_or_assign("diffuse_color",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuse_color, false });

    layer.mask = 1llu;

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = view.threads[layer_number].scale[0];
    transform.scale[1] = view.threads[layer_number].scale[1];
    transform.scale[2] = view.threads[layer_number].scale[2];

    std::array<float, 3> half_scale{
        view.threads[layer_number].scale[0] / 2.0f,
        view.threads[layer_number].scale[1] / 2.0f,
        view.threads[layer_number].scale[2] / 2.0f,
    };

    transform.position[0] = -0.5f + half_scale[0] + (block_number[0] * transform.scale[0]);
    transform.position[1] = 0.5f - half_scale[1] - (block_number[1] * transform.scale[1]);
    transform.position[2] = -0.5f + half_scale[2] + (block_number[2] * transform.scale[2]);

    parent.id = parent_id;

    if (layer_number == 0) {
        auto& iteration{ *std::static_pointer_cast<Visconfig::Components::ImplicitIterationComponent>(
            entity.components[iteration_index].data) };

        iteration.order = Visconfig::Components::IterationOrder::XYZ;
        iteration.numIterations[0] = view.threads[layer_number].num_iterations[0];
        iteration.numIterations[1] = view.threads[layer_number].num_iterations[1];
        iteration.numIterations[2] = view.threads[layer_number].num_iterations[2];

        /*
        iteration.ticksPerIteration = view.threads[layer_number].absolute_scale[0]
            * view.threads[layer_number].absolute_scale[1] * view.threads[layer_number].absolute_scale[2];
        */
        iteration.ticksPerIteration = 1;
    }

    return entity;
}

Visconfig::Entity generate_output_view_cube(Visconfig::World& world, const ProcessedConfig& mdh_config,
    const detail::OutputViewInfo& view, size_t& entity_id, std::size_t parent_id, std::size_t view_number,
    std::size_t layer_number, const std::string& cube_texture, const std::string& mesh_name,
    const std::string& shader_asset_name, float min_transparency, float max_transparency)
{
    Visconfig::Entity entity{};
    entity.id = entity_id;

    entity.components.push_back({ Visconfig::Components::ComponentType::Transform,
        std::make_shared<Visconfig::Components::TransformComponent>() });
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Parent, std::make_shared<Visconfig::Components::ParentComponent>() });

    if (layer_number > 0) {
        entity.components.push_back({ Visconfig::Components::ComponentType::ImplicitIteration,
            std::make_shared<Visconfig::Components::ImplicitIterationComponent>() });
    }

    constexpr auto transform_index{ 0 };
    constexpr auto parent_index{ 1 };
    constexpr auto iteration_index{ 2 };

    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transform_index].data) };
    auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
        entity.components[parent_index].data) };

    std::array<float, 3> half_scale{
        view.layers[layer_number].size[0] / 2.0f,
        view.layers[layer_number].size[1] / 2.0f,
        view.layers[layer_number].size[2] / 2.0f,
    };

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = view.layers[layer_number].size[0];
    transform.scale[1] = view.layers[layer_number].size[1];
    transform.scale[2] = view.layers[layer_number].size[2];

    transform.position[0] = -0.5f + half_scale[0];
    transform.position[1] = 0.5f - half_scale[1];
    transform.position[2] = -0.5f + half_scale[2];

    std::array<float, 4> color{
        static_cast<float>(mdh_config.config[layer_number].model.colors.tile[0]) / 255.0f,
        static_cast<float>(mdh_config.config[layer_number].model.colors.tile[1]) / 255.0f,
        static_cast<float>(mdh_config.config[layer_number].model.colors.tile[2]) / 255.0f,
        interpolate_linear<std::size_t>(min_transparency, max_transparency, 0ull,
            mdh_config.main_view.layers.size() + mdh_config.main_view.threads.size() - 1, layer_number),
    };

    auto& thread_layer{ mdh_config.main_view.threads.front() };
    std::array<float, 3> child_scale{
        thread_layer.absolute_scale[0] / mdh_config.output_view.layers[layer_number].absolute_size[0],
        thread_layer.absolute_scale[1] / mdh_config.output_view.layers[layer_number].absolute_size[1],
        thread_layer.absolute_scale[2] / mdh_config.output_view.layers[layer_number].absolute_size[2],
    };

    std::array<float, 3> child_start_pos{
        -0.5f,
        0.5f,
        -0.5f,
    };

    auto parent_entity{ entity_id };

    auto child{ generate_cube(child_start_pos, child_scale, color, ++entity_id, parent_entity, 1llu << view_number,
        true, cube_texture, cube_texture, cube_texture, mesh_name, shader_asset_name) };
    child.components.push_back({ Visconfig::Components::ComponentType::MeshIteration,
        std::make_shared<Visconfig::Components::MeshIterationComponent>() });

    auto& child_mesh_iteration{ *std::static_pointer_cast<Visconfig::Components::MeshIterationComponent>(
        child.components.back().data) };
    child_mesh_iteration.dimensions = {
        static_cast<std::size_t>(
            std::abs(view.layers[layer_number].absolute_size[0] / mdh_config.main_view.threads[0].absolute_scale[0])),
        static_cast<std::size_t>(
            std::abs(view.layers[layer_number].absolute_size[1] / mdh_config.main_view.threads[0].absolute_scale[1])),
        static_cast<std::size_t>(
            std::abs(view.layers[layer_number].absolute_size[2] / mdh_config.main_view.threads[0].absolute_scale[2])),
    };
    child_mesh_iteration.positions = view.layers[layer_number].grid_positions;
    child_mesh_iteration.ticksPerIteration = view.layers[layer_number].iteration_rates;

    world.entities.push_back(child);

    parent.id = parent_id;

    if (layer_number > 0) {
        auto& iteration{ *std::static_pointer_cast<Visconfig::Components::ImplicitIterationComponent>(
            entity.components[iteration_index].data) };

        iteration.order = Visconfig::Components::IterationOrder::XYZ;
        iteration.numIterations[0] = view.layers[layer_number].num_iterations[0];
        iteration.numIterations[1] = view.layers[layer_number].num_iterations[1];
        iteration.numIterations[2] = view.layers[layer_number].num_iterations[2];
        iteration.ticksPerIteration = view.layers[layer_number].iteration_rate;
    }

    return entity;
}

Visconfig::Entity generate_sub_view_cube(const ProcessedConfig& mdh_config, const detail::SubViewInfo& view,
    std::size_t entity_id, std::size_t parent_id, std::size_t view_number, std::size_t layer_number,
    const std::string& front_texture, const std::string& side_texture, const std::string& top_texture,
    const std::string& mesh_name, const std::string& shader_asset_name, float min_transparency, float max_transparency)
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
    entity.components.push_back(
        { Visconfig::Components::ComponentType::Parent, std::make_shared<Visconfig::Components::ParentComponent>() });
    entity.components.push_back({ Visconfig::Components::ComponentType::ExplicitIteration,
        std::make_shared<Visconfig::Components::ExplicitIterationComponent>() });

    [[maybe_unused]] constexpr auto cube_index{ 0 };
    constexpr auto mesh_index{ 1 };
    constexpr auto material_index{ 2 };
    constexpr auto layer_index{ 3 };
    constexpr auto transform_index{ 4 };
    constexpr auto parent_index{ 5 };
    constexpr auto iteration_index{ 6 };

    auto& mesh{ *std::static_pointer_cast<Visconfig::Components::MeshComponent>(entity.components[mesh_index].data) };
    auto& material{ *std::static_pointer_cast<Visconfig::Components::MaterialComponent>(
        entity.components[material_index].data) };
    auto& layer{ *std::static_pointer_cast<Visconfig::Components::LayerComponent>(
        entity.components[layer_index].data) };
    auto& transform{ *std::static_pointer_cast<Visconfig::Components::TransformComponent>(
        entity.components[transform_index].data) };
    auto& parent{ *std::static_pointer_cast<Visconfig::Components::ParentComponent>(
        entity.components[parent_index].data) };
    auto& iteration{ *std::static_pointer_cast<Visconfig::Components::ExplicitIterationComponent>(
        entity.components[iteration_index].data) };

    mesh.asset = mesh_name;

    auto texture_front{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_side{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto texture_top{ std::make_shared<Visconfig::Components::Sampler2DMaterialAttribute>() };
    auto diffuse_color{ std::make_shared<Visconfig::Components::Vec4MaterialAttribute>() };

    texture_front->asset = front_texture;
    texture_front->slot = 0;

    texture_side->asset = side_texture;
    texture_side->slot = 1;

    texture_top->asset = top_texture;
    texture_top->slot = 2;

    std::size_t color_layer;

    if (mdh_config.config[layer_number].tps.memRegionInp.contains(view.name)) {
        color_layer = mdh_config.config[layer_number].tps.memRegionInp.at(view.name);
    } else {
        color_layer = mdh_config.config[layer_number].tps.memRegionRes.at(view.name);
    }

    diffuse_color->value[0] = static_cast<float>(mdh_config.config[color_layer].model.colors.memory[0]) / 255.0f;
    diffuse_color->value[1] = static_cast<float>(mdh_config.config[color_layer].model.colors.memory[1]) / 255.0f;
    diffuse_color->value[2] = static_cast<float>(mdh_config.config[color_layer].model.colors.memory[2]) / 255.0f;
    diffuse_color->value[3] = static_cast<float>(mdh_config.config[color_layer].model.colors.memory[3]) / 255.0f;
    diffuse_color->value[3] = interpolate_linear<std::size_t>(
        min_transparency, max_transparency, 0ull, view.layers.size() - 1, layer_number);
    diffuse_color->value[3] = interpolate_linear<std::size_t>(min_transparency, max_transparency, 0ull,
        mdh_config.main_view.layers.size() + mdh_config.main_view.threads.size() - 1, layer_number);

    material.asset = shader_asset_name;
    material.attributes.insert_or_assign("gridTextureFront",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_front, false });
    material.attributes.insert_or_assign("gridTextureSide",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_side, false });
    material.attributes.insert_or_assign("gridTextureTop",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Sampler2D, texture_top, false });
    material.attributes.insert_or_assign("diffuse_color",
        Visconfig::Components::MaterialAttribute{
            Visconfig::Components::MaterialAttributeType::Vec4, diffuse_color, false });

    layer.mask = 1llu << (view_number + 2);

    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;

    transform.scale[0] = view.layers[layer_number].scale[0];
    transform.scale[1] = view.layers[layer_number].scale[1];
    transform.scale[2] = view.layers[layer_number].scale[2];

    std::array<float, 3> half_scale{
        view.layers[layer_number].scale[0] / 2.0f,
        view.layers[layer_number].scale[1] / 2.0f,
        view.layers[layer_number].scale[2] / 2.0f,
    };

    if (layer_number > 0) {
        transform.position[0] = -0.5f + half_scale[0];
        transform.position[1] = 0.5f - half_scale[1];
        transform.position[2] = -0.5f + half_scale[2];
    } else {
        transform.position[0] = 0.0f;
        transform.position[1] = 0.0f;
        transform.position[2] = 0.0f;
    }

    parent.id = parent_id;

    iteration.positions = view.layers[layer_number].positions;
    iteration.ticksPerIteration = view.layers[layer_number].iteration_rate;

    return entity;
}

Visconfig::Entity generate_view_camera(std::size_t entity_id, std::size_t focus, std::size_t view_index,
    const std::string& framebuffer_name, float fov, float aspect, float near, float far, float distance,
    float orthographic_width, float orthographic_height, bool active, bool fixed, bool perspective)
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
    camera->layerMask = 1ull << view_index;
    camera->targets.insert_or_assign("cube", framebuffer_name);
    camera->targets.insert_or_assign("text", framebuffer_name);

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
    fixed_camera->focus = focus;
    fixed_camera->distance = distance;
    fixed_camera->horizontalAngle = 0.0f;
    fixed_camera->verticalAngle = 0.0f;

    return entity;
}

void extend_camera_switcher(Visconfig::World& world, std::size_t camera)
{
    auto camera_switcher{ std::static_pointer_cast<Visconfig::Components::CameraSwitcherComponent>(
        world.entities[0].components[1].data) };

    camera_switcher->cameras.push_back(camera);
}

void extend_copy(Visconfig::World& world, const std::string& source, const std::string& destination,
    const std::vector<Visconfig::Components::CopyOperationFlag>& flags,
    Visconfig::Components::CopyOperationFilter filter)
{
    auto copy_component{ std::static_pointer_cast<Visconfig::Components::CopyComponent>(
        world.entities[0].components[2].data) };

    copy_component->operations.push_back(Visconfig::Components::CopyOperation{ source, destination, flags, filter });
}

void extend_composition(Visconfig::World& world, std::array<float, 2> scale, std::array<float, 2> position,
    std::vector<std::string> src, const std::string& target, const std::string& shader, std::size_t id, bool draggable)
{
    auto composition{ std::static_pointer_cast<Visconfig::Components::CompositionComponent>(
        world.entities[0].components[0].data) };

    composition->operations.push_back(Visconfig::Components::CompositionOperation{
        { scale[0], scale[1] }, { position[0], position[1] }, std::move(src), target, shader, id, draggable });
}

}
