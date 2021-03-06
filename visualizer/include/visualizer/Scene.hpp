#pragma once

#include <glad/glad.h>
#include <optional>
#include <vector>

#include <visconfig/Config.hpp>

#include <visualizer/World.hpp>

namespace Visualizer {

struct Scene {
    std::size_t activeWorld;
    std::vector<World> worlds;
};

Scene initialize_scene(const Visconfig::Config& config);

void tick(Scene& scene);
void draw(const Scene& scene);

}
