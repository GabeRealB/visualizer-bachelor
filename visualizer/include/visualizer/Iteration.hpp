#pragma once

#include <cstdlib>
#include <vector>

#include <glm/glm.hpp>

namespace Visualizer {

struct Iteration {
    std::vector<glm::vec3> positions;
    std::size_t ticksPerIteration;
    std::size_t index;
};

}
