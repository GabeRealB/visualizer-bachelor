#include <visualizer/ComponentManager.hpp>

#include <cassert>
#include <limits>
#include <mutex>

namespace Visualizer {

/**************************************************************************************************
 **************************************** ComponentManager ****************************************
 **************************************************************************************************/

ComponentManager::~ComponentManager() noexcept
{
    m_chunks.clear();
    m_archetypeMap.clear();

    m_freeIds.clear();
    m_entities.clear();
    m_typeAssociations.clear();
}

bool ComponentManager::hasArchetype(const EntityArchetype& archetype) const
{
    auto pos{ m_archetypeMap.find(archetype) };
    return pos != m_archetypeMap.end();
}

bool ComponentManager::hasArchetype(EntityArchetypeId archetypeId) const
{
    auto pos{ m_chunks.find(archetypeId) };
    return pos != m_chunks.end();
}

std::optional<EntityArchetypeId> ComponentManager::getArchetypeId(Entity entity) const
{
    auto pos{ m_entities.find(entity) };
    if (pos == m_entities.end()) {
        return std::nullopt;
    } else {
        return pos->second;
    }
}

std::optional<EntityArchetypeId> ComponentManager::getArchetypeId(const EntityArchetype& archetype) const
{
    auto pos{ m_archetypeMap.find(archetype) };
    if (pos == m_archetypeMap.end()) {
        return std::nullopt;
    } else {
        return pos->second;
    }
}

EntityArchetypeId ComponentManager::addArchetype(const EntityArchetype& archetype)
{
    /// TODO: Remove
    assert(false);
    (void)archetype;
    /*auto pos{ m_archetypeMap.find(archetype) };
    if (pos != m_archetypeMap.end()) {
        return pos->second;
    } else {
        auto id{ generateNewId() };
        auto result{ m_archetypeMap.insert({ archetype, id }) };
        m_chunks.emplace(id, result.first->first);

        for (auto type : archetype.types()) {
            if (auto typePos{ m_typeAssociations.find(type) }; typePos != m_typeAssociations.end()) {
                typePos->second.insert(id);
            } else {
                m_typeAssociations.emplace(type, 0).first->second.insert(id);
            }
        }

        return id;
    }*/
    return 0;
}

bool ComponentManager::addEntity(Entity entity, EntityArchetypeId archetypeId)
{
    auto chunkPos{ m_chunks.find(archetypeId) };
    if (chunkPos == m_chunks.end()) {
        return false;
    }

    auto entityPos{ m_entities.find(entity) };
    if (entityPos != m_entities.end()) {
        return false;
    }

    m_entities.insert({ entity, archetypeId });
    chunkPos->second.init(entity);
    return true;
}

bool ComponentManager::changeEntityArchetype(Entity entity, EntityArchetypeId archetypeId)
{
    auto entityPos{ m_entities.find(entity) };
    if (entityPos == m_entities.end()) {
        return false;
    }

    if (entityPos->second == archetypeId) {
        return true;
    }

    auto dstChunkPos{ m_chunks.find(archetypeId) };
    if (dstChunkPos == m_chunks.end()) {
        return false;
    }

    auto& srcChunk{ m_chunks.at(entityPos->second) };
    auto& dstChunk{ dstChunkPos->second };

    dstChunk.init_move(entity, srcChunk, srcChunk.entity_location(entity));
    return true;
}

void ComponentManager::removeEntity(Entity entity)
{
    auto entityPos{ m_entities.find(entity) };
    if (entityPos == m_entities.end()) {
        return;
    }

    auto& chunk{ m_chunks.at(entityPos->second) };
    chunk.erase(chunk.entity_location(entity));
}

bool ComponentManager::addEntities(std::span<const Entity> entities, EntityArchetypeId archetypeId)
{
    auto chunkPos{ m_chunks.find(archetypeId) };
    if (chunkPos == m_chunks.end()) {
        return false;
    }

    for (auto entity : entities) {
        auto entityPos{ m_entities.find(entity) };
        if (entityPos != m_entities.end()) {
            return false;
        }
    }

    for (auto entity : entities) {
        chunkPos->second.init(entity);
        m_entities.insert({ entity, archetypeId });
    }

    return true;
}

bool ComponentManager::changeEntityArchetypes(std::span<const Entity> entities, EntityArchetypeId archetypeId)
{
    for (auto entity : entities) {
        if (!changeEntityArchetype(entity, archetypeId)) {
            return false;
        }
    }
    return true;
}

void ComponentManager::removeEntities(std::span<const Entity> entities)
{
    for (auto entity : entities) {
        removeEntity(entity);
    }
}

bool ComponentManager::has_component(Entity entity, TypeId component_type) const
{
    auto entityPos{ m_entities.find(entity) };
    if (entityPos == m_entities.end()) {
        return false;
    } else {
        auto& chunk{ m_chunks.at(entityPos->second) };
        return chunk.has_component(component_type);
    }
}

void* ComponentManager::getEntityComponentPointer(Entity entity, TypeId typeId)
{
    auto entityPos{ m_entities.find(entity) };
    if (entityPos == m_entities.end()) {
        return nullptr;
    } else {
        auto& chunk{ m_chunks.at(entityPos->second) };
        auto entity_location{ chunk.entity_location(entity) };
        auto component_idx{ chunk.component_idx(typeId) };
        return chunk.fetch_unchecked(entity_location, component_idx);
    }
}

const void* ComponentManager::getEntityComponentPointer(Entity entity, TypeId typeId) const
{
    auto entityPos{ m_entities.find(entity) };
    if (entityPos == m_entities.end()) {
        return nullptr;
    } else {
        auto& chunk{ m_chunks.at(entityPos->second) };
        auto entity_location{ chunk.entity_location(entity) };
        auto component_idx{ chunk.component_idx(typeId) };
        return chunk.fetch_unchecked(entity_location, component_idx);
    }
}

EntityQueryResult ComponentManager::query(const EntityQuery& query) const
{
    auto withTypes{ query.withTypes() };
    auto withoutTypes{ query.withoutTypes() };

    std::unordered_set<EntityArchetypeId> withIds{};
    if (withTypes.size() > 0) {
        if (auto pos{ m_typeAssociations.find(withTypes[0]) }; pos != m_typeAssociations.end()) {
            withIds = pos->second;
        }

        for (std::size_t i = 1; i < withTypes.size() && withIds.size() > 0; ++i) {
            if (auto pos{ m_typeAssociations.find(withTypes[i]) }; pos != m_typeAssociations.end()) {
                std::erase_if(withIds, [&](const EntityArchetypeId& id) { return !pos->second.contains(id); });
            } else {
                withIds.clear();
            }
        }
    }

    if (withIds.empty()) {
        return EntityQueryResult{};
    }

    std::unordered_set<EntityArchetypeId> withoutIds{};
    for (auto type : withoutTypes) {
        if (auto pos{ m_typeAssociations.find(type) }; pos != m_typeAssociations.end()) {
            withoutIds.insert(pos->second.begin(), pos->second.end());
        }
    }

    if (!withoutIds.empty()) {
        std::erase_if(withIds, [&](const EntityArchetypeId& id) { return withoutIds.contains(id); });
    }

    if (withIds.empty()) {
        return EntityQueryResult{};
    }

    std::vector<Entity> entities{};
    std::vector<void*> components{};
    std::vector<TypeId> types{ withTypes.begin(), withTypes.end() };

    for (auto archetypeId : withIds) {
        auto& chunk{ m_chunks.at(archetypeId) };

        for (auto& entity_chunk : chunk.entity_chunks()) {
            for (auto& entity : entity_chunk.entities()) {
                entities.push_back(entity);
                auto entity_idx{ entity_chunk.entity_idx(entity) };
                for (auto type : withTypes) {
                    auto component_idx{ entity_chunk.component_idx(type) };
                    components.push_back(const_cast<void*>(entity_chunk.fetch_unchecked(entity_idx, component_idx)));
                }
            }
        }
    }

    return { std::move(entities), std::move(components), types };
}

EntityArchetypeId ComponentManager::generateNewId()
{
    if (!m_freeIds.empty()) {
        auto id{ m_freeIds.back() };
        m_freeIds.pop_back();
        return id;
    } else {
        return ++m_lastId;
    }
}

/**************************************************************************************************
 *************************************** EntityDatabaseImpl ***************************************
 **************************************************************************************************/

bool EntityDatabaseImpl::has_entity(Entity entity) const { return m_entities.contains(entity); }

bool EntityDatabaseImpl::has_component(ComponentType component_type) const
{
    return m_component_descriptors.contains(component_type);
}

bool EntityDatabaseImpl::entity_has_component(Entity entity, ComponentType component_type) const
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    return fetch_entity_container(entity).has_component(component_type);
}

