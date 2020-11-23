#pragma once

#include <array>
#include <functional>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

#include <visualizer/AlignedMemory.hpp>
#include <visualizer/Entity.hpp>
#include <visualizer/EntityArchetype.hpp>

namespace Visualizer {

constexpr std::size_t ENTITY_CHUNK_SIZE{ 32 };
constexpr std::size_t ENTITY_CHUNK_ALLOCATION_BUFFER{ 2 };

class EntityContainer;
class EntityDatabaseImpl;

struct EntityLocation {
    std::size_t chunk_idx;
    std::size_t entity_idx;
};

class ComponentLayout {
public:
    ComponentLayout(const EntityArchetype2& archetype, const EntityDatabaseImpl& entity_database);

    std::size_t size() const;

    bool has_component(TypeId component_type) const;
    std::size_t component_idx(TypeId component_type) const;
    EntityComponentData component_desc(std::size_t idx) const;

    std::span<const EntityComponentData> component_descriptors() const;

    EntityArchetype2 archetype() const;

private:
    EntityArchetype2 m_archetype;
    std::vector<EntityComponentData> m_component_descriptors;
};

class ComponentChunk {
public:
    ComponentChunk(EntityComponentData component_data, std::size_t capacity);
    ComponentChunk(const ComponentChunk& other) = delete;
    ComponentChunk(ComponentChunk&& other) noexcept = default;
    ~ComponentChunk();

    ComponentChunk& operator=(const ComponentChunk& other) = delete;
    ComponentChunk& operator=(ComponentChunk&& other) noexcept;

    std::size_t size() const;
    std::size_t capacity() const;

    std::size_t init();
    std::size_t init_move(void* src);
    std::size_t init_copy(const void* src);
    void erase(std::size_t idx);

    void read(std::size_t idx, void* dst) const;
    void write_move(std::size_t idx, void* src);
    void write_copy(std::size_t idx, const void* src);

    void write_uninitialized_init(std::size_t idx);
    void write_uninitialized_move(std::size_t idx, void* src);
    void write_uninitialized_copy(std::size_t idx, const void* src);

    void* fetch_unchecked(std::size_t idx);
    const void* fetch_unchecked(std::size_t idx) const;

private:
    std::size_t phantom_init();

    std::size_t m_size;
    std::size_t m_capacity;
    EntityComponentData m_component_data;
    std::unique_ptr<std::byte[], AlignedDeleter<std::byte>> m_data;
};

class EntityChunk {
public:
    EntityChunk(const ComponentLayout& layout);
    EntityChunk(const EntityChunk& other) = delete;
    EntityChunk(EntityChunk&& other) noexcept = default;

    EntityChunk& operator=(const EntityChunk& other) = delete;
    EntityChunk& operator=(EntityChunk&& other) noexcept;

    std::size_t size() const;
    std::size_t capacity() const;
    std::size_t component_size() const;

    bool has_entity(Entity entity) const;
    bool has_component(TypeId component_type) const;

    std::size_t entity_idx(Entity entity) const;
    std::size_t component_idx(TypeId component_type) const;

    std::size_t init(Entity entity);
    std::size_t init_move(Entity entity, EntityContainer& entity_container, EntityLocation entity_location);
    std::size_t init_copy(Entity entity, const EntityContainer& entity_container, EntityLocation entity_location);
    void erase(std::size_t entity_idx);

    void read(std::size_t entity_idx, std::size_t component_idx, void* dst) const;
    void write_move(std::size_t entity_idx, std::size_t component_idx, void* src);
    void write_copy(std::size_t entity_idx, std::size_t component_idx, const void* src);

    void* fetch_unchecked(std::size_t entity_idx, std::size_t component_idx);
    const void* fetch_unchecked(std::size_t entity_idx, std::size_t component_idx) const;

    std::span<const Entity> entities() const;

    EntityArchetype2 archetype() const;

private:
    std::size_t phantom_init(Entity entity);

    std::size_t m_size;
    std::reference_wrapper<const ComponentLayout> m_layout;
    std::vector<ComponentChunk> m_component_chunks;
    std::array<Entity, ENTITY_CHUNK_SIZE> m_entities;
};

class EntityContainer {
public:
    EntityContainer(const EntityArchetype2& archetype, const EntityDatabaseImpl& entity_database);

    std::size_t size() const;
    std::size_t capacity() const;
    std::size_t component_size() const;

    bool has_entity(Entity entity) const;
    bool has_component(TypeId component_type) const;

    EntityLocation entity_location(Entity entity) const;
    std::size_t component_idx(TypeId component_type) const;

    EntityLocation init(Entity entity);
    EntityLocation init_move(Entity entity, EntityContainer& entity_container, EntityLocation entity_location);
    EntityLocation init_copy(Entity entity, const EntityContainer& entity_container, EntityLocation entity_location);
    void erase(EntityLocation entity_location);

    void read(EntityLocation entity_location, std::size_t component_idx, void* dst) const;
    void write_move(EntityLocation entity_location, std::size_t component_idx, void* src);
    void write_copy(EntityLocation entity_location, std::size_t component_idx, const void* src);

    void* fetch_unchecked(EntityLocation entity_location, std::size_t component_idx);
    const void* fetch_unchecked(EntityLocation entity_location, std::size_t component_idx) const;

    EntityChunk& entity_chunk(std::size_t chunk_idx);
    const EntityChunk& entity_chunk(std::size_t chunk_idx) const;

    std::span<EntityChunk> entity_chunks();
    std::span<const EntityChunk> entity_chunks() const;

    EntityArchetype2 archetype() const;

private:
    struct ChunkCapacity {
        std::size_t chunk_idx;
        std::size_t chunk_capacity;
    };

    /// Returns the index of a suitable chunk and saves the entity.
    std::size_t phantom_init(Entity entity);

    std::size_t m_size;
    std::size_t m_capacity;
    std::size_t m_empty_chunks;
    std::size_t m_filled_chunks;
    ComponentLayout m_layout;
    std::vector<EntityChunk> m_entity_chunks;
    std::vector<ChunkCapacity> m_capacity_map;
    std::unordered_map<Entity, std::size_t, EntityHasher> m_entity_map;
};

}
