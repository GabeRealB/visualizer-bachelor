#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <string>
#include <unordered_map>
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
    Camera,
    FreeFlyCamera,
    FixedCamera,
    CameraSwitcher,
    Composition,
    Copy,
};

struct ComponentData {
};

void to_json(nlohmann::json& j, const std::shared_ptr<ComponentData>& v, ComponentType type);
void from_json(const nlohmann::json& j, std::shared_ptr<ComponentData>& v, ComponentType type);

struct CubeComponent : public ComponentData {
};

struct MeshComponent : public ComponentData {
    std::string asset;

    static constexpr const char* assetJson{ "asset" };
};

struct ParentComponent : public ComponentData {
    std::size_t id;

    static constexpr const char* idJson{ "id" };
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
};

struct MaterialAttributeData {
};

template <typename T> struct TMaterialAttribute : MaterialAttributeData {
    T value;

    static constexpr const char* valueJson{ "value" };
};

template <typename T> struct TArrayMaterialAttribute : MaterialAttributeData {
    std::vector<T> value;

    static constexpr const char* valueJson{ "value" };
};

struct Sampler2DMaterialAttribute : MaterialAttributeData {
    std::string asset;
    std::size_t slot;

    static constexpr const char* assetJson{ "asset" };
    static constexpr const char* slotJson{ "slot" };
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

struct MaterialComponent : public ComponentData {
    std::string asset;
    std::unordered_map<std::string, MaterialAttribute> attributes;

    static constexpr const char* assetJson{ "asset" };
    static constexpr const char* attributesJson{ "attributes" };
};

struct LayerComponent : public ComponentData {
    std::size_t mask;

    static constexpr const char* maskJson{ "mask" };
};

struct TransformComponent : public ComponentData {
    float scale[3];
    float position[3];
    float rotation[3];

    static constexpr const char* scaleJson{ "scale" };
    static constexpr const char* positionJson{ "position" };
    static constexpr const char* rotationJson{ "rotation" };
};

enum class IterationOrder { XYZ, XZY, YXZ, YZX, ZXY, ZYX };

struct ImplicitIterationComponent : public ComponentData {
    IterationOrder order;
    float startPos[3];
    std::size_t numIterations[3];
    std::size_t ticksPerIteration;

    static constexpr const char* orderJson{ "order" };
    static constexpr const char* startPosJson{ "start_position" };
    static constexpr const char* numIterationsJson{ "num_iterations" };
    static constexpr const char* ticksPerIterationJson{ "ticks_per_iteration" };
};

struct ExplicitIterationComponent : public ComponentData {
    std::vector<std::array<float, 3>> positions;
    std::size_t ticksPerIteration;

    static constexpr const char* positionsJson{ "positions" };
    static constexpr const char* ticksPerIterationJson{ "ticks_per_iteration" };
};

struct EntityActivationComponent : public ComponentData {
    std::size_t layer;
    std::vector<std::size_t> entities;
    std::vector<std::size_t> ticksPerIteration;

    static constexpr const char* layerJson{ "layer" };
    static constexpr const char* entitiesJson{ "entities" };
    static constexpr const char* ticksPerIterationJson{ "ticks_per_iteration" };
};

struct MeshIterationComponent : public ComponentData {
    std::array<std::size_t, 3> dimensions;
    std::vector<std::size_t> ticksPerIteration;
    std::vector<std::array<float, 3>> positions;

    static constexpr const char* dimensionsJson{ "dimensions" };
    static constexpr const char* positionsJson{ "positions" };
    static constexpr const char* ticksPerIterationJson{ "ticks_per_iteration" };
};

struct ExplicitHeterogeneousIterationComponent : public ComponentData {
    std::vector<std::size_t> ticksPerIteration;
    std::vector<std::array<float, 3>> scales;
    std::vector<std::array<float, 3>> positions;

    static constexpr const char* scalesJson{ "scales" };
    static constexpr const char* positionsJson{ "positions" };
    static constexpr const char* ticksPerIterationJson{ "ticks_per_iteration" };
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
    std::unordered_map<std::string, std::string> targets;

    static constexpr const char* activeJson{ "active" };
    static constexpr const char* fixedJson{ "fixed" };
    static constexpr const char* perspectiveJson{ "perspective" };
    static constexpr const char* fovJson{ "fov" };
    static constexpr const char* farJson{ "far" };
    static constexpr const char* nearJson{ "near" };
    static constexpr const char* aspectJson{ "aspect" };
    static constexpr const char* orthographicWidthJson{ "orthographic_width" };
    static constexpr const char* orthographicHeightJson{ "orthographic_height" };
    static constexpr const char* layerMaskJson{ "layer_mask" };
    static constexpr const char* targetsJson{ "targets" };
};

struct FreeFlyCameraComponent : public ComponentData {
};

struct FixedCameraComponent : public ComponentData {
    std::size_t focus;
    float distance;
    float horizontalAngle;
    float verticalAngle;

    static constexpr const char* focusJson{ "focus" };
    static constexpr const char* distanceJson{ "distance" };
    static constexpr const char* horizontalAngleJson{ "horizontal_angle" };
    static constexpr const char* verticalAngleJson{ "vertical_angle" };
};

struct CameraSwitcherComponent : public ComponentData {
    std::vector<std::size_t> cameras;
    std::size_t active;

    static constexpr const char* camerasJson{ "cameras" };
    static constexpr const char* activeJson{ "active" };
};

struct CompositionOperation {
    float scale[2];
    float position[2];
    std::vector<std::string> sourceTexture;
    std::string target;
    std::string shader;
    std::size_t id;
    bool draggable;

