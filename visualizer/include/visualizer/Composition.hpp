#pragma once

#include <memory>

#include <visualizer/Framebuffer.hpp>
#include <visualizer/Texture.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

struct CompositionOperation {
    Transform transform;
    std::shared_ptr<Texture2D> source;
    std::shared_ptr<Framebuffer> destination;
};

struct Composition {
    std::vector<CompositionOperation> operations;
};

}
