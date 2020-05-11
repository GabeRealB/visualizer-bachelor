#pragma once

#include <glm/glm.hpp>

namespace Visualizer {

struct CubeTickInfo {
    const glm::ivec3 limits;
    const glm::ivec3 order;
    glm::ivec3 currentIter;
    const int tickRate;
    int currentTick;
};

}
