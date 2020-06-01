#include <visualizer/Systems.hpp>

namespace Visualizer {

void tick(CubeTickInfo& cubeInfo)
{
    if (!cubeInfo.canTick || ++cubeInfo.currentTick % cubeInfo.tickRate != 0) {
        return;
    } else {
        cubeInfo.currentTick = 0;
    }

    if (cubeInfo.currentIter[cubeInfo.order[0]] < cubeInfo.limits[cubeInfo.order[0]]) {
        cubeInfo.currentIter[cubeInfo.order[0]]++;
    } else {
        cubeInfo.currentIter[cubeInfo.order[0]] = 0;
        if (cubeInfo.currentIter[cubeInfo.order[1]] < cubeInfo.limits[cubeInfo.order[1]]) {
            cubeInfo.currentIter[cubeInfo.order[1]]++;
        } else {
            cubeInfo.currentIter[cubeInfo.order[1]] = 0;
            if (cubeInfo.currentIter[cubeInfo.order[2]] < cubeInfo.limits[cubeInfo.order[2]]) {
                cubeInfo.currentIter[cubeInfo.order[2]]++;
            } else {
                cubeInfo.currentIter[cubeInfo.order[2]] = 0;
            }
        }
    }
}

void tick(const CubeTickInfo& cubeInfo, Transform& transform)
{
    auto posX{ transform.scale.x * cubeInfo.currentIter.x };
    auto posY{ -transform.scale.y * cubeInfo.currentIter.y };
    auto posZ{ transform.scale.z * cubeInfo.currentIter.z };
    transform.position = cubeInfo.startPos + glm::vec3{ posX, posY, posZ };
}

}