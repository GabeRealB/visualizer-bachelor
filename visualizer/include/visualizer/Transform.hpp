#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Visualizer {

struct Transform {
    glm::quat rotation;
    glm::vec3 position;
    glm::vec3 scale;
};

glm::mat4 getModelMatrix(const Transform& transform);

}