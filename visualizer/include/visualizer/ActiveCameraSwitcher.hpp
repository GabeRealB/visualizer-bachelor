#pragma once

#include <vector>

#include <visualizer/Entity.hpp>

namespace Visualizer {

struct ActiveCameraSwitcher {
    std::vector<Entity> cameras;
    std::size_t current;
};

}