ComponentType EntityDatabaseImpl::register_component_desc(
    ComponentType component_type, EntityComponentData component_desc)
{
    assert(!has_component(component_type));
    auto [pos, success] = m_component_descriptors.insert({ component_type, component_desc });
    assert(success);
    return component_type;
}

const EntityComponentData& EntityDatabaseImpl::fetch_component_desc(ComponentType component_type) const
{
    assert(has_component(component_type));
    return m_component_descriptors.at(component_type);
}

Entity EntityDatabaseImpl::init_entity(const EntityArchetype2& archetype)
{
    for (auto component_type : archetype.component_types()) {
        assert(has_component(component_type));
    }
    auto entity{ generate_new_entity() };
    auto& entity_container{ fetch_or_init_entity_container(archetype) };
    m_entities.emplace(entity, m_archetype_map.at(archetype));
    entity_container.init(entity);
    return entity;
}

Entity EntityDatabaseImpl::init_entity(EntityBuilder&& entity_builder)
{
    /// TODO: Implement move parameterised entity initialisation
    assert(false);
    (void)entity_builder;
    return Entity();
}

Entity EntityDatabaseImpl::init_entity(const EntityBuilder& entity_builder)
{
    /// TODO: Implement parameterised entity initialisation
    assert(false);
    (void)entity_builder;
    return Entity();
}

