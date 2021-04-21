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
    case MaterialAttributeType::Image2D:
        to_json(j, *std::static_pointer_cast<Image2DMaterialAttribute>(v));
        break;
    case MaterialAttributeType::Image2DMS:
        to_json(j, *std::static_pointer_cast<Image2DMSMaterialAttribute>(v));
        break;
    case MaterialAttributeType::ImageBuffer:
        to_json(j, *std::static_pointer_cast<ImageBufferMaterialAttribute>(v));
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
    case MaterialAttributeType::Image2D: {
        auto ptr{ std::make_shared<Image2DMaterialAttribute>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<MaterialAttributeData>(ptr);
    } break;
    case MaterialAttributeType::Image2DMS: {
        auto ptr{ std::make_shared<Image2DMSMaterialAttribute>() };
        from_json(j, *ptr);
        v = std::static_pointer_cast<MaterialAttributeData>(ptr);
    } break;
    case MaterialAttributeType::ImageBuffer: {
        auto ptr{ std::make_shared<ImageBufferMaterialAttribute>() };
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
    case LegendGUIEntryType::ImageEntry: {
        LegendGUIImageEntry entry;
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
    case CanvasEntryType::CompositionGUI: {
        CompositionGUI gui;
        from_json(j[CanvasEntry::gui_data_json], gui);
        v.gui_data = std::move(gui);
        break;
    }
    case CanvasEntryType::ConfigDumpGUI: {
        ConfigDumpGUI gui;
        from_json(j[CanvasEntry::gui_data_json], gui);
        v.gui_data = std::move(gui);
        break;
    }
    }
}

}
