#pragma once

#include <vector>

#include <visualizer/SubScene.hpp>

namespace Visualizer {

class Scene {
public:
    Scene() = default;

    void addSubScene(SubScene&& subScene);
    std::vector<SubScene>& subScenes();

    void tick();
    void draw();

private:
    std::vector<SubScene> m_subScenes;
};

}