Entity EntityDatabaseImpl::init_entity_copy(Entity entity, const EntityArchetype2& archetype)
{
    assert(has_entity(entity));
    for (auto component_type : archetype.component_types()) {
        assert(has_component(component_type));
    }
    auto new_entity{ generate_new_entity() };
    auto& entity_container{ fetch_or_init_entity_container(archetype) };
    const auto& src_entity_container{ fetch_entity_container(entity) };
    auto src_entity_location{ src_entity_container.entity_location(entity) };
    m_entities.emplace(new_entity, m_archetype_map.at(archetype));
    entity_container.init_copy(new_entity, src_entity_container, src_entity_location);
    return new_entity;
}

void EntityDatabaseImpl::erase_entity(Entity entity)
{
    assert(has_entity(entity));
    auto& entity_container{ fetch_entity_container(entity) };
    auto entity_location{ entity_container.entity_location(entity) };
    entity_container.erase(entity_location);

    m_entities.erase(entity);
    m_free_entities.push_back(Entity{ entity.id, entity.generation + 1 });

    if (entity_container.size() == 0) {
        auto container_id{ m_entities.at(entity) };
        auto container_archetype{ entity_container.archetype() };
        m_entity_containers.erase(container_id);
        m_archetype_map.erase(container_archetype);
        for (auto component_type : container_archetype.component_types()) {
            m_type_associations.at(component_type).erase(container_id);
        }
        m_free_container_ids.push_back(container_id);
    }
}

void EntityDatabaseImpl::move_entity(Entity entity, const EntityArchetype2& archetype)
{
    assert(has_entity(entity));
    for (auto component_type : archetype.component_types()) {
        assert(has_component(component_type));
    }
    auto& dst_entity_container{ fetch_or_init_entity_container(archetype) };
    auto& src_entity_container{ fetch_entity_container(entity) };
    if (&dst_entity_container != &src_entity_container) {
        auto src_entity_location{ src_entity_container.entity_location(entity) };
        dst_entity_container.init_move(entity, src_entity_container, src_entity_location);

        if (src_entity_container.size() == 0) {
            auto src_entity_container_id{ m_entities.at(entity) };
            auto container_archetype{ src_entity_container.archetype() };
            m_entity_containers.erase(src_entity_container_id);
            m_archetype_map.erase(container_archetype);
            for (auto component_type : container_archetype.component_types()) {
                m_type_associations.at(component_type).erase(src_entity_container_id);
            }
            m_free_container_ids.push_back(src_entity_container_id);
        }

        m_entities.at(entity) = m_archetype_map.at(archetype);
    }
}

void EntityDatabaseImpl::add_component(Entity entity, ComponentType component_type)
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    auto src_archetype{ fetch_entity_archetype(entity) };
    auto dst_archetype{ src_archetype.with(component_type) };
    move_entity(entity, dst_archetype);
}

