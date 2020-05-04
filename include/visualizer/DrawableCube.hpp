#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec4.hpp>
#include <memory>

#include <visualizer/Mesh.hpp>
#include <visualizer/VisualizerConfiguration.hpp>

namespace Visualizer {

struct Transformation {
    glm::vec4 position;
    glm::quat rotation;
    glm::vec3 scale;

    glm::mat4 model();
};

class CubeInfo {
public:
    CubeInfo(CubeInfo* parent, glm::ivec3 order, glm::ivec3 limits);

    void tick();

    glm::ivec3 currentIter();

private:
    glm::ivec3 const m_limits;
    glm::ivec3 m_currentIter;
    glm::ivec3 const m_order;
    CubeInfo* const m_parent;
};

class DrawableCube {
public:
    DrawableCube(
        Transformation transformation, CubeInfo cubeInfo, std::shared_ptr<Mesh const>&& mesh);

    void tick();
    void draw();

private:
    CubeInfo m_cubeInfo;
    Transformation m_transformation;
    std::shared_ptr<Mesh const> const m_mesh;
};

}
