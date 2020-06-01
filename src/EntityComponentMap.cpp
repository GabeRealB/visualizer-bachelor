#include <visualizer/EntityComponentMap.hpp>

#include <algorithm>
#include <cassert>
#include <iterator>

namespace Visualizer {

/**************************************************************************************************
 ************************************ EntityComponentLayoutMap ************************************
 **************************************************************************************************/

EntityComponentLayoutMap::EntityComponentLayoutMap(const EntityArchetype& archetype)
    : m_layoutInfos{}
    , m_componentMap{}
{
    m_layoutInfos.reserve(archetype.size());
    m_componentMap.reserve(archetype.size());

    auto typeInfos{ archetype.componentInfos() };
    for (auto& typeInfo : typeInfos) {
        EntityComponentLayoutInfo layoutInfo{
            .typeId = typeInfo.id, .size = typeInfo.size, .alignment = typeInfo.alignment, .startIndex = 0
        };
        m_layoutInfos.push_back(layoutInfo);
    }

    std::sort(m_layoutInfos.begin(), m_layoutInfos.end(),
        [](const EntityComponentLayoutInfo& lhs, const EntityComponentLayoutInfo& rhs) -> bool {
            return lhs.alignment > rhs.alignment;
        });

    for (std::size_t i = 0; i < m_layoutInfos.size(); ++i) {
        auto& layoutInfo{ m_layoutInfos[i] };

        if (i == 0) {
            continue;
        } else {
            auto& previousLayoutInfo{ m_layoutInfos[i - 1] };
            auto minIndex{ previousLayoutInfo.startIndex + previousLayoutInfo.size };
            auto indexAlignmentOffset{ minIndex % layoutInfo.alignment };
            auto adjustedIndex{ minIndex + indexAlignmentOffset };
            layoutInfo.startIndex = adjustedIndex;
        }

        m_componentMap.emplace(layoutInfo.typeId, i);
    }
}

std::optional<EntityComponentLayoutInfo> EntityComponentLayoutMap::operator[](std::size_t idx) const
{
    if (idx >= count()) {
        return std::nullopt;
    } else {
        return m_layoutInfos[idx];
    }
}

std::size_t EntityComponentLayoutMap::size() const
{
    auto& lastComponentLayoutInfo{ m_layoutInfos.back() };
    return lastComponentLayoutInfo.startIndex + lastComponentLayoutInfo.size;
}

std::size_t EntityComponentLayoutMap::count() const { return m_layoutInfos.size(); }

std::size_t EntityComponentLayoutMap::stride() const { return size() % alignment(); }

std::size_t EntityComponentLayoutMap::alignment() const { return m_layoutInfos.front().alignment; }

std::size_t EntityComponentLayoutMap::paddedSize() const { return size() + stride(); }

std::span<const EntityComponentLayoutInfo> EntityComponentLayoutMap::layout() const
{
    return { m_layoutInfos.data(), m_layoutInfos.size() };
}

bool EntityComponentLayoutMap::hasComponent(TypeId typeId) const { return m_componentMap.contains(typeId); }

std::optional<EntityComponentLayoutInfo> EntityComponentLayoutMap::layoutInfo(TypeId typeId) const
{
    if (auto pos = m_componentMap.find(typeId); pos != m_componentMap.end()) {
        return this->operator[](pos->second);
    } else {
        return std::nullopt;
    }
}

/**************************************************************************************************
 *************************************** EntityComponentMap ***************************************
 **************************************************************************************************/

EntityComponentMap::EntityComponentMap(const EntityArchetype& archetype)
    : m_entityCount{ 0 }
    , m_entities{}
    , m_archetype{ archetype }
    , m_componentsSpan{}
    , m_availableIndices{ { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 } }
    , m_componentLayoutMap{ archetype }
    , m_entityMap{}
    , m_componentsData{ AlignedDeleter<unsigned char>::allocate(
                            m_componentLayoutMap.alignment(), m_componentLayoutMap.paddedSize() * 10),
        AlignedDeleter<unsigned char>{} }
{
    m_entities.assign(10, { 0, 0 });
    m_componentsSpan = { m_componentsData.get(), m_componentLayoutMap.paddedSize() * 10 };
    m_entityMap.reserve(10);
    assert(reinterpret_cast<std::uintptr_t>(m_componentsData.get()) % m_componentLayoutMap[0]->alignment == 0);
}

EntityComponentMap::EntityComponentMap(const EntityArchetype& archetype, const EntityComponentLayoutMap& layoutMap)
    : m_entityCount{ 0 }
    , m_entities{}
    , m_archetype{ archetype }
    , m_componentsSpan{}
    , m_availableIndices{}
    , m_componentLayoutMap{ layoutMap }
    , m_entityMap{}
    , m_componentsData{}
{
}

EntityComponentMap& EntityComponentMap::operator=(EntityComponentMap&& other) noexcept
{
    if (this != &other) {
        std::swap(m_entityCount, other.m_entityCount);
        std::swap(m_entities, other.m_entities);
        std::swap(m_componentsSpan, other.m_componentsSpan);
        std::swap(m_availableIndices, other.m_availableIndices);
        std::swap(m_componentLayoutMap, other.m_componentLayoutMap);
        std::swap(m_entityMap, other.m_entityMap);
        std::swap(m_componentsData, other.m_componentsData);
    }

    return *this;
}

EntityComponentMap::~EntityComponentMap()
{
    for (auto entity : m_entities) {
        erase(entity);
    }
}

bool EntityComponentMap::empty() const { return m_entityCount == 0; }

std::size_t EntityComponentMap::size() const { return m_entityCount; }

std::size_t EntityComponentMap::capacity() const { return m_entities.capacity(); }

std::span<const Entity> EntityComponentMap::entities() const { return { m_entities.data(), m_entities.size() }; }

const EntityArchetype& EntityComponentMap::archetype() const { return m_archetype; }

bool EntityComponentMap::hasEntity(Entity entity) const { return m_entityMap.contains(entity); }

bool EntityComponentMap::hasComponent(TypeId typeId) const { return m_archetype.hasComponent(typeId); }

void EntityComponentMap::insert(Entity entity)
{
    if (size() == capacity()) {
        grow(capacity() * 2);
    } else {
        auto entityIndex{ m_availableIndices.back() };
        m_availableIndices.pop_back();
        m_entityCount++;
        m_entities[entityIndex] = entity;
        m_entityMap.insert({ entity, entityIndex });

        for (auto type : m_archetype.types()) {
            auto componentPtr{ component(entity, type) };
            m_archetype.componentInfo(type)->createFunc(componentPtr);
        }
    }
}

void EntityComponentMap::erase(Entity entity)
{
    if (auto pos = m_entityMap.find(entity); pos != m_entityMap.end()) {
        for (auto type : m_archetype.types()) {
            auto componentPtr{ component(entity, type) };
            m_archetype.componentInfo(type)->destructorFunc(componentPtr);
        }

        m_entityCount--;
        m_availableIndices.push_back(pos->second);
        m_entityMap.erase(pos);
    }
}

void EntityComponentMap::insert(std::span<const Entity> entities)
{
    for (auto& entity : entities) {
        insert(entity);
    }
}

void EntityComponentMap::erase(std::span<const Entity> entities)
{
    for (auto& entity : entities) {
        erase(entity);
    }
}

void* EntityComponentMap::component(Entity entity, TypeId typeId) const
{
    if (!hasEntity(entity) || !hasComponent(typeId)) {
        return nullptr;
    } else {
        auto entityIndex{ m_entityMap.at(entity) };
        auto componentStartIndex{ m_componentLayoutMap.layoutInfo(typeId)->startIndex };
        auto componentIndex{ (entityIndex * m_componentLayoutMap.paddedSize()) + componentStartIndex };

        return &m_componentsSpan[componentIndex];
    }
}

void EntityComponentMap::moveComponents(const EntityComponentMap& src, Entity entity)
{
    if (hasEntity(entity) || !src.hasEntity(entity)) {
        return;
    } else {
        if (size() == capacity()) {
            grow(capacity() * 2);
        } else {
            auto entityIndex{ m_availableIndices.back() };
            m_availableIndices.pop_back();
            m_entityCount++;
            m_entities[entityIndex] = entity;
            m_entityMap.insert({ entity, entityIndex });

            for (auto type : m_archetype.types()) {
                auto componentPtr{ component(entity, type) };
                if (src.hasComponent(type)) {
                    auto srcComponentPtr{ src.component(entity, type) };
                    m_archetype.componentInfo(type)->moveFunc(srcComponentPtr, componentPtr);
                } else {
                    m_archetype.componentInfo(type)->createFunc(componentPtr);
                }
            }
        }
    }
}

void EntityComponentMap::grow(std::size_t entityCount)
{
    if (entityCount <= capacity()) {
        return;
    } else {
        EntityComponentMap newMap{ m_archetype, m_componentLayoutMap };
        newMap.m_entities.assign(entityCount, { 0, 0 });
        newMap.m_componentsData = { AlignedDeleter<unsigned char>::allocate(m_componentLayoutMap.alignment(),
                                        m_componentLayoutMap.paddedSize() * entityCount),
            AlignedDeleter<unsigned char>{} };
        newMap.m_componentsSpan = { newMap.m_componentsData.get(), m_componentLayoutMap.paddedSize() * entityCount };
        newMap.m_entityMap.reserve(entityCount);

        assert(
            reinterpret_cast<std::uintptr_t>(newMap.m_componentsData.get()) % newMap.m_componentLayoutMap[0]->alignment
            == 0);

        for (std::size_t i = entityCount - 1; i <= 0; --i) {
            newMap.m_availableIndices.push_back(i);
        }

        for (auto& entity : m_entityMap) {
            newMap.moveComponents(*this, entity.first);
        }

        *this = std::move(newMap);
    }
}

}