void EntityDatabaseImpl::add_component_move(Entity entity, ComponentType component_type, void* src)
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    add_component(entity, component_type);
    write_component_move(entity, component_type, src);
}

void EntityDatabaseImpl::add_component_copy(Entity entity, ComponentType component_type, const void* src)
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    add_component(entity, component_type);
    write_component_copy(entity, component_type, src);
}

void EntityDatabaseImpl::remove_component(Entity entity, ComponentType component_type)
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    auto& src_entity_container{ fetch_entity_container(entity) };
    auto src_archetype{ src_entity_container.archetype() };
    auto dst_archetype{ src_archetype.without(component_type) };
    move_entity(entity, dst_archetype);
}

void EntityDatabaseImpl::read_component(Entity entity, ComponentType component_type, void* dst) const
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    assert(entity_has_component(entity, component_type));
    const auto& entity_container{ fetch_entity_container(entity) };
    auto entity_location{ entity_container.entity_location(entity) };
    auto component_idx{ entity_container.component_idx(component_type) };
    entity_container.read(entity_location, component_idx, dst);
}

void EntityDatabaseImpl::write_component_move(Entity entity, ComponentType component_type, void* src)
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    assert(entity_has_component(entity, component_type));
    auto& entity_container{ fetch_entity_container(entity) };
    auto entity_location{ entity_container.entity_location(entity) };
    auto component_idx{ entity_container.component_idx(component_type) };
    entity_container.write_move(entity_location, component_idx, src);
}

void EntityDatabaseImpl::write_component_copy(Entity entity, ComponentType component_type, const void* src)
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    assert(entity_has_component(entity, component_type));
    auto& entity_container{ fetch_entity_container(entity) };
    auto entity_location{ entity_container.entity_location(entity) };
    auto component_idx{ entity_container.component_idx(component_type) };
    entity_container.write_copy(entity_location, component_idx, src);
}

void* EntityDatabaseImpl::fetch_component_unchecked(Entity entity, ComponentType component_type)
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    assert(entity_has_component(entity, component_type));
    auto& entity_container{ fetch_entity_container(entity) };
    auto entity_location{ entity_container.entity_location(entity) };
    auto component_idx{ entity_container.component_idx(component_type) };
    return entity_container.fetch_unchecked(entity_location, component_idx);
}

const void* EntityDatabaseImpl::fetch_component_unchecked(Entity entity, ComponentType component_type) const
{
    assert(has_entity(entity));
    assert(has_component(component_type));
    assert(entity_has_component(entity, component_type));
    auto& entity_container{ fetch_entity_container(entity) };
    auto entity_location{ entity_container.entity_location(entity) };
    auto component_idx{ entity_container.component_idx(component_type) };
    return entity_container.fetch_unchecked(entity_location, component_idx);
}

EntityContainer& EntityDatabaseImpl::fetch_entity_container(Entity entity)
{
    assert(has_entity(entity));
    auto container_id{ m_entities.at(entity) };
    return m_entity_containers.at(container_id);
}

const EntityContainer& EntityDatabaseImpl::fetch_entity_container(Entity entity) const
{
    assert(has_entity(entity));
    auto container_id{ m_entities.at(entity) };
    return m_entity_containers.at(container_id);
}

EntityArchetype2 EntityDatabaseImpl::fetch_entity_archetype(Entity entity) const
{
    assert(has_entity(entity));
    return fetch_entity_container(entity).archetype();
}

