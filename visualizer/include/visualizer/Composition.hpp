#pragma once

#include <memory>

#include <visualizer/Framebuffer.hpp>
#include <visualizer/Texture.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

struct CompositionOperation {
    std::size_t id;
    Material material;
    Transform transform;
    std::vector<std::shared_ptr<Texture2D>> source;
    std::shared_ptr<Framebuffer> destination;
};

struct Composition {
    std::vector<CompositionOperation> operations;
};

struct CopyOperation {
    std::shared_ptr<Framebuffer> source;
    std::shared_ptr<Framebuffer> destination;
    std::vector<FramebufferCopyFlags> flags;
    FramebufferCopyFilter filter;
};

struct Copy {
    std::vector<CopyOperation> operations;
};

struct BoundingBox {
    std::size_t id;
    double xStart;
    double yStart;
    double xEnd;
    double yEnd;
};

struct Draggable {
    std::vector<BoundingBox> boxes;
};

}
