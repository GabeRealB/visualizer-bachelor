#pragma once

#include <vector>

#include <visualizer/Camera.hpp>
#include <visualizer/DrawableCube.hpp>

namespace Visualizer {

class SubScene {
public:
    SubScene() = default;

    Camera& camera();

    void addDrawableCube(DrawableCube& cube);
    std::vector<DrawableCube>& drawableCubes();

    void tick();
    void draw();

private:
    Camera m_camera;
    std::vector<DrawableCube> m_drawableCubes;
};

}
