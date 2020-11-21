#pragma once

#include <functional>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <visualizer/Entity.hpp>
#include <visualizer/EntityArchetype.hpp>
#include <visualizer/EntityComponentMap.hpp>
#include <visualizer/EntityContainer.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/TypeId.hpp>
#include <visualizer/World.hpp>

namespace Visualizer {

using EntityArchetypeId = std::size_t;

class ComponentManager : public GenericManager {
public:
    ComponentManager() = default;
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager(ComponentManager&&) noexcept = default;
    ~ComponentManager() noexcept;

    ComponentManager& operator=(const ComponentManager&) = delete;
    ComponentManager& operator=(ComponentManager&&) noexcept = default;

    bool hasArchetype(const EntityArchetype& archetype) const;
    bool hasArchetype(EntityArchetypeId archetypeId) const;

    std::optional<EntityArchetype> getArchetype(Entity entity) const;
    std::optional<EntityArchetype> getArchetype(EntityArchetypeId id) const;

    std::optional<EntityArchetypeId> getArchetypeId(Entity entity) const;
    std::optional<EntityArchetypeId> getArchetypeId(const EntityArchetype& archetype) const;

    EntityArchetypeId addArchetype(const EntityArchetype& archetype);
    bool removeArchetype(EntityArchetypeId archetypeId);

    bool addEntity(Entity entity, EntityArchetypeId archetypeId);
    bool changeEntityArchetype(Entity entity, EntityArchetypeId archetypeId);
    void removeEntity(Entity entity);

    bool addEntities(std::span<const Entity> entities, EntityArchetypeId archetypeId);
    bool changeEntityArchetypes(std::span<const Entity> entities, EntityArchetypeId archetypeId);
    void removeEntities(std::span<const Entity> entities);

    bool has_component(Entity entity, TypeId component_type) const;

    void* getEntityComponentPointer(Entity entity, TypeId typeId);
    const void* getEntityComponentPointer(Entity entity, TypeId typeId) const;

    EntityQueryResult query(const EntityQuery& query) const;

private:
    EntityArchetypeId generateNewId();

    EntityArchetypeId m_lastId;
    std::vector<EntityArchetypeId> m_freeIds;

    std::unordered_map<EntityArchetypeId, EntityContainer> m_chunks;
    std::unordered_map<Entity, EntityArchetypeId, EntityHasher> m_entities;
    std::unordered_map<TypeId, std::unordered_set<EntityArchetypeId>> m_typeAssociations;
    std::unordered_map<EntityArchetype, EntityArchetypeId, EntityArchetypeHasher> m_archetypeMap;
};
}
