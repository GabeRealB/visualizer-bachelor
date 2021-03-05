#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

namespace Visconfig::Components {

enum class ComponentType {
    Cube,
    Mesh,
    Parent,
    Material,
    Layer,
    Transform,
    ImplicitIteration,
    ExplicitIteration,
    EntityActivation,
    MeshIteration,
    ExplicitHeterogeneousIteration,
    CuboidCommandList,
    Camera,
    FreeFlyCamera,
    FixedCamera,
    CameraSwitcher,
    Composition,
    Copy,
    Canvas,
};

struct ComponentData {
};

void to_json(nlohmann::json& j, const std::shared_ptr<ComponentData>& v, ComponentType type);
void from_json(const nlohmann::json& j, std::shared_ptr<ComponentData>& v, ComponentType type);

struct CubeComponent : public ComponentData {
};

struct MeshComponent : public ComponentData {
    std::string asset;
};

struct ParentComponent : public ComponentData {
    std::size_t id;
};

enum class MaterialAttributeType {
    Bool,
    Int,
    UInt,
    Float,
    BVec2,
    BVec3,
    BVec4,
    IVec2,
    IVec3,
    IVec4,
    UVec2,
    UVec3,
    UVec4,
    Vec2,
    Vec3,
    Vec4,
    Mat2x2,
    Mat2x3,
    Mat2x4,
    Mat3x2,
    Mat3x3,
    Mat3x4,
    Mat4x2,
    Mat4x3,
    Mat4x4,
    Sampler2D,
    Sampler2DMS,
};

struct MaterialAttributeData {
};

template <typename T> struct TMaterialAttribute : MaterialAttributeData {
    T value;

    static constexpr const char* value_json{ "value" };
};

template <typename T> struct TArrayMaterialAttribute : MaterialAttributeData {
    std::vector<T> value;

    static constexpr const char* value_json{ "value" };
};

struct Sampler2DMaterialAttribute : MaterialAttributeData {
    std::string asset;
    std::size_t slot;
};

struct Sampler2DMSMaterialAttribute : MaterialAttributeData {
    std::string asset;
    std::size_t slot;
};

using BoolMaterialAttribute = TMaterialAttribute<bool>;
using IntMaterialAttribute = TMaterialAttribute<std::int32_t>;
using UIntMaterialAttribute = TMaterialAttribute<std::uint32_t>;
using FloatMaterialAttribute = TMaterialAttribute<float>;
using BVec2MaterialAttribute = TMaterialAttribute<std::array<bool, 2>>;
using BVec3MaterialAttribute = TMaterialAttribute<std::array<bool, 3>>;
using BVec4MaterialAttribute = TMaterialAttribute<std::array<bool, 4>>;
using IVec2MaterialAttribute = TMaterialAttribute<std::array<std::int32_t, 2>>;
using IVec3MaterialAttribute = TMaterialAttribute<std::array<std::int32_t, 3>>;
using IVec4MaterialAttribute = TMaterialAttribute<std::array<std::int32_t, 4>>;
using UVec2MaterialAttribute = TMaterialAttribute<std::array<std::uint32_t, 2>>;
using UVec3MaterialAttribute = TMaterialAttribute<std::array<std::uint32_t, 3>>;
using UVec4MaterialAttribute = TMaterialAttribute<std::array<std::uint32_t, 4>>;
using Vec2MaterialAttribute = TMaterialAttribute<std::array<float, 2>>;
using Vec3MaterialAttribute = TMaterialAttribute<std::array<float, 3>>;
using Vec4MaterialAttribute = TMaterialAttribute<std::array<float, 4>>;
using Mat2x2MaterialAttribute = TMaterialAttribute<std::array<std::array<float, 2>, 2>>;
using Mat2x3MaterialAttribute = TMaterialAttribute<std::array<std::array<float, 3>, 2>>;
using Mat2x4MaterialAttribute = TMaterialAttribute<std::array<std::array<float, 4>, 2>>;
using Mat3x2MaterialAttribute = TMaterialAttribute<std::array<std::array<float, 2>, 3>>;
using Mat3x3MaterialAttribute = TMaterialAttribute<std::array<std::array<float, 3>, 3>>;
using Mat3x4MaterialAttribute = TMaterialAttribute<std::array<std::array<float, 4>, 3>>;
using Mat4x2MaterialAttribute = TMaterialAttribute<std::array<std::array<float, 2>, 4>>;
using Mat4x3MaterialAttribute = TMaterialAttribute<std::array<std::array<float, 3>, 4>>;
using Mat4x4MaterialAttribute = TMaterialAttribute<std::array<std::array<float, 4>, 4>>;

using BoolArrayMaterialAttribute = TArrayMaterialAttribute<bool>;
using IntArrayMaterialAttribute = TArrayMaterialAttribute<std::int32_t>;
using UIntArrayMaterialAttribute = TArrayMaterialAttribute<std::uint32_t>;
using FloatArrayMaterialAttribute = TArrayMaterialAttribute<float>;
using BVec2ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<bool, 2>>;
using BVec3ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<bool, 3>>;
using BVec4ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<bool, 4>>;
using IVec2ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::int32_t, 2>>;
using IVec3ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::int32_t, 3>>;
using IVec4ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::int32_t, 4>>;
using UVec2ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::uint32_t, 2>>;
using UVec3ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::uint32_t, 3>>;
using UVec4ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::uint32_t, 4>>;
using Vec2ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<float, 2>>;
using Vec3ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<float, 3>>;
using Vec4ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<float, 4>>;
using Mat2x2ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::array<float, 2>, 2>>;
using Mat2x3ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::array<float, 3>, 2>>;
using Mat2x4ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::array<float, 4>, 2>>;
using Mat3x2ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::array<float, 2>, 3>>;
using Mat3x3ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::array<float, 3>, 3>>;
using Mat3x4ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::array<float, 4>, 3>>;
using Mat4x2ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::array<float, 2>, 4>>;
using Mat4x3ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::array<float, 3>, 4>>;
using Mat4x4ArrayMaterialAttribute = TArrayMaterialAttribute<std::array<std::array<float, 4>, 4>>;

struct MaterialAttribute {
    MaterialAttributeType type;
    std::shared_ptr<MaterialAttributeData> data;
    bool isArray;

