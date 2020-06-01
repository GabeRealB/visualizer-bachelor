#pragma once

#include <glm/glm.hpp>

namespace Visualizer {

struct CubeTickInfo {
    glm::ivec3 limits;
    glm::ivec3 order;
    glm::ivec3 currentIter;
    glm::vec3 startPos;
    int tickRate;
    int currentTick;
    bool canTick;
};

}
