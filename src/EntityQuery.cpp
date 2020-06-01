#include <visualizer/EntityQuery.hpp>

#include <algorithm>

#include <visualizer/ComponentManager.hpp>

namespace Visualizer {

EntityQuery::EntityQuery(const EntityArchetype& archetype)
    : m_withTypes{}
    , m_withoutTypes{}
{
    auto types{ archetype.types() };
    m_withTypes = { types.begin(), types.end() };
}

std::span<const TypeId> EntityQuery::withTypes() const { return { m_withTypes.data(), m_withTypes.size() }; }

std::span<const TypeId> EntityQuery::withoutTypes() const { return { m_withoutTypes.data(), m_withoutTypes.size() }; }

EntityQuery& EntityQuery::with(TypeId typeId)
{
    auto withContains{ std::binary_search(m_withTypes.begin(), m_withTypes.end(), typeId) };
    if (!withContains) {
        m_withTypes.insert(std::upper_bound(m_withTypes.begin(), m_withTypes.end(), typeId), typeId);
    }

    auto withoutPos{ std::upper_bound(m_withoutTypes.begin(), m_withoutTypes.end(), typeId) };
    if (withoutPos != m_withoutTypes.end()) {
        m_withoutTypes.erase(withoutPos);
    }

    return *this;
}

EntityQuery& EntityQuery::without(TypeId typeId)
{
    auto withoutContains{ std::binary_search(m_withoutTypes.begin(), m_withoutTypes.end(), typeId) };
    if (!withoutContains) {
        m_withoutTypes.insert(std::upper_bound(m_withoutTypes.begin(), m_withoutTypes.end(), typeId), typeId);
    }

    auto withPos{ std::upper_bound(m_withTypes.begin(), m_withTypes.end(), typeId) };
    if (withPos != m_withTypes.end()) {
        m_withTypes.erase(withPos);
    }

    return *this;
}

EntityQuery& EntityQuery::with(std::span<const TypeId> types)
{
    for (auto type : types) {
        with(type);
    }
    return *this;
}

EntityQuery& EntityQuery::without(std::span<const TypeId> types)
{
    for (auto type : types) {
        without(type);
    }
    return *this;
}

EntityQueryResult EntityQuery::query(World& world)
{
    auto componentManager{ world.getManager<ComponentManager>() };
    return query(*componentManager);
}

EntityQueryResult EntityQuery::query(ComponentManager& componentManager) { return componentManager.query(*this); }

EntityQueryResult::EntityQueryResult(
    std::vector<Entity> entities, std::vector<void*> components, const std::vector<TypeId>& types)
    : m_entities{ std::move(entities) }
    , m_components{ std::move(components) }
    , m_typeIndexMap{}
{
    for (std::size_t i = 0; i < types.size(); ++i) {
        m_typeIndexMap.insert({ types[i], i });
    }
}

std::size_t EntityQueryResult::count() const { return m_entities.size(); }

std::size_t EntityQueryResult::componentCount() const { return m_components.size() / count(); }

std::span<const Entity> EntityQueryResult::entities() const { return { m_entities.data(), m_entities.size() }; }

std::span<void*> EntityQueryResult::components() const
{
    return { const_cast<void**>(m_components.data()), m_components.size() };
}

std::optional<std::size_t> EntityQueryResult::typeIndex(TypeId typeId) const
{
    if (auto pos{ m_typeIndexMap.find(typeId) }; pos != m_typeIndexMap.end()) {
        return pos->second;
    } else {
        return std::nullopt;
    }
}

bool EntityQueryResult::contains(TypeId typeId) const { return m_typeIndexMap.contains(typeId); }

bool EntityQueryResult::contains(std::span<const TypeId> types) const
{
    for (auto type : types) {
        if (!contains(type)) {
            return false;
        }
    }

    return true;
}

std::optional<std::tuple<Entity, std::span<void*>>> EntityQueryResult::at(std::size_t idx) const
{
    if (idx >= m_entities.size()) {
        return std::nullopt;
    } else {
        auto entity{ m_entities[idx] };
        auto componentSpan{ components() };
        auto numComponents{ componentCount() };

        std::tuple<Entity, std::span<void*>> result{ entity,
            componentSpan.subspan(idx * numComponents, numComponents) };
        return result;
    }
}

std::optional<std::tuple<Entity, std::span<void*>>> EntityQueryResult::operator[](std::size_t idx) const
{
    return at(idx);
}

}