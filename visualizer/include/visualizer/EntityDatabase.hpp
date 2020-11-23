#pragma once

#include <atomic>
#include <functional>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <visualizer/Entity.hpp>
#include <visualizer/EntityArchetype.hpp>
#include <visualizer/EntityContainer.hpp>
#include <visualizer/EntityQuery.hpp>
#include <visualizer/TypeId.hpp>
#include <visualizer/World.hpp>

namespace Visualizer {

using EntityArchetypeId = std::size_t;

class EntityBuilder;
class EntityDatabaseContext;
class EntityDatabaseLazyContext;

using ComponentType = TypeId;

using EntityDatabaseCallback = void (*)(EntityDatabaseContext& context);
using EntityDatabaseConstCallback = void (*)(const EntityDatabaseContext& context);
using EntityDatabaseLazyCallback = void (*)(EntityDatabaseLazyContext& context);
using EntityDatabaseLazyConstCallback = void (*)(const EntityDatabaseLazyContext& context);

class EntityDatabaseImpl {
public:
    EntityDatabaseImpl() = default;
    EntityDatabaseImpl(const EntityDatabaseImpl&) = delete;
    EntityDatabaseImpl(EntityDatabaseImpl&&) noexcept = default;
    ~EntityDatabaseImpl() noexcept = default;

    EntityDatabaseImpl& operator=(const EntityDatabaseImpl&) = delete;
    EntityDatabaseImpl& operator=(EntityDatabaseImpl&&) noexcept = default;

    bool has_entity(Entity entity) const;
    bool has_component(ComponentType component_type) const;
    bool entity_has_component(Entity entity, ComponentType component_type) const;

    ComponentType register_component_desc(ComponentType component_type, EntityComponentData component_desc);
    const EntityComponentData& fetch_component_desc(ComponentType component_type) const;

    Entity init_entity(const EntityArchetype2& archetype);
    Entity init_entity(EntityBuilder&& entity_builder);
    Entity init_entity(const EntityBuilder& entity_builder);
    Entity init_entity_copy(Entity entity, const EntityArchetype2& archetype);
    void erase_entity(Entity entity);

    void move_entity(Entity entity, const EntityArchetype2& archetype);

    void add_component(Entity entity, ComponentType component_type);
    void add_component_move(Entity entity, ComponentType component_type, void* src);
    void add_component_copy(Entity entity, ComponentType component_type, const void* src);
    void remove_component(Entity entity, ComponentType component_type);

    void read_component(Entity entity, ComponentType component_type, void* dst) const;
    void write_component_move(Entity entity, ComponentType component_type, void* src);
    void write_component_copy(Entity entity, ComponentType component_type, const void* src);

    void* fetch_component_unchecked(Entity entity, ComponentType component_type);
    const void* fetch_component_unchecked(Entity entity, ComponentType component_type) const;

    EntityContainer& fetch_entity_container(Entity entity);
    const EntityContainer& fetch_entity_container(Entity entity) const;

    EntityArchetype2 fetch_entity_archetype(Entity entity) const;

    EntityQueryResult query(const EntityQuery& query);

private:
    using EntityContainerId = std::size_t;

    Entity generate_new_entity();
    EntityContainer& fetch_or_init_entity_container(const EntityArchetype2& archetype);

    Entity m_last_entity;
    EntityContainerId m_last_container_id;

    std::vector<Entity> m_free_entities;
    std::vector<EntityContainerId> m_free_container_ids;

    std::unordered_map<Entity, EntityContainerId, EntityHasher> m_entities;
    std::unordered_map<TypeId, EntityComponentData> m_component_descriptors;
    std::unordered_map<EntityContainerId, EntityContainer> m_entity_containers;
    std::unordered_map<TypeId, std::unordered_set<EntityContainerId>> m_type_associations;
    std::unordered_map<EntityArchetype2, EntityContainerId, EntityArchetype2Hasher> m_archetype_map;
};

class EntityDatabase : public GenericManager {
public:
    EntityDatabase() = default;
    EntityDatabase(const EntityDatabase& other) = delete;
    EntityDatabase(EntityDatabase&& other) noexcept = delete;
    ~EntityDatabase() noexcept;

    EntityDatabase& operator=(const EntityDatabase& other) = delete;
    EntityDatabase& operator=(EntityDatabase&& other) noexcept = delete;

    template <typename F> void enter_secure_context(F&& f);
    template <typename F> void enter_secure_context(F&& f) const;

