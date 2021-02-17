#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <visualizer/Framebuffer.hpp>
#include <visualizer/RenderLayer.hpp>

namespace Visualizer {

struct Camera {
    bool m_active;
    bool m_fixed;
    bool perspective;
    float fov;
    float far;
    float near;
    float aspect;
    float orthographicWidth;
    float orthographicHeight;
    RenderLayer m_visibleLayers;
    std::shared_ptr<Framebuffer> m_renderTarget;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Framebuffer>>> m_renderTargets;
};

}
