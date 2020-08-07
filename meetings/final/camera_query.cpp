auto camera_query = EntityQuery{}.with<Camera>();
auto free_camera_query = EntityQuery{}.with<Camera, FreeCamera>();
auto fixed_camera_query = EntityQuery{}.with<Camera, FixedCamera>();
... query.query(componentManager)
    .filter<Camera>([](const Camera* camera) -> bool { return camera->active; })
    .forEach<...>([&](...) { ... });