EntityQueryResult EntityDatabaseImpl::query(const EntityQuery& query)
{
    auto withTypes{ query.withTypes() };
    auto withoutTypes{ query.withoutTypes() };

    std::unordered_set<EntityArchetypeId> with_ids{};
    if (withTypes.size() > 0) {
        if (auto pos{ m_type_associations.find(withTypes[0]) }; pos != m_type_associations.end()) {
            with_ids = pos->second;
        }

        for (std::size_t i = 1; i < withTypes.size() && with_ids.size() > 0; ++i) {
            if (auto pos{ m_type_associations.find(withTypes[i]) }; pos != m_type_associations.end()) {
                std::erase_if(with_ids, [&](const EntityArchetypeId& id) { return !pos->second.contains(id); });
            } else {
                with_ids.clear();
            }
        }
    }

    if (with_ids.empty()) {
        return EntityQueryResult{};
    }

    std::unordered_set<EntityArchetypeId> without_ids{};
    for (auto type : withoutTypes) {
        if (auto pos{ m_type_associations.find(type) }; pos != m_type_associations.end()) {
            without_ids.insert(pos->second.begin(), pos->second.end());
        }
    }

    if (!without_ids.empty()) {
        std::erase_if(with_ids, [&](const EntityArchetypeId& id) { return without_ids.contains(id); });
    }

    if (with_ids.empty()) {
        return EntityQueryResult{};
    }

    std::vector<Entity> entities{};
    std::vector<void*> components{};
    std::vector<TypeId> types{ withTypes.begin(), withTypes.end() };

    std::vector<std::size_t> component_indices{};

    for (auto archetypeId : with_ids) {
        auto& entity_container{ m_entity_containers.at(archetypeId) };
        component_indices.reserve(withTypes.size());
        for (auto type : withTypes) {
            auto component_idx{ entity_container.component_idx(type) };
            component_indices.push_back(component_idx);
        }

        for (auto& entity_chunk : entity_container.entity_chunks()) {
            for (auto& entity : entity_chunk.entities()) {
                entities.push_back(entity);
                auto entity_idx{ entity_chunk.entity_idx(entity) };
                for (auto component_idx : component_indices) {
                    components.push_back(const_cast<void*>(entity_chunk.fetch_unchecked(entity_idx, component_idx)));
                }
            }
        }

        component_indices.clear();
    }

    return { std::move(entities), std::move(components), types };
}

Entity EntityDatabaseImpl::generate_new_entity()
{
    if (m_free_entities.empty()) {
        auto entity{ Entity{ m_last_entity.id + 1, m_last_entity.generation } };
        m_last_entity = entity;
        return entity;
    } else {
        auto entity{ m_free_entities.back() };
        m_free_entities.pop_back();
        return entity;
    }
}

EntityContainer& EntityDatabaseImpl::fetch_or_init_entity_container(const EntityArchetype2& archetype)
{
    if (m_archetype_map.contains(archetype)) {
        return m_entity_containers.at(m_archetype_map.at(archetype));
    } else {
        for (auto component_type : archetype.component_types()) {
            assert(has_component(component_type));
        }
        EntityContainerId container_id;
        if (m_free_container_ids.empty()) {
            container_id = m_last_container_id++;
        } else {
            container_id = m_free_container_ids.back();
            m_free_container_ids.pop_back();
        }
        auto [pos, res] = m_entity_containers.emplace(
            std::piecewise_construct, std::forward_as_tuple(container_id), std::forward_as_tuple(archetype, *this));
        assert(res);
        m_archetype_map.emplace(archetype, container_id);
        for (auto component_type : archetype.component_types()) {
            m_type_associations.try_emplace(component_type).first->second.emplace(container_id);
        }
        return pos->second;
    }
}

/**************************************************************************************************
 ***************************************** EntityDatabase *****************************************
 **************************************************************************************************/

EntityDatabase::~EntityDatabase() noexcept { std::scoped_lock lock{ m_context_mutex }; }

/**************************************************************************************************
 ************************************* EntityDatabaseContext *************************************
 **************************************************************************************************/

EntityDatabaseContext::EntityDatabaseContext(EntityDatabaseImpl& database)
    : m_database{ database }
{
}

bool EntityDatabaseContext::has_entity(Entity entity) const { return m_database.has_entity(entity); }

bool EntityDatabaseContext::has_component(ComponentType component_type) const
{
    return m_database.has_component(component_type);
}

bool EntityDatabaseContext::entity_has_component(Entity entity, ComponentType component_type) const
{
    return m_database.entity_has_component(entity, component_type);
}

ComponentType EntityDatabaseContext::register_component_desc(
    ComponentType component_type, EntityComponentData component_desc)
{
    return m_database.register_component_desc(component_type, component_desc);
}

const EntityComponentData& EntityDatabaseContext::fetch_component_desc(ComponentType component_type) const
{
    return m_database.fetch_component_desc(component_type);
}

Entity EntityDatabaseContext::init_entity(const EntityArchetype2& archetype)
{
    return m_database.init_entity(archetype);
}

Entity EntityDatabaseContext::init_entity(EntityBuilder&& entity_builder)
{
    return m_database.init_entity(entity_builder);
}