    static constexpr const char* typeJson{ "type" };
    static constexpr const char* dataJson{ "data" };
    static constexpr const char* isArrayJson{ "is_array" };
};

struct MaterialPass {
    std::string asset;
    std::unordered_map<std::string, MaterialAttribute> attributes;
};

struct MaterialComponent : public ComponentData {
    std::string pipeline;
    std::vector<MaterialPass> passes;
};

struct LayerComponent : public ComponentData {
    std::size_t mask;
};

struct TransformComponent : public ComponentData {
    float scale[3];
    float position[3];
    float rotation[3];
};

enum class IterationOrder { XYZ, XZY, YXZ, YZX, ZXY, ZYX };

struct ImplicitIterationComponent : public ComponentData {
    IterationOrder order;
    float startPos[3];
    std::size_t numIterations[3];
    std::size_t ticksPerIteration;
};

struct ExplicitIterationComponent : public ComponentData {
    std::vector<std::array<float, 3>> positions;
    std::size_t ticksPerIteration;
};

struct EntityActivationComponent : public ComponentData {
    std::size_t layer;
    std::vector<std::size_t> entities;
    std::vector<std::size_t> ticksPerIteration;
};

struct MeshIterationComponent : public ComponentData {
    std::array<std::size_t, 3> dimensions;
    std::vector<std::size_t> ticksPerIteration;
    std::vector<std::array<float, 3>> positions;
};

struct ExplicitHeterogeneousIterationComponent : public ComponentData {
    std::vector<std::size_t> ticksPerIteration;
    std::vector<std::array<float, 3>> scales;
    std::vector<std::array<float, 3>> positions;
};

enum class CuboidCommandType { NOOP, DRAW, DRAW_MULTIPLE, DELETE, DELETE_MULTIPLE };

struct NoopCommand {
    std::size_t counter;
};

struct DrawCommand {
    bool out_of_bounds;
    std::size_t cuboid_idx;
};

struct DrawMultipleCommand {
    std::vector<std::size_t> cuboid_indices;
    std::vector<std::size_t> out_of_bounds_indices;
};

struct DeleteCommand {
};

struct DeleteMultipleCommand {
    std::size_t counter;
};

struct CuboidCommand {
    CuboidCommandType type;
    std::variant<NoopCommand, DrawCommand, DrawMultipleCommand, DeleteCommand, DeleteMultipleCommand> command;

