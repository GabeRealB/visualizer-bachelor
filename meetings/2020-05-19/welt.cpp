int main()
{
    World world{};

    auto componentManager{ world.addManager<ComponentManager>() };
    auto entityManager{ world.addManager<EntityManager>() };

    assert(componentManager == world.getManager<ComponentManager>());
    assert(entityManager == world.getManager<EntityManager>());
}