#pragma once

#include <array>
#include <cstdlib>
#include <vector>

#include <glm/glm.hpp>

#include <visualizer/Entity.hpp>
#include <visualizer/RenderLayer.hpp>

namespace Visualizer {

struct HomogeneousIteration {
    std::vector<glm::vec3> positions;
    std::size_t ticksPerIteration;
    std::size_t index;
    std::size_t tick;
};

struct MeshIteration {
    std::vector<bool> grid;
    std::vector<glm::u64vec3> positions;
    std::array<std::size_t, 3> dimensions;
    std::vector<std::size_t> ticksPerIteration;
    std::size_t index;
    std::size_t tick;
    bool initialized;
};

struct EntityActivation {
    RenderLayer layer;
    std::vector<Entity> entities;
    std::vector<std::size_t> ticksPerIteration;
    std::size_t index;
    std::size_t tick;
};

struct HeterogeneousIteration {
    std::vector<glm::vec3> scales;
    std::vector<glm::vec3> positions;
    std::vector<std::size_t> ticksPerIteration;
    std::size_t index;
    std::size_t tick;
};

}
