#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

#include <visualizer/ComponentManager.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/System.hpp>

namespace Visualizer {

class CameraMovementSystem : public System {
public:
    CameraMovementSystem();

    void run(void* data) final;
    void initialize() final;
    void terminate() final;

private:
    double m_mouseX;
    double m_mouseY;
    double m_currentTime;
    float m_movementSpeed;
    float m_rotationSpeed;
    float m_movementMultiplier;
    EntityQuery m_cameraQuery;
    std::shared_ptr<ComponentManager> m_componentManager;
};

}