#pragma once

#include <memory>
#include <optional>
#include <unordered_map>

#include <visualizer/AlignedMemory.hpp>
#include <visualizer/Entity.hpp>
#include <visualizer/EntityArchetype.hpp>
#include <visualizer/TypeId.hpp>
#include <visualizer/UniqueTypes.hpp>

namespace Visualizer {

struct EntityComponentLayoutInfo {
    TypeId typeId;
    std::size_t size;
    std::size_t alignment;
    std::size_t startIndex;
};

class EntityComponentLayoutMap {
public:
    EntityComponentLayoutMap(const EntityArchetype& archetype);

    std::optional<EntityComponentLayoutInfo> operator[](std::size_t idx) const;

    std::size_t size() const;
    std::size_t count() const;
    std::size_t stride() const;
    std::size_t alignment() const;
    std::size_t paddedSize() const;
    std::span<const EntityComponentLayoutInfo> layout() const;

    bool hasComponent(TypeId typeId) const;
    std::optional<EntityComponentLayoutInfo> layoutInfo(TypeId typeId) const;

    template <typename T> requires NoCVRefs<T> bool hasComponent() const;

    template <typename T> requires NoCVRefs<T> std::optional<EntityComponentLayoutInfo> layoutInfo() const;

private:
    std::vector<EntityComponentLayoutInfo> m_layoutInfos;
    std::unordered_map<TypeId, std::size_t> m_componentMap;
};

class EntityComponentMap {
public:
    EntityComponentMap(const EntityArchetype& archetype);
    EntityComponentMap(const EntityComponentMap& other) = delete;
    EntityComponentMap(EntityComponentMap&& other) noexcept = default;
    ~EntityComponentMap();

    EntityComponentMap& operator=(const EntityComponentMap& other) = delete;

    bool empty() const;
    std::size_t size() const;

    std::size_t capacity() const;
    std::span<const Entity> entities() const;
    const EntityArchetype& archetype() const;

    bool hasEntity(Entity entity) const;
    bool hasComponent(TypeId typeId) const;

    void insert(Entity entity);
    void erase(Entity entity);

    void insert(std::span<const Entity> entities);
    void erase(std::span<const Entity> entities);

    void* component(Entity entity, TypeId typeId) const;
    void moveComponents(const EntityComponentMap& src, Entity entity);

private:
    EntityComponentMap(const EntityArchetype& archetype, const EntityComponentLayoutMap& layoutMap);
    EntityComponentMap& operator=(EntityComponentMap&& other) noexcept;

    void grow(std::size_t entityCount);

    std::size_t m_entityCount;
    std::vector<Entity> m_entities;
    const EntityArchetype& m_archetype;
    std::span<unsigned char> m_componentsSpan;
    std::vector<std::size_t> m_availableIndices;
    EntityComponentLayoutMap m_componentLayoutMap;
    std::unordered_map<Entity, std::size_t, EntityHasher> m_entityMap;
    std::unique_ptr<unsigned char[], AlignedDeleter<unsigned char>> m_componentsData;
};

}

#include <visualizer/EntityComponentMap.impl>