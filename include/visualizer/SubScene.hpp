#pragma once

#include <vector>

#include <visualizer/Camera.hpp>
#include <visualizer/SceneObject.hpp>
#include <visualizer/Systems.hpp>

namespace Visualizer {

struct SubScene {
    Camera m_camera;
    std::vector<SceneObject> m_Objects;
};

void tick(SubScene& subScene);
void draw(const SubScene& subScene);

}
