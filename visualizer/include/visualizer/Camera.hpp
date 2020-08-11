#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <visualizer/Framebuffer.hpp>
#include <visualizer/RenderLayer.hpp>

namespace Visualizer {

struct Camera {
    bool m_active;
    bool m_fixed;
    RenderLayer m_visibleLayers;
    std::shared_ptr<Framebuffer> m_renderTarget;
    std::unordered_map<std::string, std::shared_ptr<Framebuffer>> m_renderTargets;
};

}
