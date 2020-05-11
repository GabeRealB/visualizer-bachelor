#pragma once
#include <visualizer/CubeTickInfo.hpp>
#include <visualizer/Transform.hpp>

namespace Visualizer {

void tick(CubeTickInfo& cubeInfo);
void tick(const CubeTickInfo&, Transform& transformation);

}