    static constexpr const char* scaleJson{ "scale" };
    static constexpr const char* positionJson{ "position" };
    static constexpr const char* sourceTextureJson{ "source_texture" };
    static constexpr const char* targetJson{ "target" };
    static constexpr const char* shaderJson{ "shader" };
    static constexpr const char* idJson{ "id" };
    static constexpr const char* draggableJson{ "draggable" };
};

struct CompositionComponent : public ComponentData {
    std::vector<CompositionOperation> operations;

    static constexpr const char* operationsJson{ "operations" };
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

    static constexpr const char* sourceJson{ "source" };
    static constexpr const char* destinationJson{ "destination" };
    static constexpr const char* flagsJson{ "flags" };
    static constexpr const char* filterJson{ "filter" };
};

struct CopyComponent : public ComponentData {
    std::vector<CopyOperation> operations;

    static constexpr const char* operationsJson{ "operations" };
};

/*Enums*/

void to_json(nlohmann::json& j, const ComponentType& v);
void from_json(const nlohmann::json& j, ComponentType& v);

void to_json(nlohmann::json& j, const MaterialAttributeType& v);
void from_json(const nlohmann::json& j, MaterialAttributeType& v);

void to_json(nlohmann::json& j, const IterationOrder& v);
void from_json(const nlohmann::json& j, IterationOrder& v);

void to_json(nlohmann::json& j, const CopyOperationFlag& v);
void from_json(const nlohmann::json& j, CopyOperationFlag& v);

void to_json(nlohmann::json& j, const CopyOperationFilter& v);
void from_json(const nlohmann::json& j, CopyOperationFilter& v);

/*Components*/

void to_json(nlohmann::json& j, const CubeComponent& v);
void from_json(const nlohmann::json& j, CubeComponent& v);

void to_json(nlohmann::json& j, const MeshComponent& v);
void from_json(const nlohmann::json& j, MeshComponent& v);

void to_json(nlohmann::json& j, const ParentComponent& v);
void from_json(const nlohmann::json& j, ParentComponent& v);

void to_json(nlohmann::json& j, const MaterialComponent& v);
void from_json(const nlohmann::json& j, MaterialComponent& v);

void to_json(nlohmann::json& j, const LayerComponent& v);
void from_json(const nlohmann::json& j, LayerComponent& v);

void to_json(nlohmann::json& j, const TransformComponent& v);
void from_json(const nlohmann::json& j, TransformComponent& v);

void to_json(nlohmann::json& j, const ImplicitIterationComponent& v);
void from_json(const nlohmann::json& j, ImplicitIterationComponent& v);

void to_json(nlohmann::json& j, const ExplicitIterationComponent& v);
void from_json(const nlohmann::json& j, ExplicitIterationComponent& v);

void to_json(nlohmann::json& j, const EntityActivationComponent& v);
void from_json(const nlohmann::json& j, EntityActivationComponent& v);

void to_json(nlohmann::json& j, const MeshIterationComponent& v);
void from_json(const nlohmann::json& j, MeshIterationComponent& v);

void to_json(nlohmann::json& j, const ExplicitHeterogeneousIterationComponent& v);
void from_json(const nlohmann::json& j, ExplicitHeterogeneousIterationComponent& v);

void to_json(nlohmann::json& j, const CameraComponent& v);
void from_json(const nlohmann::json& j, CameraComponent& v);

void to_json(nlohmann::json& j, const FreeFlyCameraComponent& v);
void from_json(const nlohmann::json& j, FreeFlyCameraComponent& v);

void to_json(nlohmann::json& j, const FixedCameraComponent& v);
void from_json(const nlohmann::json& j, FixedCameraComponent& v);

void to_json(nlohmann::json& j, const CameraSwitcherComponent& v);
void from_json(const nlohmann::json& j, CameraSwitcherComponent& v);

void to_json(nlohmann::json& j, const CompositionComponent& v);
void from_json(const nlohmann::json& j, CompositionComponent& v);

void to_json(nlohmann::json& j, const CopyComponent& v);
void from_json(const nlohmann::json& j, CopyComponent& v);

/*Internal Structs*/

template <typename T> void to_json(nlohmann::json& j, const TMaterialAttribute<T>& v)
{
    j[TMaterialAttribute<T>::valueJson] = v.value;
}

template <typename T> void from_json(const nlohmann::json& j, TMaterialAttribute<T>& v)
{
    j[TMaterialAttribute<T>::valueJson].get_to(v.value);
}

template <typename T> void to_json(nlohmann::json& j, const TArrayMaterialAttribute<T>& v)
{
    j[TArrayMaterialAttribute<T>::valueJson] = v.value;
}

template <typename T> void from_json(const nlohmann::json& j, TArrayMaterialAttribute<T>& v)
{
    j[TArrayMaterialAttribute<T>::valueJson].get_to(v.value);
}

void to_json(
    nlohmann::json& j, const std::shared_ptr<MaterialAttributeData>& v, MaterialAttributeType type, bool array);
void from_json(
    const nlohmann::json& j, std::shared_ptr<MaterialAttributeData>& v, MaterialAttributeType type, bool array);

void to_json(nlohmann::json& j, const MaterialAttribute& v);
void from_json(const nlohmann::json& j, MaterialAttribute& v);

void to_json(nlohmann::json& j, const Sampler2DMaterialAttribute& v);
void from_json(const nlohmann::json& j, Sampler2DMaterialAttribute& v);

void to_json(nlohmann::json& j, const CompositionOperation& v);
void from_json(const nlohmann::json& j, CompositionOperation& v);

void to_json(nlohmann::json& j, const CopyOperation& v);
void from_json(const nlohmann::json& j, CopyOperation& v);

}