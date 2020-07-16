#pragma once

#include <memory>

#include <visualizer/Framebuffer.hpp>
#include <visualizer/RenderLayer.hpp>

namespace Visualizer {

struct Camera {
    bool m_active;
    RenderLayer m_visibleLayers;
    std::shared_ptr<Framebuffer> m_renderTarget;
};

}
