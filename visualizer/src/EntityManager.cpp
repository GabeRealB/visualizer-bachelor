#include <visualizer/EntityManager.hpp>

#include <utility>

namespace Visualizer {

EntityManager::EntityManager()
    : m_lastEntityId{ 0 }
    , m_freeEntities{}
    , m_entities{}
{
}

EntityManager::EntityManager(EntityManager&& other) noexcept
    : m_lastEntityId{ std::exchange(other.m_lastEntityId, 0) }
    , m_freeEntities{ std::exchange(other.m_freeEntities, {}) }
    , m_entities{ std::exchange(other.m_entities, {}) }
{
}

EntityManager& EntityManager::operator=(EntityManager&& other) noexcept
{
    if (this != &other) {
        m_lastEntityId = std::exchange(other.m_lastEntityId, 0);
        m_freeEntities = std::exchange(other.m_freeEntities, {});
        m_entities = std::exchange(other.m_entities, {});
    }
    return *this;
}

bool EntityManager::hasEntity(Entity entity) const { return m_entities.contains(entity); }

Entity EntityManager::addEntity(const EntityArchetype& archetype)
{
    auto componentManager{ m_world->getManager<ComponentManager>() };
    if (componentManager == nullptr) {
        return { 0, 0 };
    }
    auto entity{ generateNextEntity() };
    auto archetypeId{ componentManager->addArchetype(archetype) };

    if (!componentManager->addEntity(entity, archetypeId)) {
        entity.generation--;
        m_freeEntities.push_back(entity);
        return { 0, 0 };
    }

    return entity;
}

bool EntityManager::removeEntity(Entity entity)
{
    auto componentManager{ m_world->getManager<ComponentManager>() };
    if (componentManager == nullptr) {
        return false;
    }
    if (!hasEntity(entity)) {
        return true;
    }
    componentManager->removeEntity(entity);
    m_freeEntities.push_back(entity);
    return true;
}

std::vector<Entity> EntityManager::addEntities(std::size_t numEntities, const EntityArchetype& archetype)
{
    auto componentManager{ m_world->getManager<ComponentManager>() };
    if (componentManager == nullptr) {
        return {};
    }
    auto archetypeId{ componentManager->addArchetype(archetype) };
    std::vector<Entity> entities{};
    entities.reserve(numEntities);

    for (std::size_t i = 0; i < numEntities; i++) {
        entities.push_back(generateNextEntity());
    }

    std::span<Entity> entitySpan{ entities.data(), entities.size() };
    if (!componentManager->addEntities(entitySpan, archetypeId)) {
        for (std::size_t i = 0; i < numEntities; i++) {
            entities[i].generation--;
            m_freeEntities.push_back(entities[i]);
            return {};
        }
    }

    return entities;
}

Entity EntityManager::generateNextEntity()
{
    if (m_freeEntities.empty()) {
        return { ++m_lastEntityId, 0 };
    } else {
        auto entity{ m_freeEntities.back() };
        entity.generation++;
        m_freeEntities.pop_back();
        return entity;
    }
}

}