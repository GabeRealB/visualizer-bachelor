#pragma once

#include <visualizer/Entity.hpp>

namespace Visualizer {

struct FixedCamera {
    Entity focus;
    float distance;
    float horizontalAngle;
    float verticalAngle;
};

}
