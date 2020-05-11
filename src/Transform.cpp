#include <visualizer/Transform.hpp>

namespace Visualizer {

glm::mat4 getModelMatrix(const Transform& transform)
{
    auto translation{ glm::translate(glm::mat4{ 1.0f }, transform.position) };
    auto rotation{ glm::toMat4(transform.rotation) };
    auto scale = glm::scale(glm::mat4{ 1.0f }, transform.scale);

    return translation * rotation * scale;
}

}