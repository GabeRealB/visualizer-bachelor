#include <visualizer/EntityDBQuery.hpp>

#include <algorithm>
#include <cassert>

#include <visualizer/EntityDatabase.hpp>

namespace Visualizer {

/**************************************************************************************************
 ***************************************** EntityDBQuery *****************************************
 **************************************************************************************************/

EntityDBQuery::EntityDBQuery(const EntityArchetype& archetype)
    : m_required_components{}
    , m_prohibited_components{}
    , m_optional_components{}
{
    auto archetype_components{ archetype.component_types() };
    m_required_components = { archetype_components.begin(), archetype_components.end() };
}

EntityDBQuery& EntityDBQuery::with_component(ComponentType component_type)
{
    auto required_pos{ std::lower_bound(m_required_components.begin(), m_required_components.end(), component_type) };
    auto prohibited_pos{ std::lower_bound(
        m_prohibited_components.begin(), m_prohibited_components.end(), component_type) };
    auto optional_pos{ std::lower_bound(m_optional_components.begin(), m_optional_components.end(), component_type) };

    if (required_pos == m_required_components.end() || *required_pos != component_type) {
        m_required_components.insert(required_pos, component_type);
    }

    if (prohibited_pos != m_prohibited_components.end() && *prohibited_pos == component_type) {
        m_prohibited_components.erase(prohibited_pos);
    }

    if (optional_pos != m_optional_components.end() && *optional_pos == component_type) {
        m_optional_components.erase(optional_pos);
    }

    return *this;
}

EntityDBQuery& EntityDBQuery::without_component(ComponentType component_type)
{
    auto required_pos{ std::lower_bound(m_required_components.begin(), m_required_components.end(), component_type) };
    auto prohibited_pos{ std::lower_bound(
        m_prohibited_components.begin(), m_prohibited_components.end(), component_type) };
    auto optional_pos{ std::lower_bound(m_optional_components.begin(), m_optional_components.end(), component_type) };

    if (required_pos != m_required_components.end() && *required_pos == component_type) {
        m_required_components.erase(required_pos);
    }

    if (prohibited_pos == m_prohibited_components.end() || *prohibited_pos != component_type) {
        m_prohibited_components.insert(prohibited_pos, component_type);
    }

    if (optional_pos != m_optional_components.end() && *optional_pos == component_type) {
        m_optional_components.erase(optional_pos);
    }

    return *this;
}

EntityDBQuery& EntityDBQuery::with_optional_component(ComponentType component_type)
{
    auto required_pos{ std::lower_bound(m_required_components.begin(), m_required_components.end(), component_type) };
    auto prohibited_pos{ std::lower_bound(
        m_prohibited_components.begin(), m_prohibited_components.end(), component_type) };
    auto optional_pos{ std::lower_bound(m_optional_components.begin(), m_optional_components.end(), component_type) };

    if (required_pos != m_required_components.end() && *required_pos == component_type) {
        m_required_components.erase(required_pos);
    }

    if (prohibited_pos != m_prohibited_components.end() && *prohibited_pos == component_type) {
        m_prohibited_components.erase(prohibited_pos);
    }

    if (optional_pos == m_optional_components.end() || *optional_pos != component_type) {
        m_optional_components.insert(optional_pos, component_type);
    }

    return *this;
}

EntityDBQuery& EntityDBQuery::with_component(std::span<const ComponentType> component_types)
{
    for (auto component : component_types) {
        with_component(component);
    }
    return *this;
}

EntityDBQuery& EntityDBQuery::without_component(std::span<const ComponentType> component_types)
{
    for (auto component : component_types) {
        without_component(component);
    }
    return *this;
}

EntityDBQuery& EntityDBQuery::with_optional_component(std::span<const ComponentType> component_types)
{
    for (auto component : component_types) {
        with_optional_component(component);
    }
    return *this;
}

std::span<const ComponentType> EntityDBQuery::required_components() const
{
    return std::span<const ComponentType>{ m_required_components.data(), m_required_components.size() };
}

std::span<const ComponentType> EntityDBQuery::prohibited_components() const
{
    return std::span<const ComponentType>{ m_prohibited_components.data(), m_prohibited_components.size() };
}

std::span<const ComponentType> EntityDBQuery::optional_components() const
{
    return std::span<const ComponentType>{ m_optional_components.data(), m_optional_components.size() };
}

EntityDBWindow EntityDBQuery::query_db_window(EntityDatabaseContext& database_context)
{
    return database_context.query_db_window(*this);
}

/**************************************************************************************************
 ***************************************** EntityDBWindow *****************************************
 **************************************************************************************************/

EntityDBWindow::EntityDBWindow(std::vector<Entity>&& entities, std::vector<std::vector<void*>>&& components,
    std::unordered_map<ComponentType, std::size_t>&& component_type_map)
    : m_entities{ std::move(entities) }
    , m_components{ std::move(components) }
    , m_component_type_map{ std::move(component_type_map) }
    , m_entity_index_map{}
{
#ifndef NDEBUG
    assert(m_components.size() == m_component_type_map.size());
    for (const auto& component_vec : m_components) {
        assert(component_vec.size() == m_entities.size());
    }
#endif

    m_entity_index_map.reserve(m_entities.size());
    for (std::size_t i{ 0 }; i < m_entities.size(); ++i) {
        m_entity_index_map.insert({ m_entities[i], i });
    }
}

std::size_t EntityDBWindow::size() const { return m_entities.size(); }

std::size_t EntityDBWindow::component_size() const { return m_components.size(); }

std::size_t EntityDBWindow::entity_idx(Entity entity) const
{
    assert(has_entity(entity));
    return m_entity_index_map.at(entity);
}

std::size_t EntityDBWindow::component_idx(ComponentType component_type) const
{
    assert(has_component(component_type));
    return m_component_type_map.at(component_type);
}

bool EntityDBWindow::has_entity(Entity entity) const { return m_entity_index_map.contains(entity); }

bool EntityDBWindow::has_component(ComponentType component_type) const
{
    return m_component_type_map.contains(component_type);
}

bool EntityDBWindow::entity_has_component(Entity entity, ComponentType component_type) const
{
    assert(has_entity(entity));
    assert(has_component(component_type));

    const auto entity_index{ entity_idx(entity) };
    const auto component_index{ component_idx(component_type) };
    return fetch_component_unchecked(entity_index, component_index) != nullptr;
}

void* EntityDBWindow::fetch_component_unchecked(std::size_t entity_idx, std::size_t component_idx)
{
    assert(entity_idx < m_entities.size());
    assert(component_idx < m_components.size());
    return m_components[component_idx][entity_idx];
}

const void* EntityDBWindow::fetch_component_unchecked(std::size_t entity_idx, std::size_t component_idx) const
{
    assert(entity_idx < m_entities.size());
    assert(component_idx < m_components.size());
    return m_components[component_idx][entity_idx];
}

std::span<const Entity> EntityDBWindow::entity_span() const
{
    return std::span<const Entity>{ m_entities.data(), m_entities.size() };
}

std::span<void*> EntityDBWindow::component_span(std::size_t component_idx)
{
    assert(component_idx < m_components.size());
    return std::span<void*>{ m_components[component_idx].data(), m_components[component_idx].size() };
}

std::span<void* const> EntityDBWindow::component_span(std::size_t component_idx) const
{
    assert(component_idx < m_components.size());
    return std::span<void* const>{ m_components[component_idx].data(), m_components[component_idx].size() };
}

}