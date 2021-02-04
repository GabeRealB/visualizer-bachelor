#include <visualizer/EntityDatabase.hpp>

#include <cassert>
#include <limits>
#include <mutex>

namespace Visualizer {

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
    ComponentType component_type, ComponentDescriptor component_desc)
{
    assert(!has_component(component_type));
    [[maybe_unused]] auto [pos, success] = m_component_descriptors.insert({ component_type, component_desc });
    assert(success);
    return component_type;
}

const ComponentDescriptor& EntityDatabaseImpl::fetch_component_desc(ComponentType component_type) const
{
    assert(has_component(component_type));
    return m_component_descriptors.at(component_type);
}

Entity EntityDatabaseImpl::init_entity(const EntityArchetype& archetype)
{
#ifndef NDEBUG
    for (auto component_type : archetype.component_types()) {
        assert(has_component(component_type));
    }
#endif
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

Entity EntityDatabaseImpl::init_entity_copy(Entity entity, const EntityArchetype& archetype)
{
#ifndef NDEBUG
    assert(has_entity(entity));
    for (auto component_type : archetype.component_types()) {
        assert(has_component(component_type));
    }
#endif
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

void EntityDatabaseImpl::move_entity(Entity entity, const EntityArchetype& archetype)
{
#ifndef NDEBUG
    assert(has_entity(entity));
    for (auto component_type : archetype.component_types()) {
        assert(has_component(component_type));
    }
#endif
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

EntityArchetype EntityDatabaseImpl::fetch_entity_archetype(Entity entity) const
{
    assert(has_entity(entity));
    return fetch_entity_container(entity).archetype();
}

template <typename T, typename It> void set_intersection_in_place(T& cont, It first2, It last2)
{
    auto it1{ std::begin(cont) };
    auto it2{ first2 };

    while (it1 != std::end(cont) && it2 != last2) {
        if (*it1 < *it2) {
            it1 = cont.erase(it1);
        } else if (*it2 < *it1) {
            ++it2;
        } else {
            ++it1;
            ++it2;
        }
    }

    cont.erase(it1, std::end(cont));
}

template <typename T, typename It> void set_union_in_place(T& cont, It first2, It last2)
{
    auto it1{ std::begin(cont) };
    auto it2{ first2 };

    while (it1 != std::end(cont) && it2 != last2) {
        if (*it1 < *it2) {
            ++it1;
        } else if (*it2 < *it1) {
            it1 = cont.insert(it1, *it2);
            ++it2;
        } else {
            ++it1;
            ++it2;
        }
    }

    cont.insert(it1, it2, last2);
}

template <typename T, typename It> void set_difference_in_place(T& cont, It first2, It last2)
{
    auto it1{ std::begin(cont) };
    auto it2{ first2 };

    while (it1 != std::end(cont) && it2 != last2) {
        if (*it1 < *it2) {
            ++it1;
        } else if (*it2 < *it1) {
            ++it2;
        } else {
            it1 = cont.erase(it1);
            ++it2;
        }
    }
}

EntityDBWindow EntityDatabaseImpl::query_db_window(const EntityDBQuery& query)
{
    auto required_components{ query.required_components() };
    auto prohibited_components{ query.prohibited_components() };
    auto optional_components{ query.optional_components() };

    std::vector<Entity> entities{};
    std::vector<std::vector<void*>> components{};
    std::unordered_map<ComponentType, std::size_t> component_type_map;

    components.reserve(required_components.size() + optional_components.size());
    component_type_map.reserve(required_components.size() + optional_components.size());
    for (std::size_t i{ 0 }; i < required_components.size(); ++i) {
        components.emplace_back();
        component_type_map.insert({ required_components[i], i });
    }
    for (std::size_t i{ 0 }; i < optional_components.size(); ++i) {
        components.emplace_back();
        component_type_map.insert({ optional_components[i], i + required_components.size() });
    }

    std::vector<EntityContainerId> required_container_ids{};
    if (required_components.size() > 0) {
        if (auto pos{ m_type_associations.find(required_components[0]) }; pos != m_type_associations.end()) {
            required_container_ids = { pos->second.begin(), pos->second.end() };
        }

        for (std::size_t i{ 1 }; i < required_components.size() && !required_container_ids.empty(); ++i) {
            if (auto pos{ m_type_associations.find(required_components[i]) }; pos != m_type_associations.end()) {
                set_intersection_in_place(required_container_ids, pos->second.begin(), pos->second.end());
            } else {
                required_container_ids.clear();
            }
        }
    }

    if (required_container_ids.empty()) {
        return EntityDBWindow{ std::move(entities), std::move(components), std::move(component_type_map) };
    }

    std::vector<EntityContainerId> prohibited_container_ids{};
    for (auto prohibited_component : prohibited_components) {
        if (auto pos{ m_type_associations.find(prohibited_component) }; pos != m_type_associations.end()) {
            set_union_in_place(prohibited_container_ids, pos->second.begin(), pos->second.end());
        }
    }

    if (!prohibited_container_ids.empty()) {
        set_difference_in_place(
            required_container_ids, prohibited_container_ids.begin(), prohibited_container_ids.end());
    }

    if (required_container_ids.empty()) {
        return EntityDBWindow{ std::move(entities), std::move(components), std::move(component_type_map) };
    }

    std::vector<std::size_t> component_indices{};
    std::vector<bool> optional_component_presence{};

    for (auto container_id : required_container_ids) {
        auto& entity_container{ m_entity_containers.at(container_id) };

        component_indices.reserve(required_components.size() + optional_components.size());
        optional_component_presence.reserve(optional_components.size());
        for (auto component_type : required_components) {
            auto component_idx{ entity_container.component_idx(component_type) };
            component_indices.push_back(component_idx);
        }
        for (auto component_type : optional_components) {
            if (entity_container.has_component(component_type)) {
                auto component_idx{ entity_container.component_idx(component_type) };
                component_indices.push_back(component_idx);
                optional_component_presence.push_back(true);
            } else {
                component_indices.push_back(0);
                optional_component_presence.push_back(false);
            }
        }

        for (auto& component_vec : components) {
            component_vec.reserve(component_vec.size() + entity_container.size());
        }

        for (auto& entity_chunk : entity_container.entity_chunks()) {
            for (auto& entity : entity_chunk.entities()) {
                entities.push_back(entity);
                auto entity_idx{ entity_chunk.entity_idx(entity) };

                for (std::size_t i{ 0 }; i < required_components.size(); ++i) {
                    auto component_ptr{ const_cast<void*>(
                        entity_chunk.fetch_unchecked(entity_idx, component_indices[i])) };
                    components[i].push_back(component_ptr);
                }
                for (std::size_t i{ 0 }; i < optional_components.size(); ++i) {
                    if (optional_component_presence[i]) {
                        auto component_ptr{ const_cast<void*>(entity_chunk.fetch_unchecked(
                            entity_idx, component_indices[i + required_components.size()])) };
                        components[i].push_back(component_ptr);
                    } else {
                        components[i + required_components.size()].push_back(nullptr);
                    }
                }
            }
        }

        component_indices.clear();
        optional_component_presence.clear();
    }

    return EntityDBWindow{ std::move(entities), std::move(components), std::move(component_type_map) };
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

EntityContainer& EntityDatabaseImpl::fetch_or_init_entity_container(const EntityArchetype& archetype)
{
    if (m_archetype_map.contains(archetype)) {
        return m_entity_containers.at(m_archetype_map.at(archetype));
    } else {
#ifndef NDEBUG
        for (auto component_type : archetype.component_types()) {
            assert(has_component(component_type));
        }
#endif
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
    ComponentType component_type, ComponentDescriptor component_desc)
{
    return m_database.register_component_desc(component_type, component_desc);
}

const ComponentDescriptor& EntityDatabaseContext::fetch_component_desc(ComponentType component_type) const
{
    return m_database.fetch_component_desc(component_type);
}

Entity EntityDatabaseContext::init_entity(const EntityArchetype& archetype)
{
    return m_database.init_entity(archetype);
}

Entity EntityDatabaseContext::init_entity(EntityBuilder&& entity_builder)
{
    return m_database.init_entity(std::move(entity_builder));
}

Entity EntityDatabaseContext::init_entity(const EntityBuilder& entity_builder)
{
    return m_database.init_entity(entity_builder);
}

Entity EntityDatabaseContext::init_entity_copy(Entity entity, const EntityArchetype& archetype)
{
    return m_database.init_entity_copy(entity, archetype);
}

void EntityDatabaseContext::erase_entity(Entity entity) { m_database.erase_entity(entity); }

void EntityDatabaseContext::move_entity(Entity entity, const EntityArchetype& archetype)
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

EntityArchetype EntityDatabaseContext::fetch_entity_archetype(Entity entity) const
{
    return m_database.fetch_entity_archetype(entity);
}

EntityDBWindow EntityDatabaseContext::query_db_window(const EntityDBQuery& query)
{
    return m_database.query_db_window(query);
}

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

EntityArchetype EntityDatabaseLazyContext::fetch_entity_archetype(Entity entity) const
{
    return m_database.fetch_entity_archetype(entity);
}

}