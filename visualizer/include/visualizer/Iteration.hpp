#pragma once

#include <cstdlib>
#include <vector>

#include <glm/glm.hpp>

namespace Visualizer {

struct HomogeneousIteration {
    std::vector<glm::vec3> positions;
    std::size_t ticksPerIteration;
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
