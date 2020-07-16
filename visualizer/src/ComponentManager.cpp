#include <visualizer/ComponentManager.hpp>

#include <limits>
#include <visualizer/EntityArchetype.hpp>
#include <visualizer/EntityComponentMap.hpp>

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

std::optional<EntityArchetype> ComponentManager::getArchetype(Entity entity) const
{
    auto pos{ m_entities.find(entity) };
    if (pos == m_entities.end()) {
        return std::nullopt;
    } else {
        return getArchetype(pos->second);
    }
}

std::optional<EntityArchetype> ComponentManager::getArchetype(EntityArchetypeId id) const
{
    auto pos{ m_chunks.find(id) };
    if (pos == m_chunks.end()) {
        return std::nullopt;
    } else {
        return pos->second.archetype();
    }
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
    auto pos{ m_archetypeMap.find(archetype) };
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
    }
}

bool ComponentManager::removeArchetype(EntityArchetypeId archetypeId)
{
    auto chunkPos{ m_chunks.find(archetypeId) };
    if (chunkPos == m_chunks.end()) {
        return true;
    }

    if (!chunkPos->second.empty()) {
        return false;
    }

    auto& archetype{ chunkPos->second.archetype() };
    m_chunks.erase(chunkPos);

    for (auto type : archetype.types()) {
        auto typePos{ m_typeAssociations.find(type) };
        typePos->second.erase(archetypeId);
        if (typePos->second.empty()) {
            m_typeAssociations.erase(typePos);
        }
    }

    m_archetypeMap.erase(archetype);
    m_freeIds.push_back(archetypeId);
    return true;
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
    chunkPos->second.insert(entity);
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

    dstChunk.moveComponents(srcChunk, entity);
    // dstChunk.copyEntityFrom(entity, srcChunk.archetype, srcChunk.getEntityPtr(entity));
    return true;
}

void ComponentManager::removeEntity(Entity entity)
{
    auto entityPos{ m_entities.find(entity) };
    if (entityPos == m_entities.end()) {
        return;
    }

    auto& chunk{ m_chunks.at(entityPos->second) };
    chunk.erase(entity);
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

    chunkPos->second.insert(entities);
    for (auto entity : entities) {
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

void* ComponentManager::getEntityComponentPointer(Entity entity, TypeId typeId) const
{
    auto entityPos{ m_entities.find(entity) };
    if (entityPos == m_entities.end()) {
        return nullptr;
    } else {
        return m_chunks.at(entityPos->second).component(entity, typeId);
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

        for (auto& entity : chunk.entities()) {
            if (chunk.hasEntity(entity)) {
                entities.push_back(entity);
                for (auto type : withTypes) {
                    components.push_back(chunk.component(entity, type));
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

}