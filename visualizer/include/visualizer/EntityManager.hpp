#pragma once

#include <unordered_set>
#include <vector>

#include <visualizer/ComponentManager.hpp>
#include <visualizer/Entity.hpp>
#include <visualizer/World.hpp>

namespace Visualizer {

class EntityManager : public GenericManager {
public:
    EntityManager();
    EntityManager(const EntityManager& other) = delete;
    EntityManager(EntityManager&& other) noexcept;

    EntityManager& operator=(const EntityManager& other) = delete;
    EntityManager& operator=(EntityManager&& other) noexcept;

    bool hasEntity(Entity entity) const;
    Entity addEntity(const EntityArchetype& archetype);
    bool removeEntity(Entity entity);

    std::vector<Entity> addEntities(std::size_t numEntities, const EntityArchetype& archetype);

private:
    Entity generateNextEntity();

    std::size_t m_lastEntityId;
    std::vector<Entity> m_freeEntities;
    std::unordered_set<Entity, EntityHasher> m_entities;
};

}