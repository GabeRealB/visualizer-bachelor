#include <visconfig/Components.hpp>

namespace Visconfig::Components {

void to_json(nlohmann::json& j, const std::shared_ptr<ComponentData>& v, ComponentType type)
{
    switch (type) {
    case ComponentType::Cube:
        to_json(j, *std::static_pointer_cast<CubeComponent>(v));
        break;
    case ComponentType::Mesh:
        to_json(j, *std::static_pointer_cast<MeshComponent>(v));
        break;
    case ComponentType::Parent:
        to_json(j, *std::static_pointer_cast<ParentComponent>(v));
        break;
    case ComponentType::Material:
        to_json(j, *std::static_pointer_cast<MaterialComponent>(v));
        break;
    case ComponentType::Layer:
        to_json(j, *std::static_pointer_cast<LayerComponent>(v));
        break;
    case ComponentType::Transform:
        to_json(j, *std::static_pointer_cast<TransformComponent>(v));
        break;
    case ComponentType::ImplicitIteration:
        to_json(j, *std::static_pointer_cast<ImplicitIterationComponent>(v));
        break;
    case ComponentType::ExplicitIteration:
        to_json(j, *std::static_pointer_cast<ExplicitIterationComponent>(v));
        break;
    case ComponentType::EntityActivation:
        to_json(j, *std::static_pointer_cast<EntityActivationComponent>(v));
        break;
    case ComponentType::MeshIteration:
        to_json(j, *std::static_pointer_cast<MeshIterationComponent>(v));
        break;
    case ComponentType::ExplicitHeterogeneousIteration:
        to_json(j, *std::static_pointer_cast<ExplicitHeterogeneousIterationComponent>(v));
        break;
    case ComponentType::CuboidCommandList:
        to_json(j, *std::static_pointer_cast<CuboidCommandListComponent>(v));
        break;
    case ComponentType::Camera:
        to_json(j, *std::static_pointer_cast<CameraComponent>(v));
        break;
    case ComponentType::FreeFlyCamera:
        to_json(j, *std::static_pointer_cast<FreeFlyCameraComponent>(v));
        break;
    case ComponentType::FixedCamera:
        to_json(j, *std::static_pointer_cast<FixedCameraComponent>(v));
        break;
    case ComponentType::CameraSwitcher:
        to_json(j, *std::static_pointer_cast<CameraSwitcherComponent>(v));
        break;
    case ComponentType::Composition:
        to_json(j, *std::static_pointer_cast<CompositionComponent>(v));
        break;
    case ComponentType::Copy:
        to_json(j, *std::static_pointer_cast<CopyComponent>(v));
        break;
    case ComponentType::Canvas:
        to_json(j, *std::static_pointer_cast<CanvasComponent>(v));
        break;
    }
}

void from_json(const nlohmann::json& j, std::shared_ptr<ComponentData>& v, ComponentType type)
{
    switch (type) {
    case ComponentType::Cube: {
        auto ptr{ std::make_shared<CubeComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::Mesh: {
        auto ptr{ std::make_shared<MeshComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::Parent: {
        auto ptr{ std::make_shared<ParentComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::Material: {
        auto ptr{ std::make_shared<MaterialComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::Layer: {
        auto ptr{ std::make_shared<LayerComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::Transform: {
        auto ptr{ std::make_shared<TransformComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::ImplicitIteration: {
        auto ptr{ std::make_shared<ImplicitIterationComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::ExplicitIteration: {
        auto ptr{ std::make_shared<ExplicitIterationComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::EntityActivation: {
        auto ptr{ std::make_shared<EntityActivationComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::MeshIteration: {
        auto ptr{ std::make_shared<MeshIterationComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::ExplicitHeterogeneousIteration: {
        auto ptr{ std::make_shared<ExplicitHeterogeneousIterationComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::CuboidCommandList: {
        auto ptr{ std::make_shared<CuboidCommandListComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::Camera: {
        auto ptr{ std::make_shared<CameraComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::FreeFlyCamera: {
        auto ptr{ std::make_shared<FreeFlyCameraComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::FixedCamera: {
        auto ptr{ std::make_shared<FixedCameraComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::CameraSwitcher: {
        auto ptr{ std::make_shared<CameraSwitcherComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::Composition: {
        auto ptr{ std::make_shared<CompositionComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::Copy: {
        auto ptr{ std::make_shared<CopyComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    case ComponentType::Canvas: {
        auto ptr{ std::make_shared<CanvasComponent>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<ComponentData>(ptr);
    } break;
    }
}

/*Enums*/

std::unordered_map<ComponentType, std::string> sComponentTypeStringNameMap{
    { ComponentType::Cube, "cube" },
    { ComponentType::Mesh, "mesh" },
    { ComponentType::Parent, "parent" },
    { ComponentType::Material, "material" },
    { ComponentType::Layer, "layer" },
    { ComponentType::Transform, "transform" },
    { ComponentType::ImplicitIteration, "implicit_iteration" },
    { ComponentType::ExplicitIteration, "explicit_iteration" },
    { ComponentType::EntityActivation, "entity_activation" },
    { ComponentType::MeshIteration, "mesh_iteration" },
    { ComponentType::ExplicitHeterogeneousIteration, "explicit_heterogeneous_iteration" },
    { ComponentType::CuboidCommandList, "cuboid_command_list" },
    { ComponentType::Camera, "camera" },
    { ComponentType::FreeFlyCamera, "free_fly_camera" },
    { ComponentType::FixedCamera, "fixed_camera" },
    { ComponentType::CameraSwitcher, "camera_switcher" },
    { ComponentType::Composition, "composition" },
    { ComponentType::Copy, "copy" },
    { ComponentType::Canvas, "canvas" },
};

void to_json(nlohmann::json& j, const ComponentType& v) { j = sComponentTypeStringNameMap[v]; }

void from_json(const nlohmann::json& j, ComponentType& v)
{
    std::string type{};
    j.get_to(type);

    auto predicate{ [&](const decltype(sComponentTypeStringNameMap)::value_type& pair) {
        return pair.second == type;
    } };
    if (auto pos{ std::find_if(sComponentTypeStringNameMap.begin(), sComponentTypeStringNameMap.end(), predicate) };
        pos != sComponentTypeStringNameMap.end()) {
        v = pos->first;
    } else {
        std::abort();
    }
}

std::unordered_map<IterationOrder, std::string> sIterationOrderStringNameMap{
    { IterationOrder::XYZ, "xyz" },
    { IterationOrder::XZY, "xzy" },
    { IterationOrder::YXZ, "yxz" },
    { IterationOrder::YZX, "yzx" },
    { IterationOrder::ZXY, "zxy" },
    { IterationOrder::ZYX, "zyx" },
};

void to_json(nlohmann::json& j, const IterationOrder& v) { j = sIterationOrderStringNameMap[v]; }

void from_json(const nlohmann::json& j, IterationOrder& v)
{
    std::string type{};
    j.get_to(type);

    auto predicate{ [&](const decltype(sIterationOrderStringNameMap)::value_type& pair) {
        return pair.second == type;
    } };
    if (auto pos{ std::find_if(sIterationOrderStringNameMap.begin(), sIterationOrderStringNameMap.end(), predicate) };
        pos != sIterationOrderStringNameMap.end()) {
        v = pos->first;
    } else {
        std::abort();
    }
}

std::unordered_map<CuboidCommandType, std::string> sCuboidCommandTypeStringNameMap{
    { CuboidCommandType::NOOP, "noop" },
    { CuboidCommandType::DRAW, "draw" },
    { CuboidCommandType::DRAW_MULTIPLE, "draw_multiple" },
    { CuboidCommandType::DELETE, "delete" },
    { CuboidCommandType::DELETE_MULTIPLE, "delete_multiple" },
};

void to_json(nlohmann::json& j, const CuboidCommandType& v) { j = sCuboidCommandTypeStringNameMap[v]; }

void from_json(const nlohmann::json& j, CuboidCommandType& v)
{
    std::string type{};
    j.get_to(type);

    auto predicate{ [&](const decltype(sCuboidCommandTypeStringNameMap)::value_type& pair) {
        return pair.second == type;
    } };
    if (auto pos{
            std::find_if(sCuboidCommandTypeStringNameMap.begin(), sCuboidCommandTypeStringNameMap.end(), predicate) };
        pos != sCuboidCommandTypeStringNameMap.end()) {
        v = pos->first;
    } else {
        std::abort();
    }
}

std::unordered_map<CopyOperationFlag, std::string> sCopyOperationFlagStringNameMap{
    { CopyOperationFlag::Color, "color" },
    { CopyOperationFlag::Depth, "depth" },
    { CopyOperationFlag::Stencil, "stencil" },
};

void to_json(nlohmann::json& j, const CopyOperationFlag& v) { j = sCopyOperationFlagStringNameMap[v]; }

void from_json(const nlohmann::json& j, CopyOperationFlag& v)
{
    std::string type{};
    j.get_to(type);

    auto predicate{ [&](const decltype(sCopyOperationFlagStringNameMap)::value_type& pair) {
        return pair.second == type;
    } };
    if (auto pos{
            std::find_if(sCopyOperationFlagStringNameMap.begin(), sCopyOperationFlagStringNameMap.end(), predicate) };
        pos != sCopyOperationFlagStringNameMap.end()) {
        v = pos->first;
    } else {
        std::abort();
    }
}

std::unordered_map<CopyOperationFilter, std::string> sCopyOperationFilterStringNameMap{
    { CopyOperationFilter::Nearest, "nearest" },
    { CopyOperationFilter::Linear, "linear" },
};

void to_json(nlohmann::json& j, const CopyOperationFilter& v) { j = sCopyOperationFilterStringNameMap[v]; }

void from_json(const nlohmann::json& j, CopyOperationFilter& v)
{
    std::string type{};
    j.get_to(type);

    auto predicate{ [&](const decltype(sCopyOperationFilterStringNameMap)::value_type& pair) {
        return pair.second == type;
    } };
    if (auto pos{ std::find_if(
            sCopyOperationFilterStringNameMap.begin(), sCopyOperationFilterStringNameMap.end(), predicate) };
        pos != sCopyOperationFilterStringNameMap.end()) {
        v = pos->first;
    } else {
        std::abort();
    }
}

/*Components*/

void to_json(nlohmann::json&, const CubeComponent&) {}

void from_json(const nlohmann::json&, CubeComponent&) {}

void to_json(nlohmann::json& j, const MeshComponent& v) { j[MeshComponent::assetJson] = v.asset; }

void from_json(const nlohmann::json& j, MeshComponent& v) { j[MeshComponent::assetJson].get_to(v.asset); }

void to_json(nlohmann::json& j, const ParentComponent& v) { j[ParentComponent::idJson] = v.id; }

void from_json(const nlohmann::json& j, ParentComponent& v) { j[ParentComponent::idJson].get_to(v.id); }

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MaterialComponent, pipeline, passes)

void to_json(nlohmann::json& j, const LayerComponent& v) { j[LayerComponent::maskJson] = v.mask; }

void from_json(const nlohmann::json& j, LayerComponent& v) { j[LayerComponent::maskJson].get_to(v.mask); }

void to_json(nlohmann::json& j, const TransformComponent& v)
{
    j[TransformComponent::scaleJson] = v.scale;
    j[TransformComponent::positionJson] = v.position;
    j[TransformComponent::rotationJson] = v.rotation;
}

void from_json(const nlohmann::json& j, TransformComponent& v)
{
    j[TransformComponent::scaleJson].get_to(v.scale);
    j[TransformComponent::positionJson].get_to(v.position);
    j[TransformComponent::rotationJson].get_to(v.rotation);
}

void to_json(nlohmann::json& j, const ImplicitIterationComponent& v)
{
    j[ImplicitIterationComponent::orderJson] = v.order;
    j[ImplicitIterationComponent::startPosJson] = v.startPos;
    j[ImplicitIterationComponent::numIterationsJson] = v.numIterations;
    j[ImplicitIterationComponent::ticksPerIterationJson] = v.ticksPerIteration;
}

void from_json(const nlohmann::json& j, ImplicitIterationComponent& v)
{
    j[ImplicitIterationComponent::orderJson].get_to(v.order);
    j[ImplicitIterationComponent::startPosJson].get_to(v.startPos);
    j[ImplicitIterationComponent::numIterationsJson].get_to(v.numIterations);
    j[ImplicitIterationComponent::ticksPerIterationJson].get_to(v.ticksPerIteration);
}

void to_json(nlohmann::json& j, const ExplicitIterationComponent& v)
{
    j[ExplicitIterationComponent::positionsJson] = v.positions;
    j[ExplicitIterationComponent::ticksPerIterationJson] = v.ticksPerIteration;
}

void from_json(const nlohmann::json& j, ExplicitIterationComponent& v)
{
    j[ExplicitIterationComponent::positionsJson].get_to(v.positions);
    j[ExplicitIterationComponent::ticksPerIterationJson].get_to(v.ticksPerIteration);
}

void to_json(nlohmann::json& j, const EntityActivationComponent& v)
{
    j[EntityActivationComponent::layerJson] = v.layer;
    j[EntityActivationComponent::entitiesJson] = v.entities;
    j[EntityActivationComponent::ticksPerIterationJson] = v.ticksPerIteration;
}

void from_json(const nlohmann::json& j, EntityActivationComponent& v)
{
    j[EntityActivationComponent::layerJson].get_to(v.layer);
    j[EntityActivationComponent::entitiesJson].get_to(v.entities);
    j[EntityActivationComponent::ticksPerIterationJson].get_to(v.ticksPerIteration);
}

void to_json(nlohmann::json& j, const MeshIterationComponent& v)
{
    j[MeshIterationComponent::dimensionsJson] = v.dimensions;
    j[MeshIterationComponent::positionsJson] = v.positions;
    j[MeshIterationComponent::ticksPerIterationJson] = v.ticksPerIteration;
}

void from_json(const nlohmann::json& j, MeshIterationComponent& v)
{
    j[MeshIterationComponent::dimensionsJson].get_to(v.dimensions);
    j[MeshIterationComponent::positionsJson].get_to(v.positions);
    j[MeshIterationComponent::ticksPerIterationJson].get_to(v.ticksPerIteration);
}

void to_json(nlohmann::json& j, const ExplicitHeterogeneousIterationComponent& v)
{
    j[ExplicitHeterogeneousIterationComponent::scalesJson] = v.scales;
    j[ExplicitHeterogeneousIterationComponent::positionsJson] = v.positions;
    j[ExplicitHeterogeneousIterationComponent::ticksPerIterationJson] = v.ticksPerIteration;
}

void from_json(const nlohmann::json& j, ExplicitHeterogeneousIterationComponent& v)
{
    j[ExplicitHeterogeneousIterationComponent::scalesJson].get_to(v.scales);
    j[ExplicitHeterogeneousIterationComponent::positionsJson].get_to(v.positions);
    j[ExplicitHeterogeneousIterationComponent::ticksPerIterationJson].get_to(v.ticksPerIteration);
}

void to_json(nlohmann::json& j, const CuboidCommandListComponent& v)
{
    j[CuboidCommandListComponent::global_json] = v.global;
    j[CuboidCommandListComponent::global_size_json] = v.global_size;
    j[CuboidCommandListComponent::commands_json] = v.commands;
    j[CuboidCommandListComponent::positions_json] = v.positions;
}

void from_json(const nlohmann::json& j, CuboidCommandListComponent& v)
{
    j[CuboidCommandListComponent::global_json].get_to(v.global);
    j[CuboidCommandListComponent::global_size_json].get_to(v.global_size);
    j[CuboidCommandListComponent::commands_json].get_to(v.commands);
    j[CuboidCommandListComponent::positions_json].get_to(v.positions);
}

void to_json(nlohmann::json& j, const CameraComponent& v)
{
    j[CameraComponent::activeJson] = v.active;
    j[CameraComponent::fixedJson] = v.fixed;
    j[CameraComponent::perspectiveJson] = v.perspective;
    j[CameraComponent::fovJson] = v.fov;
    j[CameraComponent::farJson] = v.far;
    j[CameraComponent::nearJson] = v.near;
    j[CameraComponent::aspectJson] = v.aspect;
    j[CameraComponent::orthographicWidthJson] = v.orthographicWidth;
    j[CameraComponent::orthographicHeightJson] = v.orthographicHeight;
    j[CameraComponent::layerMaskJson] = v.layerMask.to_string();
    j[CameraComponent::targetsJson] = v.targets;
}

void from_json(const nlohmann::json& j, CameraComponent& v)
{
    j[CameraComponent::activeJson].get_to(v.active);
    j[CameraComponent::fixedJson].get_to(v.fixed);
    j[CameraComponent::perspectiveJson].get_to(v.perspective);
    j[CameraComponent::fovJson].get_to(v.fov);
    j[CameraComponent::farJson].get_to(v.far);
    j[CameraComponent::nearJson].get_to(v.near);
    j[CameraComponent::aspectJson].get_to(v.aspect);
    j[CameraComponent::orthographicWidthJson].get_to(v.orthographicWidth);
    j[CameraComponent::orthographicHeightJson].get_to(v.orthographicHeight);
    v.layerMask = std::bitset<64>{ j[CameraComponent::layerMaskJson].get<std::string>() };
    j[CameraComponent::targetsJson].get_to(v.targets);
}

void to_json(nlohmann::json&, const FreeFlyCameraComponent&) {}

void from_json(const nlohmann::json&, FreeFlyCameraComponent&) {}

void to_json(nlohmann::json& j, const FixedCameraComponent& v)
{
    j[FixedCameraComponent::focusJson] = v.focus;
    j[FixedCameraComponent::distanceJson] = v.distance;
    j[FixedCameraComponent::horizontalAngleJson] = v.horizontalAngle;
    j[FixedCameraComponent::verticalAngleJson] = v.verticalAngle;
}

void from_json(const nlohmann::json& j, FixedCameraComponent& v)
{
    j[FixedCameraComponent::focusJson].get_to(v.focus);
    j[FixedCameraComponent::distanceJson].get_to(v.distance);
    j[FixedCameraComponent::horizontalAngleJson].get_to(v.horizontalAngle);
    j[FixedCameraComponent::verticalAngleJson].get_to(v.verticalAngle);
}

void to_json(nlohmann::json& j, const CameraSwitcherComponent& v)
{
    j[CameraSwitcherComponent::camerasJson] = v.cameras;
    j[CameraSwitcherComponent::activeJson] = v.active;
}

void from_json(const nlohmann::json& j, CameraSwitcherComponent& v)
{
    j[CameraSwitcherComponent::camerasJson].get_to(v.cameras);
    j[CameraSwitcherComponent::activeJson].get_to(v.active);
}

void to_json(nlohmann::json& j, const CompositionComponent& v)
{
    j[CompositionComponent::operationsJson] = v.operations;
}

void from_json(const nlohmann::json& j, CompositionComponent& v)
{
    j[CompositionComponent::operationsJson].get_to(v.operations);
}

void to_json(nlohmann::json& j, const CopyComponent& v) { j[CopyComponent::operationsJson] = v.operations; }

void from_json(const nlohmann::json& j, CopyComponent& v) { j[CopyComponent::operationsJson].get_to(v.operations); }

void to_json(nlohmann::json& j, const CanvasComponent& v) { j[CanvasComponent::entries_json] = v.entries; }

void from_json(const nlohmann::json& j, CanvasComponent& v) { j[CanvasComponent::entries_json].get_to(v.entries); }

/*Internal Structs*/

void to_json(nlohmann::json& j, const std::shared_ptr<MaterialAttributeData>& v, MaterialAttributeType type, bool array)
{
    switch (type) {
    case MaterialAttributeType::Bool:
        if (!array) {
            to_json(j, *std::static_pointer_cast<BoolMaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<BoolArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Int:
        if (!array) {
            to_json(j, *std::static_pointer_cast<IntMaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<IntArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::UInt:
        if (!array) {
            to_json(j, *std::static_pointer_cast<UIntMaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<UIntArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Float:
        if (!array) {
            to_json(j, *std::static_pointer_cast<FloatMaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<FloatArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::BVec2:
        if (!array) {
            to_json(j, *std::static_pointer_cast<BVec2MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<BVec2ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::BVec3:
        if (!array) {
            to_json(j, *std::static_pointer_cast<BVec3MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<BVec3ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::BVec4:
        if (!array) {
            to_json(j, *std::static_pointer_cast<BVec4MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<BVec4ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::IVec2:
        if (!array) {
            to_json(j, *std::static_pointer_cast<IVec2MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<IVec2ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::IVec3:
        if (!array) {
            to_json(j, *std::static_pointer_cast<IVec3MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<IVec3ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::IVec4:
        if (!array) {
            to_json(j, *std::static_pointer_cast<IVec4MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<IVec4ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::UVec2:
        if (!array) {
            to_json(j, *std::static_pointer_cast<UVec2MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<UVec2ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::UVec3:
        if (!array) {
            to_json(j, *std::static_pointer_cast<UVec3MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<UVec3ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::UVec4:
        if (!array) {
            to_json(j, *std::static_pointer_cast<UVec4MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<UVec4ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Vec2:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Vec2MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Vec2ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Vec3:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Vec3MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Vec3ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Vec4:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Vec4MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Vec4ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Mat2x2:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Mat2x2MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Mat2x2ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Mat2x3:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Mat2x3MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Mat2x3ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Mat2x4:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Mat2x4MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Mat2x4ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Mat3x2:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Mat3x2MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Mat3x2ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Mat3x3:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Mat3x3MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Mat3x3ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Mat3x4:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Mat3x4MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Mat3x4ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Mat4x2:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Mat4x2MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Mat4x2ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Mat4x3:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Mat4x3MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Mat4x3ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Mat4x4:
        if (!array) {
            to_json(j, *std::static_pointer_cast<Mat4x4MaterialAttribute>(v));
        } else {
            to_json(j, *std::static_pointer_cast<Mat4x4ArrayMaterialAttribute>(v));
        }
        break;
    case MaterialAttributeType::Sampler2D:
        to_json(j, *std::static_pointer_cast<Sampler2DMaterialAttribute>(v));
        break;
    case MaterialAttributeType::Sampler2DMS:
        to_json(j, *std::static_pointer_cast<Sampler2DMSMaterialAttribute>(v));
        break;
    }
}

void from_json(
    const nlohmann::json& j, std::shared_ptr<MaterialAttributeData>& v, MaterialAttributeType type, bool array)
{
    switch (type) {
    case MaterialAttributeType::Bool: {
        if (!array) {
            auto ptr{ std::make_shared<BoolMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<BoolArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Int: {
        if (!array) {
            auto ptr{ std::make_shared<IntMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<IntArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::UInt: {
        if (!array) {
            auto ptr{ std::make_shared<UIntMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<UIntArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Float: {
        if (!array) {
            auto ptr{ std::make_shared<FloatMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<FloatArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::BVec2: {
        if (!array) {
            auto ptr{ std::make_shared<BVec2MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<BVec2ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::BVec3: {
        if (!array) {
            auto ptr{ std::make_shared<BVec3MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<BVec3ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::BVec4: {
        if (!array) {
            auto ptr{ std::make_shared<BVec4MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<BVec4ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::IVec2: {
        if (!array) {
            auto ptr{ std::make_shared<IVec2MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<IVec2ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::IVec3: {
        if (!array) {
            auto ptr{ std::make_shared<IVec3MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<IVec3ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::IVec4: {
        if (!array) {
            auto ptr{ std::make_shared<IVec4MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<IVec4ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::UVec2: {
        if (!array) {
            auto ptr{ std::make_shared<UVec2MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<UVec2ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::UVec3: {
        if (!array) {
            auto ptr{ std::make_shared<UVec3MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<UVec3ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::UVec4: {
        if (!array) {
            auto ptr{ std::make_shared<UVec4MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<UVec4ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Vec2: {
        if (!array) {
            auto ptr{ std::make_shared<Vec2MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Vec2ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Vec3: {
        if (!array) {
            auto ptr{ std::make_shared<Vec3MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Vec3ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Vec4: {
        if (!array) {
            auto ptr{ std::make_shared<Vec4MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Vec4ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Mat2x2: {
        if (!array) {
            auto ptr{ std::make_shared<Mat2x2MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Mat2x2ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Mat2x3: {
        if (!array) {
            auto ptr{ std::make_shared<Mat2x3MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Mat2x3ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Mat2x4: {
        if (!array) {
            auto ptr{ std::make_shared<Mat2x4MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Mat2x4ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Mat3x2: {
        if (!array) {
            auto ptr{ std::make_shared<Mat3x2MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Mat3x2ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Mat3x3: {
        if (!array) {
            auto ptr{ std::make_shared<Mat3x3MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Mat3x3ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Mat3x4: {
        if (!array) {
            auto ptr{ std::make_shared<Mat3x4MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Mat3x4ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Mat4x2: {
        if (!array) {
            auto ptr{ std::make_shared<Mat4x2MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Mat4x2ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Mat4x3: {
        if (!array) {
            auto ptr{ std::make_shared<Mat4x3MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Mat4x3ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Mat4x4: {
        if (!array) {
            auto ptr{ std::make_shared<Mat4x4MaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        } else {
            auto ptr{ std::make_shared<Mat4x4ArrayMaterialAttribute>() };
            from_json(j, *ptr);
            v = std::static_pointer_cast<MaterialAttributeData>(ptr);
        }
    } break;
    case MaterialAttributeType::Sampler2D: {
        auto ptr{ std::make_shared<Sampler2DMaterialAttribute>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<MaterialAttributeData>(ptr);
    } break;
    case MaterialAttributeType::Sampler2DMS: {
        auto ptr{ std::make_shared<Sampler2DMSMaterialAttribute>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<MaterialAttributeData>(ptr);
    } break;
    }
}

void to_json(nlohmann::json& j, const MaterialAttribute& v)
{
    j[MaterialAttribute::typeJson] = v.type;
    j[MaterialAttribute::isArrayJson] = v.isArray;

    nlohmann::json dataJson{};
    to_json(dataJson, v.data, v.type, v.isArray);

    j[MaterialAttribute::dataJson] = dataJson;
}

void from_json(const nlohmann::json& j, MaterialAttribute& v)
{
    j[MaterialAttribute::typeJson].get_to(v.type);
    j[MaterialAttribute::isArrayJson].get_to(v.isArray);

    from_json(j[MaterialAttribute::dataJson], v.data, v.type, v.isArray);
}

void to_json(nlohmann::json& j, const Sampler2DMaterialAttribute& v)
{
    j[Sampler2DMaterialAttribute::assetJson] = v.asset;
    j[Sampler2DMaterialAttribute::slotJson] = v.slot;
}

void from_json(const nlohmann::json& j, Sampler2DMaterialAttribute& v)
{
    j[Sampler2DMaterialAttribute::assetJson].get_to(v.asset);
    j[Sampler2DMaterialAttribute::slotJson].get_to(v.slot);
}

void to_json(nlohmann::json& j, const NoopCommand& v) { j[NoopCommand::counter_json] = v.counter; }

void from_json(const nlohmann::json& j, NoopCommand& v) { j[NoopCommand::counter_json].get_to(v.counter); }

void to_json(nlohmann::json& j, const DrawCommand& v) { j[DrawCommand::cuboid_idx_json] = v.cuboid_idx; }

void from_json(const nlohmann::json& j, DrawCommand& v) { j[DrawCommand::cuboid_idx_json].get_to(v.cuboid_idx); }

void to_json(nlohmann::json& j, const DrawMultipleCommand& v)
{
    j[DrawMultipleCommand::cuboid_indices_json] = v.cuboid_indices;
}

void from_json(const nlohmann::json& j, DrawMultipleCommand& v)
{
    j[DrawMultipleCommand::cuboid_indices_json].get_to(v.cuboid_indices);
}

void to_json([[maybe_unused]] nlohmann::json& j, [[maybe_unused]] const DeleteCommand& v) {}

void from_json([[maybe_unused]] const nlohmann::json& j, [[maybe_unused]] DeleteCommand& v) {}

void to_json(nlohmann::json& j, const DeleteMultipleCommand& v) { j[DeleteMultipleCommand::counter_json] = v.counter; }

void from_json(const nlohmann::json& j, DeleteMultipleCommand& v)
{
    j[DeleteMultipleCommand::counter_json].get_to(v.counter);
}

void to_json(nlohmann::json& j, const CuboidCommand& v)
{
    j[CuboidCommand::type_json] = v.type;

    switch (v.type) {
    case CuboidCommandType::NOOP:
        j[CuboidCommand::command_json] = std::get<NoopCommand>(v.command);
        break;
    case CuboidCommandType::DRAW:
        j[CuboidCommand::command_json] = std::get<DrawCommand>(v.command);
        break;
    case CuboidCommandType::DRAW_MULTIPLE:
        j[CuboidCommand::command_json] = std::get<DrawMultipleCommand>(v.command);
        break;
    case CuboidCommandType::DELETE:
        j[CuboidCommand::command_json] = std::get<DeleteCommand>(v.command);
        break;
    case CuboidCommandType::DELETE_MULTIPLE:
        j[CuboidCommand::command_json] = std::get<DeleteMultipleCommand>(v.command);
        break;
    }
}

void from_json(const nlohmann::json& j, CuboidCommand& v)
{
    j[CuboidCommand::type_json].get_to(v.type);

    switch (v.type) {
    case CuboidCommandType::NOOP:
        v.command = j[CuboidCommand::command_json].get<NoopCommand>();
        break;
    case CuboidCommandType::DRAW:
        v.command = j[CuboidCommand::command_json].get<DrawCommand>();
        break;
    case CuboidCommandType::DRAW_MULTIPLE:
        v.command = j[CuboidCommand::command_json].get<DrawMultipleCommand>();
        break;
    case CuboidCommandType::DELETE:
        v.command = j[CuboidCommand::command_json].get<DeleteCommand>();
        break;
    case CuboidCommandType::DELETE_MULTIPLE:
        v.command = j[CuboidCommand::command_json].get<DeleteMultipleCommand>();
        break;
    }
}

void to_json(nlohmann::json& j, const CompositionOperation& v)
{
    j[CompositionOperation::scaleJson] = v.scale;
    j[CompositionOperation::positionJson] = v.position;
    j[CompositionOperation::sourceTextureJson] = v.sourceTexture;
    j[CompositionOperation::targetJson] = v.target;
    j[CompositionOperation::shaderJson] = v.shader;
    j[CompositionOperation::idJson] = v.id;
    j[CompositionOperation::draggableJson] = v.draggable;
}

void from_json(const nlohmann::json& j, CompositionOperation& v)
{
    j[CompositionOperation::scaleJson].get_to(v.scale);
    j[CompositionOperation::positionJson].get_to(v.position);
    j[CompositionOperation::sourceTextureJson].get_to(v.sourceTexture);
    j[CompositionOperation::targetJson].get_to(v.target);
    j[CompositionOperation::shaderJson].get_to(v.shader);
    j[CompositionOperation::idJson].get_to(v.id);
    j[CompositionOperation::draggableJson].get_to(v.draggable);
}

void to_json(nlohmann::json& j, const CopyOperation& v)
{
    j[CopyOperation::sourceJson] = v.source;
    j[CopyOperation::destinationJson] = v.destination;
    j[CopyOperation::flagsJson] = v.flags;
    j[CopyOperation::filterJson] = v.filter;
}

void from_json(const nlohmann::json& j, CopyOperation& v)
{
    j[CopyOperation::sourceJson].get_to(v.source);
    j[CopyOperation::destinationJson].get_to(v.destination);
    j[CopyOperation::flagsJson].get_to(v.flags);
    j[CopyOperation::filterJson].get_to(v.filter);
}

void to_json(nlohmann::json& j, const LegendGUIEntry& v)
{
    j[LegendGUIEntry::type_json] = v.type;
    std::visit([&](auto& entry) { j[LegendGUIEntry::entry_json] = entry; }, v.entry);
}

void from_json(const nlohmann::json& j, LegendGUIEntry& v)
{
    j[LegendGUIEntry::type_json].get_to(v.type);
    switch (v.type) {
    case LegendGUIEntryType::ColorEntry: {
        LegendGUIColorEntry entry;
        from_json(j[LegendGUIEntry::entry_json], entry);
        v.entry = std::move(entry);
        break;
    }
    }
}

void to_json(nlohmann::json& j, const CanvasEntry& v)
{
    j[CanvasEntry::type_json] = v.type;
    j[CanvasEntry::size_json] = v.size;
    j[CanvasEntry::position_json] = v.position;
    std::visit([&](auto& entry) { j[CanvasEntry::gui_data_json] = entry; }, v.gui_data);
}

void from_json(const nlohmann::json& j, CanvasEntry& v)
{
    j[CanvasEntry::type_json].get_to(v.type);
    j[CanvasEntry::size_json].get_to(v.size);
    j[CanvasEntry::position_json].get_to(v.position);
    switch (v.type) {
    case CanvasEntryType::LegendGUI: {
        LegendGUI gui;
        from_json(j[CanvasEntry::gui_data_json], gui);
        v.gui_data = std::move(gui);
        break;
    }
    }
}

}
