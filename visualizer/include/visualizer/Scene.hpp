#pragma once

#include <glad/glad.h>
#include <optional>
#include <vector>

#include <visualizer/VisualizerConfiguration.hpp>
#include <visualizer/World.hpp>

namespace Visualizer {

struct Scene {
    std::size_t activeWorld;
    std::vector<World> worlds;
};

std::optional<Scene> loadScene(const VisualizerConfiguration& config);

void tick(Scene& scene);
void draw(const Scene& scene);

}
