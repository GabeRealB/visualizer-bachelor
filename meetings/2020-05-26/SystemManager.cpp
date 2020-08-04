auto systemManager{ world - getManager<SystemManager>() };

systemManager->addSystem<CubeInitialisationSystem>("init");
...

    systemManager->addSystem<CubeMovementSystem>("tick");
systemManager->addSystem<CameraMovementSystem>("tick");
...

    systemManager->addSystem<CubeDrawSystem>("draw");
...

    systemManager->run("init");

while (true) {
    systemManager->run("tick");
    systemManager->run("draw");
}