#include <visualizer/EntityContainer.hpp>

#include <algorithm>
#include <cassert>

#include <visualizer/ComponentManager.hpp>

namespace Visualizer {

/**************************************************************************************************
 **************************************** ComponentLayout ****************************************
 **************************************************************************************************/

ComponentLayout::ComponentLayout(const EntityArchetype2& archetype, const EntityDatabaseImpl& entity_database)
    : m_archetype{ archetype }
    , m_component_descriptors{}
{
    std::vector<TypeId> component_types{};
    component_types.reserve(archetype.size());
    m_component_descriptors.reserve(archetype.size());

    for (const auto component_type : archetype.component_types()) {
        component_types.insert(
            std::upper_bound(component_types.begin(), component_types.end(), component_type), component_type);
    }

    for (const auto component_type : component_types) {
        m_component_descriptors.push_back(entity_database.fetch_component_desc(component_type));
    }
}

std::size_t ComponentLayout::size() const { return m_component_descriptors.size(); }

bool ComponentLayout::has_component(TypeId component_type) const
{
    return std::find_if(m_component_descriptors.begin(), m_component_descriptors.end(),
               [component_type](const EntityComponentData& desc) { return desc.id == component_type; })
        != m_component_descriptors.end();
}

std::size_t ComponentLayout::component_idx(TypeId component_type) const
{
    assert(has_component(component_type));
    const auto component_pos{ std::find_if(m_component_descriptors.begin(), m_component_descriptors.end(),
        [component_type](const EntityComponentData& desc) { return desc.id == component_type; }) };
    return std::distance(m_component_descriptors.begin(), component_pos);
}

EntityComponentData ComponentLayout::component_desc(std::size_t idx) const
{
    assert(size() > idx);
    return m_component_descriptors[idx];
}

std::span<const EntityComponentData> ComponentLayout::component_descriptors() const
{
    return std::span<const EntityComponentData>{ m_component_descriptors.data(), size() };
}

EntityArchetype2 ComponentLayout::archetype() const { return m_archetype; }

/**************************************************************************************************
 ***************************************** ComponentChunk *****************************************
 **************************************************************************************************/

ComponentChunk::ComponentChunk(EntityComponentData component_data, std::size_t capacity)
    : m_size{ 0 }
    , m_capacity{ capacity }
    , m_component_data{ component_data }
    , m_data{ AlignedDeleter<std::byte>::allocate(component_data.alignment, component_data.size * capacity),
        AlignedDeleter<std::byte>{} }
{
    assert(m_data.get() != nullptr);
    assert(reinterpret_cast<std::uintptr_t>(m_data.get()) % component_data.alignment == 0);
}

ComponentChunk::~ComponentChunk()
{
    while (size() != 0) {
        erase(size() - 1);
    }
}

ComponentChunk& ComponentChunk::operator=(ComponentChunk&& other) noexcept
{
    if (this != &other) {
        m_size = std::exchange(other.m_size, 0);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_component_data = std::exchange(other.m_component_data, {});
        m_data = std::exchange(other.m_data, {});
    }

    return *this;
}

std::size_t ComponentChunk::size() const { return m_size; }

std::size_t ComponentChunk::capacity() const { return m_capacity; }

std::size_t ComponentChunk::init()
{
    assert(size() != capacity());
    auto component_idx{ phantom_init() };
    write_uninitialized_init(component_idx);
    return component_idx;
}

std::size_t ComponentChunk::init_move(void* src)
{
    assert(size() != capacity());
    auto component_idx{ phantom_init() };
    write_uninitialized_move(component_idx, src);
    return component_idx;
}

std::size_t ComponentChunk::init_copy(const void* src)
{
    assert(size() != capacity());
    auto component_idx{ phantom_init() };
    write_uninitialized_copy(component_idx, src);
    return component_idx;
}

void ComponentChunk::erase(std::size_t idx)
{
    assert(size() > idx);
    auto component_ptr{ fetch_unchecked(idx) };
    m_component_data.destructorFunc(component_ptr);

    // Close the hole by moving all components one place to the front.
    for (auto next_idx{ idx + 1 }; next_idx != size(); ++next_idx) {
        auto next_component_ptr{ fetch_unchecked(next_idx) };
        m_component_data.moveUninitializedFunc(next_component_ptr, component_ptr);
        component_ptr = next_component_ptr;
    }

    --m_size;
}

void ComponentChunk::read(std::size_t idx, void* dst) const
{
    assert(size() > idx);
    assert(dst != nullptr);
    auto component_ptr{ fetch_unchecked(idx) };
    m_component_data.copyUninitializedFunc(component_ptr, dst);
}

void ComponentChunk::write_move(std::size_t idx, void* src)
{
    assert(size() > idx);
    assert(src != nullptr);
    auto component_ptr{ fetch_unchecked(idx) };
    m_component_data.moveFunc(src, component_ptr);
}

void ComponentChunk::write_copy(std::size_t idx, const void* src)
{
    assert(size() > idx);
    assert(src != nullptr);
    auto component_ptr{ fetch_unchecked(idx) };
    m_component_data.copyFunc(src, component_ptr);
}

void ComponentChunk::write_uninitialized_init(std::size_t idx)
{
    assert(size() > idx);
    auto component_ptr{ fetch_unchecked(idx) };
    m_component_data.createFunc(component_ptr);
}

void ComponentChunk::write_uninitialized_move(std::size_t idx, void* src)
{
    assert(size() > idx);
    assert(src != nullptr);
    auto component_ptr{ fetch_unchecked(idx) };
    m_component_data.moveUninitializedFunc(src, component_ptr);
}

void ComponentChunk::write_uninitialized_copy(std::size_t idx, const void* src)
{
    assert(size() > idx);
    assert(src != nullptr);
    auto component_ptr{ fetch_unchecked(idx) };
    m_component_data.copyUninitializedFunc(src, component_ptr);
}

void* ComponentChunk::fetch_unchecked(std::size_t idx)
{
    assert(size() > idx);
    return static_cast<void*>(m_data.get() + (idx * m_component_data.size));
}

const void* ComponentChunk::fetch_unchecked(std::size_t idx) const
{
    assert(size() > idx);
    return static_cast<const void*>(m_data.get() + (idx * m_component_data.size));
}

std::size_t ComponentChunk::phantom_init()
{
    assert(size() != capacity());
    return m_size++;
}

/**************************************************************************************************
 ****************************************** EntityChunk ******************************************
 **************************************************************************************************/

EntityChunk::EntityChunk(const ComponentLayout& layout)
    : m_size{ 0 }
    , m_layout{ layout }
    , m_component_chunks{}
    , m_entities{}
{
    m_component_chunks.reserve(layout.size());

    for (auto component : layout.component_descriptors()) {
        m_component_chunks.emplace_back(component, capacity());
    }
}

EntityChunk& EntityChunk::operator=(EntityChunk&& other) noexcept
{
    if (this != &other) {
        m_size = std::exchange(other.m_size, 0);
        std::swap(m_layout, other.m_layout);
        m_component_chunks = std::exchange(other.m_component_chunks, {});
        m_entities = std::exchange(other.m_entities, {});
    }

    return *this;
}

std::size_t EntityChunk::size() const { return m_size; }

std::size_t EntityChunk::capacity() const { return ENTITY_CHUNK_SIZE; }

std::size_t EntityChunk::component_size() const { return m_layout.get().size(); }

bool EntityChunk::has_entity(Entity entity) const
{
    return std::find(m_entities.begin(), m_entities.begin() + size(), entity) != m_entities.begin() + size();
}

bool EntityChunk::has_component(TypeId component_type) const { return m_layout.get().has_component(component_type); }

std::size_t EntityChunk::entity_idx(Entity entity) const
{
    assert(has_entity(entity));
    const auto entity_pos{ std::find(m_entities.begin(), m_entities.begin() + size(), entity) };
    return std::distance(m_entities.begin(), entity_pos);
}

std::size_t EntityChunk::component_idx(TypeId component_type) const
{
    assert(has_component(component_type));
    return m_layout.get().component_idx(component_type);
}

std::size_t EntityChunk::init(Entity entity)
{
    assert(!has_entity(entity));
    auto entity_idx{ phantom_init(entity) };
    for (auto& component_chunk : m_component_chunks) {
        component_chunk.init();
    }
    return entity_idx;
}

std::size_t EntityChunk::init_move(Entity entity, EntityContainer& entity_container, EntityLocation entity_location)
{
    assert(!has_entity(entity));

    auto entity_idx{ phantom_init(entity) };

    auto& foreign_entity_chunk{ entity_container.entity_chunk(entity_location.chunk_idx) };

    for (std::size_t component_idx{ 0 }; component_idx < m_layout.get().size(); ++component_idx) {
        auto component_type{ m_layout.get().component_desc(component_idx).id };

        if (entity_container.has_component(component_type)) {
            auto foreign_component_idx{ entity_container.component_idx(component_type) };
            auto foreign_component_ptr{ entity_container.fetch_unchecked(entity_location, foreign_component_idx) };

            // Move the component.
            m_component_chunks[component_idx].init_move(foreign_component_ptr);

            // Reinitialize the component to allow destructor calls.
            foreign_entity_chunk.m_component_chunks[foreign_component_idx].write_uninitialized_init(
                entity_location.entity_idx);
        } else {
            m_component_chunks[component_idx].init();
        }
    }

    entity_container.erase(entity_location);

    return entity_idx;
}

std::size_t EntityChunk::init_copy(
    Entity entity, const EntityContainer& entity_container, EntityLocation entity_location)
{
    assert(!has_entity(entity));

    auto entity_idx{ phantom_init(entity) };

    for (std::size_t component_idx{ 0 }; component_idx < m_layout.get().size(); ++component_idx) {
        auto component_type{ m_layout.get().component_desc(component_idx).id };

        if (entity_container.has_component(component_type)) {
            auto foreign_component_idx{ entity_container.component_idx(component_type) };
            auto foreign_component_ptr{ entity_container.fetch_unchecked(entity_location, foreign_component_idx) };

            // Copy the component.
            m_component_chunks[component_idx].init_copy(foreign_component_ptr);
        } else {
            m_component_chunks[component_idx].init();
        }
    }

    return entity_idx;
}

void EntityChunk::erase(std::size_t entity_idx)
{
    assert(size() > entity_idx);

    for (auto& component_chunk : m_component_chunks) {
        component_chunk.erase(entity_idx);
    }

    std::move(m_entities.begin() + entity_idx + 1, m_entities.begin() + size(), m_entities.begin() + entity_idx);
    --m_size;
}

void EntityChunk::read(std::size_t entity_idx, std::size_t component_idx, void* dst) const
{
    assert(size() > entity_idx);
    assert(component_size() > component_idx);
    assert(dst != nullptr);
    m_component_chunks[component_idx].read(entity_idx, dst);
}

void EntityChunk::write_move(std::size_t entity_idx, std::size_t component_idx, void* src)
{
    assert(size() > entity_idx);
    assert(component_size() > component_idx);
    assert(src != nullptr);
    m_component_chunks[component_idx].write_move(entity_idx, src);
}

void EntityChunk::write_copy(std::size_t entity_idx, std::size_t component_idx, const void* src)
{
    assert(size() > entity_idx);
    assert(component_size() > component_idx);
    assert(src != nullptr);
    m_component_chunks[component_idx].write_copy(entity_idx, src);
}

void* EntityChunk::fetch_unchecked(std::size_t entity_idx, std::size_t component_idx)
{
    assert(size() > entity_idx);
    assert(component_size() > component_idx);
    return m_component_chunks[component_idx].fetch_unchecked(entity_idx);
}

const void* EntityChunk::fetch_unchecked(std::size_t entity_idx, std::size_t component_idx) const
{
    assert(size() > entity_idx);
    assert(component_size() > component_idx);
    return m_component_chunks[component_idx].fetch_unchecked(entity_idx);
}

std::span<const Entity> EntityChunk::entities() const { return std::span<const Entity>{ m_entities.begin(), size() }; }

EntityArchetype2 EntityChunk::archetype() const { return m_layout.get().archetype(); }

std::size_t EntityChunk::phantom_init(Entity entity)
{
    assert(!has_entity(entity));
    m_entities[size()] = entity;
    return m_size++;
}

/**************************************************************************************************
 **************************************** EntityContainer ****************************************
 **************************************************************************************************/

EntityContainer::EntityContainer(const EntityArchetype2& archetype, const EntityDatabaseImpl& entity_database)
    : m_size{ 0 }
    , m_capacity{ ENTITY_CHUNK_SIZE }
    , m_empty_chunks{ 1 }
    , m_filled_chunks{ 0 }
    , m_layout{ archetype, entity_database }
    , m_entity_chunks{}
    , m_capacity_map{}
    , m_entity_map{}
{
    m_entity_chunks.emplace_back(m_layout);
    m_capacity_map.push_back({ 0, ENTITY_CHUNK_SIZE });
    m_entity_map.reserve(ENTITY_CHUNK_SIZE);
}

std::size_t EntityContainer::size() const { return m_size; }

std::size_t EntityContainer::capacity() const { return m_capacity; }

std::size_t EntityContainer::component_size() const { return m_layout.size(); }

bool EntityContainer::has_entity(Entity entity) const { return m_entity_map.contains(entity); }

bool EntityContainer::has_component(TypeId component_type) const { return m_layout.has_component(component_type); }

EntityLocation EntityContainer::entity_location(Entity entity) const
{
    assert(has_entity(entity));
    auto chunk_idx{ m_entity_map.at(entity) };
    auto entity_idx{ m_entity_chunks[chunk_idx].entity_idx(entity) };
    return EntityLocation{ chunk_idx, entity_idx };
}

std::size_t EntityContainer::component_idx(TypeId component_type) const
{
    return m_layout.component_idx(component_type);
}

EntityLocation EntityContainer::init(Entity entity)
{
    assert(!has_entity(entity));
    auto chunk_idx{ phantom_init(entity) };
    auto entity_idx{ m_entity_chunks[chunk_idx].init(entity) };
    return EntityLocation{ chunk_idx, entity_idx };
}

EntityLocation EntityContainer::init_move(
    Entity entity, EntityContainer& entity_container, EntityLocation entity_location)
{
    assert(!has_entity(entity));
    auto chunk_idx{ phantom_init(entity) };
    auto entity_idx{ m_entity_chunks[chunk_idx].init_move(entity, entity_container, entity_location) };
    return EntityLocation{ chunk_idx, entity_idx };
}

EntityLocation EntityContainer::init_copy(
    Entity entity, const EntityContainer& entity_container, EntityLocation entity_location)
{
    assert(!has_entity(entity));
    auto chunk_idx{ phantom_init(entity) };
    auto entity_idx{ m_entity_chunks[chunk_idx].init_copy(entity, entity_container, entity_location) };
    return EntityLocation{ chunk_idx, entity_idx };
}

void EntityContainer::erase(EntityLocation entity_location)
{
    assert(m_entity_chunks.size() > entity_location.chunk_idx);
    auto removed_entity{ m_entity_chunks[entity_location.chunk_idx].entities()[entity_location.entity_idx] };
    m_entity_chunks[entity_location.chunk_idx].erase(entity_location.chunk_idx);
    m_entity_map.erase(removed_entity);
    m_size--;

    auto chunk_capacity{ ChunkCapacity{ entity_location.chunk_idx,
        m_entity_chunks[entity_location.chunk_idx].capacity() - m_entity_chunks[entity_location.chunk_idx].size() } };

    auto original_capacity_map_pos{ std::lower_bound(m_capacity_map.begin() + m_filled_chunks, m_capacity_map.end(),
        chunk_capacity, [](const ChunkCapacity& capacity, const ChunkCapacity& value) {
            if (capacity.chunk_capacity < value.chunk_capacity - 1) {
                return true;
            } else {
                return capacity.chunk_idx < value.chunk_idx;
            }
        }) };

    if (m_entity_chunks.size() == 1) {
        *original_capacity_map_pos = chunk_capacity;
    } else {
        auto capacity_map_pos{ std::lower_bound(m_capacity_map.begin() + m_filled_chunks, m_capacity_map.end(),
            chunk_capacity, [](const ChunkCapacity& capacity, const ChunkCapacity& value) {
                if (capacity.chunk_capacity < value.chunk_capacity) {
                    return true;
                } else {
                    return capacity.chunk_idx < value.chunk_idx;
                }
            }) };

        if (capacity_map_pos == original_capacity_map_pos) {
            *original_capacity_map_pos = chunk_capacity;
        } else {
            auto new_idx{ std::distance(m_capacity_map.begin(), capacity_map_pos) - 1 };
            m_capacity_map.erase(original_capacity_map_pos);
            m_capacity_map.insert(m_capacity_map.begin() + new_idx, chunk_capacity);
        }
    }

    if (chunk_capacity.chunk_capacity == ENTITY_CHUNK_SIZE) {
        if (++m_empty_chunks > ENTITY_CHUNK_ALLOCATION_BUFFER) {
            const auto chunk_to_remove{ m_capacity_map.back() };

            for (auto i = chunk_to_remove.chunk_idx + 1; i < m_entity_chunks.size(); ++i) {
                auto chunk_entities{ m_entity_chunks[i].entities() };

                for (auto entity : chunk_entities) {
                    m_entity_map.find(entity)->second--;
                }
            }

            m_entity_chunks.erase(m_entity_chunks.begin() + chunk_to_remove.chunk_idx);
            m_capacity_map.erase(m_capacity_map.end() - 1);
        }
    } else if (chunk_capacity.chunk_capacity == 1) {
        m_filled_chunks--;
    }
}

void EntityContainer::read(EntityLocation entity_location, std::size_t component_idx, void* dst) const
{
    assert(m_entity_chunks.size() > entity_location.chunk_idx);
    assert(dst != nullptr);
    m_entity_chunks[entity_location.chunk_idx].read(entity_location.entity_idx, component_idx, dst);
}

void EntityContainer::write_move(EntityLocation entity_location, std::size_t component_idx, void* src)
{
    assert(m_entity_chunks.size() > entity_location.chunk_idx);
    assert(src != nullptr);
    m_entity_chunks[entity_location.chunk_idx].write_move(entity_location.entity_idx, component_idx, src);
}

void EntityContainer::write_copy(EntityLocation entity_location, std::size_t component_idx, const void* src)
{
    assert(m_entity_chunks.size() > entity_location.chunk_idx);
    assert(src != nullptr);
    m_entity_chunks[entity_location.chunk_idx].write_copy(entity_location.entity_idx, component_idx, src);
}

void* EntityContainer::fetch_unchecked(EntityLocation entity_location, std::size_t component_idx)
{
    assert(m_entity_chunks.size() > entity_location.chunk_idx);
    return m_entity_chunks[entity_location.chunk_idx].fetch_unchecked(entity_location.entity_idx, component_idx);
}

const void* EntityContainer::fetch_unchecked(EntityLocation entity_location, std::size_t component_idx) const
{
    assert(m_entity_chunks.size() > entity_location.chunk_idx);
    return m_entity_chunks[entity_location.chunk_idx].fetch_unchecked(entity_location.entity_idx, component_idx);
}

EntityChunk& EntityContainer::entity_chunk(std::size_t chunk_idx)
{
    assert(m_entity_chunks.size() > chunk_idx);
    return m_entity_chunks[chunk_idx];
}

const EntityChunk& EntityContainer::entity_chunk(std::size_t chunk_idx) const
{
    assert(m_entity_chunks.size() > chunk_idx);
    return m_entity_chunks[chunk_idx];
}

EntityArchetype2 EntityContainer::archetype() const { return m_layout.archetype(); }

std::span<EntityChunk> EntityContainer::entity_chunks()
{
    return std::span<EntityChunk>{ m_entity_chunks.data(), m_entity_chunks.size() };
}

std::span<const EntityChunk> EntityContainer::entity_chunks() const
{
    return std::span<const EntityChunk>{ m_entity_chunks.data(), m_entity_chunks.size() };
}

std::size_t EntityContainer::phantom_init(Entity entity)
{
    assert(!has_entity(entity));
    m_size++;
    if (size() <= capacity()) {
        auto suitable_chunk_pos{ m_capacity_map.begin() + m_filled_chunks };
        auto chunk{ *suitable_chunk_pos };
        chunk.chunk_capacity--;

        auto new_chunk_pos{ std::lower_bound(m_capacity_map.begin() + m_filled_chunks, m_capacity_map.end(), chunk,
            [](const ChunkCapacity& capacity, const ChunkCapacity& value) {
                if (capacity.chunk_capacity < value.chunk_capacity) {
                    return true;
                } else {
                    return capacity.chunk_idx < value.chunk_idx;
                }
            }) };

        if (suitable_chunk_pos == new_chunk_pos) {
            *suitable_chunk_pos = chunk;
        } else {
            m_capacity_map.erase(suitable_chunk_pos);
            m_capacity_map.insert(new_chunk_pos, chunk);
        }

        if (chunk.chunk_capacity == ENTITY_CHUNK_SIZE - 1) {
            m_empty_chunks--;
        } else if (chunk.chunk_capacity == 0) {
            m_filled_chunks++;
        }

        m_entity_map.insert({ entity, chunk.chunk_idx });
        return chunk.chunk_idx;
    } else {
        const auto chunk_idx{ m_entity_chunks.size() };
        m_entity_chunks.emplace_back(m_layout);
        m_capacity_map.push_back(ChunkCapacity{ chunk_idx, ENTITY_CHUNK_SIZE - 1 });
        m_capacity += ENTITY_CHUNK_SIZE;

        m_entity_map.insert({ entity, chunk_idx });
        return chunk_idx;
    }
}

}