    template <typename F> void enter_secure_lazy_context(F&& f);
    template <typename F> void enter_secure_lazy_context(F&& f) const;

private:
    mutable std::shared_mutex m_context_mutex;
    mutable EntityDatabaseImpl m_database_impl;
};

class EntityDatabaseContext {
public:
    EntityDatabaseContext(EntityDatabaseImpl& database);
    EntityDatabaseContext(const EntityDatabaseContext& other) = delete;
    EntityDatabaseContext(EntityDatabaseContext&& other) noexcept = delete;
    ~EntityDatabaseContext() noexcept = default;

    EntityDatabaseContext& operator=(const EntityDatabaseContext& other) = delete;
    EntityDatabaseContext& operator=(EntityDatabaseContext&& other) noexcept = delete;

    bool has_entity(Entity entity) const;
    bool has_component(ComponentType component_type) const;
    bool entity_has_component(Entity entity, ComponentType component_type) const;

    ComponentType register_component_desc(ComponentType component_type, EntityComponentData component_desc);
    const EntityComponentData& fetch_component_desc(ComponentType component_type) const;

    Entity init_entity(const EntityArchetype2& archetype);
    Entity init_entity(EntityBuilder&& entity_builder);
    Entity init_entity(const EntityBuilder& entity_builder);
    Entity init_entity_copy(Entity entity, const EntityArchetype2& archetype);
    void erase_entity(Entity entity);

    void move_entity(Entity entity, const EntityArchetype2& archetype);

    void add_component(Entity entity, ComponentType component_type);
    void add_component_move(Entity entity, ComponentType component_type, void* src);
    void add_component_copy(Entity entity, ComponentType component_type, const void* src);
    void remove_component(Entity entity, ComponentType component_type);

    void read_component(Entity entity, ComponentType component_type, void* dst) const;
    void write_component_move(Entity entity, ComponentType component_type, void* src);
    void write_component_copy(Entity entity, ComponentType component_type, const void* src);

    void* fetch_component_unchecked(Entity entity, ComponentType component_type);
    const void* fetch_component_unchecked(Entity entity, ComponentType component_type) const;

    EntityContainer& fetch_entity_container(Entity entity);
    const EntityContainer& fetch_entity_container(Entity entity) const;

    EntityArchetype2 fetch_entity_archetype(Entity entity) const;

    EntityQueryResult query(const EntityQuery& query);

    template <typename T> requires NoCVRefs<T> ComponentType register_component_desc();

    template <typename T> requires NoCVRefs<T> bool entity_has_component(Entity entity) const;

    template <typename T> requires NoCVRefs<T> void add_component(Entity entity);
    template <typename T> requires NoCVRefs<T> void add_component(Entity entity, T&& component);
    template <typename T> requires NoCVRefs<T> void add_component(Entity entity, const T& component);
    template <typename T> requires NoCVRefs<T> void remove_component(Entity entity);

    template <typename T> requires NoCVRefs<T> T read_component(Entity entity) const;
    template <typename T> requires NoCVRefs<T> void write_component(Entity entity, T&& component);
    template <typename T> requires NoCVRefs<T> void write_component(Entity entity, const T& component);

    template <typename T> requires NoCVRefs<T> T& fetch_component_unchecked(Entity entity);
    template <typename T> requires NoCVRefs<T> const T& fetch_component_unchecked(Entity entity) const;

private:
    EntityDatabaseImpl& m_database;
};

class EntityDatabaseLazyContext {
public:
    EntityDatabaseLazyContext(EntityDatabaseImpl& database);
    EntityDatabaseLazyContext(const EntityDatabaseLazyContext& other) = delete;
    EntityDatabaseLazyContext(EntityDatabaseLazyContext&& other) noexcept = delete;
    ~EntityDatabaseLazyContext() noexcept = default;

    EntityDatabaseLazyContext& operator=(const EntityDatabaseLazyContext& other) = delete;
    EntityDatabaseLazyContext& operator=(EntityDatabaseLazyContext&& other) noexcept = delete;

    bool has_entity(Entity entity) const;
    bool has_component(ComponentType component_type) const;
    bool entity_has_component(Entity entity, ComponentType component_type) const;

    void read_component(Entity entity, ComponentType component_type, void* dst) const;
    void write_component_move(Entity entity, ComponentType component_type, void* src);
    void write_component_copy(Entity entity, ComponentType component_type, const void* src);

    void* fetch_component_unchecked(Entity entity, ComponentType component_type);
    const void* fetch_component_unchecked(Entity entity, ComponentType component_type) const;

    EntityArchetype2 fetch_entity_archetype(Entity entity) const;

    EntityQueryResult query(const EntityQuery& query);

private:
    EntityDatabaseImpl& m_database;
};

}

#include <visualizer/EntityDatabase.impl>