Entity EntityDatabaseContext::init_entity(const EntityBuilder& entity_builder)
{
    return m_database.init_entity(entity_builder);
}

Entity EntityDatabaseContext::init_entity_copy(Entity entity, const EntityArchetype2& archetype)
{
    return m_database.init_entity_copy(entity, archetype);
}

void EntityDatabaseContext::erase_entity(Entity entity) { m_database.erase_entity(entity); }

void EntityDatabaseContext::move_entity(Entity entity, const EntityArchetype2& archetype)
{
    m_database.move_entity(entity, archetype);
}

void EntityDatabaseContext::add_component(Entity entity, ComponentType component_type)
{
    m_database.add_component(entity, component_type);
}

void EntityDatabaseContext::add_component_move(Entity entity, ComponentType component_type, void* src)
{
    m_database.add_component_move(entity, component_type, src);
}

void EntityDatabaseContext::add_component_copy(Entity entity, ComponentType component_type, const void* src)
{
    m_database.add_component_copy(entity, component_type, src);
}

void EntityDatabaseContext::remove_component(Entity entity, ComponentType component_type)
{
    m_database.remove_component(entity, component_type);
}

void EntityDatabaseContext::read_component(Entity entity, ComponentType component_type, void* dst) const
{
    m_database.read_component(entity, component_type, dst);
}

void EntityDatabaseContext::write_component_move(Entity entity, ComponentType component_type, void* src)
{
    m_database.write_component_move(entity, component_type, src);
}

void EntityDatabaseContext::write_component_copy(Entity entity, ComponentType component_type, const void* src)
{
    m_database.write_component_copy(entity, component_type, src);
}

void* EntityDatabaseContext::fetch_component_unchecked(Entity entity, ComponentType component_type)
{
    return m_database.fetch_component_unchecked(entity, component_type);
}

const void* EntityDatabaseContext::fetch_component_unchecked(Entity entity, ComponentType component_type) const
{
    return m_database.fetch_component_unchecked(entity, component_type);
}

EntityContainer& EntityDatabaseContext::fetch_entity_container(Entity entity)
{
    return m_database.fetch_entity_container(entity);
}

const EntityContainer& EntityDatabaseContext::fetch_entity_container(Entity entity) const
{
    return m_database.fetch_entity_container(entity);
}

EntityArchetype2 EntityDatabaseContext::fetch_entity_archetype(Entity entity) const
{
    return m_database.fetch_entity_archetype(entity);
}

EntityQueryResult EntityDatabaseContext::query(const EntityQuery& query) { return m_database.query(query); }

/**************************************************************************************************
 *********************************** EntityDatabaseLazyContext ***********************************
 **************************************************************************************************/

EntityDatabaseLazyContext::EntityDatabaseLazyContext(EntityDatabaseImpl& database)
    : m_database{ database }
{
}

bool EntityDatabaseLazyContext::has_entity(Entity entity) const { return m_database.has_entity(entity); }

bool EntityDatabaseLazyContext::has_component(ComponentType component_type) const
{
    return m_database.has_component(component_type);
}

bool EntityDatabaseLazyContext::entity_has_component(Entity entity, ComponentType component_type) const
{
    return m_database.entity_has_component(entity, component_type);
}

void EntityDatabaseLazyContext::read_component(Entity entity, ComponentType component_type, void* dst) const
{
    m_database.read_component(entity, component_type, dst);
}

void EntityDatabaseLazyContext::write_component_move(Entity entity, ComponentType component_type, void* src)
{
    m_database.write_component_move(entity, component_type, src);
}

void EntityDatabaseLazyContext::write_component_copy(Entity entity, ComponentType component_type, const void* src)
{
    m_database.write_component_copy(entity, component_type, src);
}

void* EntityDatabaseLazyContext::fetch_component_unchecked(Entity entity, ComponentType component_type)
{
    return m_database.fetch_component_unchecked(entity, component_type);
}

const void* EntityDatabaseLazyContext::fetch_component_unchecked(Entity entity, ComponentType component_type) const
{
    return m_database.fetch_component_unchecked(entity, component_type);
}

EntityArchetype2 EntityDatabaseLazyContext::fetch_entity_archetype(Entity entity) const
{
    return m_database.fetch_entity_archetype(entity);
}

EntityQueryResult EntityDatabaseLazyContext::query(const EntityQuery& query) { return m_database.query(query); }

}