    static constexpr const char* type_json{ "type" };
    static constexpr const char* command_json{ "command" };
};

struct CuboidCommandListComponent : public ComponentData {
    bool global;
    bool draw_heatmap;
    std::size_t heatmap_stepping;
    std::array<int, 3> global_size;
    std::vector<CuboidCommand> commands;
    std::vector<std::tuple<std::array<int, 3>, std::array<int, 3>>> positions;
};

struct CameraComponent : public ComponentData {
    bool active;
    bool fixed;
    bool perspective;
    float fov;
    float far;
    float near;
    float aspect;
    float orthographicWidth;
    float orthographicHeight;
    std::bitset<64> layerMask;
    std::unordered_map<std::string, std::vector<std::string>> targets;
};

struct FreeFlyCameraComponent : public ComponentData {
};

struct FixedCameraComponent : public ComponentData {
    std::size_t focus;
    float distance;
    float horizontalAngle;
    float verticalAngle;
};

struct CameraSwitcherComponent : public ComponentData {
    std::vector<std::size_t> cameras;
    std::size_t active;
};

struct CompositionOperation {
    float scale[2];
    float position[2];
    std::vector<std::string> sourceTexture;
    std::string target;
    std::string shader;
    std::size_t id;
    bool draggable;
};

struct CompositionComponent : public ComponentData {
    std::vector<CompositionOperation> operations;
};

enum class CopyOperationFlag {
    Color,
    Depth,
    Stencil,
};

enum class CopyOperationFilter {
    Nearest,
    Linear,
};

struct CopyOperation {
    std::string source;
    std::string destination;
    std::vector<CopyOperationFlag> flags;
    CopyOperationFilter filter;
};

struct CopyComponent : public ComponentData {
    std::vector<CopyOperation> operations;
};

enum class CanvasEntryType { LegendGUI, CompositionGUI };

enum class LegendGUIEntryType { ColorEntry, ImageEntry };

struct LegendGUIImageEntry {
    bool absolute;
    std::string image;
    std::string description;
    std::array<float, 2> scaling;
};

struct LegendGUIColorEntry {
    std::size_t entity;
    std::size_t pass;
    std::string label;
    std::string description;
    std::string attribute;
};

struct LegendGUIEntry {
    LegendGUIEntryType type;
    std::variant<LegendGUIColorEntry, LegendGUIImageEntry> entry;

    static constexpr const char* type_json{ "type" };
    static constexpr const char* entry_json{ "entry" };
};

struct LegendGUI {
    std::vector<LegendGUIEntry> entries;
};

struct CompositionGUIWindow {
    std::string name;
    bool flip_vertical;
    std::string texture_name;
    std::array<float, 2> scaling;
    std::array<float, 2> position;
};

struct CompositionGUIGroup {
    bool transparent;
    std::vector<CompositionGUIWindow> windows;
};

struct CompositionGUI {
    std::map<std::string, CompositionGUIGroup> groups;
    std::vector<std::array<std::string, 2>> group_connections;
};

struct CanvasEntry {
    CanvasEntryType type;
    std::array<float, 2> size;
    std::array<float, 2> position;
    std::variant<LegendGUI, CompositionGUI> gui_data;

    static constexpr const char* type_json{ "type" };
    static constexpr const char* size_json{ "size" };
    static constexpr const char* position_json{ "position" };
    static constexpr const char* gui_data_json{ "gui_data" };
};

struct CanvasComponent : public ComponentData {
    std::vector<CanvasEntry> entries;
};

}

/* STD Extensions */

