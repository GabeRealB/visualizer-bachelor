int main()
{
    // Welt mit EntityManager und ComponentManager schon vorhanden.

    auto archetypeTypes{ getTypeIds<double, int, float>() };
    auto archetype{ EntityArchetype::create<EntityArchetypeLayout::Sequential, double, int, float>() };

    entityManager->addEntities(100, archetype);

    auto componentQuery{ componentManager->queryComponents(archetype) };
    componentQuery.forEach<false, double&, float&>([](double& d, float& f) {
        d = 0.0;
        f = 0.0;
    });

    componentQuery.forEach<false, const double&, int&>([](const double& d, int& i) { i = d * 123543; });
}