#pragma once

#include <optional>
#include <vector>

#include <visualizer/SubScene.hpp>
#include <visualizer/VisualizerConfiguration.hpp>

namespace Visualizer {

struct Scene {
    std::vector<SubScene> m_subScenes;
};

std::optional<Scene> loadScene(const VisualizerConfiguration& config);

void tick(Scene& scene);
void draw(const Scene& scene);

}