namespace nlohmann {
template <std::size_t N> struct adl_serializer<std::bitset<N>> {
    static void to_json(nlohmann::json& j, const std::bitset<N>& v)
    {
        if constexpr (N <= sizeof(unsigned long) * 8) {
            j = v.to_ulong();
        } else if constexpr (N <= sizeof(unsigned long long) * 8) {
            j = v.to_ullong();
        } else {
            j = v.to_string();
        }
    }

    static void from_json(const nlohmann::json& j, std::bitset<N>& v)
    {
        if constexpr (N <= sizeof(unsigned long) * 8) {
            v = std::bitset<N>{ j.get<unsigned long>() };
        } else if constexpr (N <= sizeof(unsigned long long) * 8) {
            v = std::bitset<N>{ j.get<unsigned long long>() };
        } else {
            v = std::bitset<N>{ j.get<std::string>() };
        }
    }
};
}

namespace Visconfig::Components {

/*Enums*/

NLOHMANN_JSON_SERIALIZE_ENUM(ComponentType,
    {
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
    })

NLOHMANN_JSON_SERIALIZE_ENUM(MaterialAttributeType,
    {
        { MaterialAttributeType::Bool, "bool" },
        { MaterialAttributeType::Int, "int" },
        { MaterialAttributeType::UInt, "uint" },
        { MaterialAttributeType::Float, "float" },
        { MaterialAttributeType::BVec2, "bvec2" },
        { MaterialAttributeType::BVec3, "bvec3" },
        { MaterialAttributeType::BVec4, "bvec4" },
        { MaterialAttributeType::IVec2, "ivec2" },
        { MaterialAttributeType::IVec3, "ivec3" },
        { MaterialAttributeType::IVec4, "ivec4" },
        { MaterialAttributeType::UVec2, "uvec2" },
        { MaterialAttributeType::UVec3, "uvec3" },
        { MaterialAttributeType::UVec4, "uvec4" },
        { MaterialAttributeType::Vec2, "vec2" },
        { MaterialAttributeType::Vec3, "vec3" },
        { MaterialAttributeType::Vec4, "vec4" },
        { MaterialAttributeType::Mat2x2, "mat2x2" },
        { MaterialAttributeType::Mat2x3, "mat2x3" },
        { MaterialAttributeType::Mat2x4, "mat2x4" },
        { MaterialAttributeType::Mat3x2, "mat3x2" },
        { MaterialAttributeType::Mat3x3, "mat3x3" },
        { MaterialAttributeType::Mat3x4, "mat3x4" },
        { MaterialAttributeType::Mat4x2, "mat4x2" },
        { MaterialAttributeType::Mat4x3, "mat4x3" },
        { MaterialAttributeType::Mat4x4, "mat4x4" },
        { MaterialAttributeType::Sampler2D, "sampler2D" },
        { MaterialAttributeType::Sampler2DMS, "sampler2DMS" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(IterationOrder,
    {
        { IterationOrder::XYZ, "xyz" },
        { IterationOrder::XZY, "xzy" },
        { IterationOrder::YXZ, "yxz" },
        { IterationOrder::YZX, "yzx" },
        { IterationOrder::ZXY, "zxy" },
        { IterationOrder::ZYX, "zyx" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(CuboidCommandType,
    {
        { CuboidCommandType::NOOP, "noop" },
        { CuboidCommandType::DRAW, "draw" },
        { CuboidCommandType::DRAW_MULTIPLE, "draw_multiple" },
        { CuboidCommandType::DELETE, "delete" },
        { CuboidCommandType::DELETE_MULTIPLE, "delete_multiple" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(CopyOperationFlag,
    {
        { CopyOperationFlag::Color, "color" },
        { CopyOperationFlag::Depth, "depth" },
        { CopyOperationFlag::Stencil, "stencil" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(CopyOperationFilter,
    {
        { CopyOperationFilter::Nearest, "nearest" },
        { CopyOperationFilter::Linear, "linear" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(CanvasEntryType,
    {
        { CanvasEntryType::LegendGUI, "legend_gui" },
        { CanvasEntryType::CompositionGUI, "composition_gui" },
    })

NLOHMANN_JSON_SERIALIZE_ENUM(LegendGUIEntryType,
    {
        { LegendGUIEntryType::ColorEntry, "color_entry" },
        { LegendGUIEntryType::ImageEntry, "image_entry" },
    })

/*Internal Structs*/

template <typename T> void to_json(nlohmann::json& j, const TMaterialAttribute<T>& v)
{
    j[TMaterialAttribute<T>::value_json] = v.value;
}

template <typename T> void from_json(const nlohmann::json& j, TMaterialAttribute<T>& v)
{
    j[TMaterialAttribute<T>::value_json].get_to(v.value);
}

template <typename T> void to_json(nlohmann::json& j, const TArrayMaterialAttribute<T>& v)
{
    j[TArrayMaterialAttribute<T>::value_json] = v.value;
}

template <typename T> void from_json(const nlohmann::json& j, TArrayMaterialAttribute<T>& v)
{
    j[TArrayMaterialAttribute<T>::value_json].get_to(v.value);
}

void to_json(
    nlohmann::json& j, const std::shared_ptr<MaterialAttributeData>& v, MaterialAttributeType type, bool array);
void from_json(
    const nlohmann::json& j, std::shared_ptr<MaterialAttributeData>& v, MaterialAttributeType type, bool array);

void to_json(nlohmann::json& j, const MaterialAttribute& v);
void from_json(const nlohmann::json& j, MaterialAttribute& v);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MaterialPass, asset, attributes)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Sampler2DMaterialAttribute, asset, slot)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Sampler2DMSMaterialAttribute, asset, slot)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NoopCommand, counter)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DrawCommand, out_of_bounds, cuboid_idx)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DrawMultipleCommand, cuboid_indices, out_of_bounds_indices)

inline void to_json(nlohmann::json&, const DeleteCommand&) {}
inline void from_json(const nlohmann::json&, DeleteCommand&) {}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeleteMultipleCommand, counter)

void to_json(nlohmann::json& j, const CuboidCommand& v);
void from_json(const nlohmann::json& j, CuboidCommand& v);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CompositionOperation, scale, position, sourceTexture, target, shader, id, draggable)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CopyOperation, source, destination, flags, filter)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LegendGUIImageEntry, absolute, image, description, scaling)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LegendGUIColorEntry, entity, pass, label, description, attribute)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CompositionGUIWindow, name, flip_vertical, texture_name, scaling, position)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CompositionGUIGroup, transparent, windows)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CompositionGUI, groups, group_connections)

void to_json(nlohmann::json& j, const LegendGUIEntry& v);
void from_json(const nlohmann::json& j, LegendGUIEntry& v);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LegendGUI, entries)

void to_json(nlohmann::json& j, const CanvasEntry& v);
void from_json(const nlohmann::json& j, CanvasEntry& v);

/*Components*/

inline void to_json(nlohmann::json&, const CubeComponent&) {}
inline void from_json(const nlohmann::json&, CubeComponent&) {}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MeshComponent, asset)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ParentComponent, id)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MaterialComponent, pipeline, passes)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LayerComponent, mask)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TransformComponent, scale, position, rotation)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ImplicitIterationComponent, order, startPos, numIterations, ticksPerIteration)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ExplicitIterationComponent, positions, ticksPerIteration)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityActivationComponent, layer, entities, ticksPerIteration)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MeshIterationComponent, dimensions, positions, ticksPerIteration)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ExplicitHeterogeneousIterationComponent, scales, positions, ticksPerIteration)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    CuboidCommandListComponent, global, draw_heatmap, heatmap_stepping, global_size, commands, positions)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraComponent, active, fixed, perspective, fov, far, near, aspect,
    orthographicWidth, orthographicHeight, layerMask, targets)

inline void to_json(nlohmann::json&, const FreeFlyCameraComponent&) {}
inline void from_json(const nlohmann::json&, FreeFlyCameraComponent&) {}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FixedCameraComponent, focus, distance, horizontalAngle, verticalAngle)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraSwitcherComponent, cameras, active)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CompositionComponent, operations)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CopyComponent, operations)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CanvasComponent, entries)

}