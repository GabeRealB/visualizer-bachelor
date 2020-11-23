#include <visualizer/EntityArchetype.hpp>

namespace Visualizer {

bool EntityComponentData::operator==(const EntityComponentData& other)
{
    return (size == other.size && alignment == other.alignment && createFunc == other.createFunc
        && copyFunc == other.copyFunc && moveFunc == other.moveFunc && destructorFunc == other.destructorFunc);
}

/**************************************************************************************************
 **************************************** EntityArchetype2 ****************************************
 **************************************************************************************************/

EntityArchetype2::EntityArchetype2(TypeId component_type)
    : EntityArchetype2{ std::span<const TypeId>{ &component_type, 1 } }
{
}

EntityArchetype2::EntityArchetype2(std::span<const TypeId> component_types)
    : m_hash{}
    , m_identifier{}
    , m_component_types{}
{
    m_component_types.reserve(component_types.size());

    for (auto component_type : component_types) {
        auto insertion_pos{ std::upper_bound(m_component_types.begin(), m_component_types.end(), component_type) };
        if (insertion_pos == m_component_types.end() || *insertion_pos != component_type) {
            m_component_types.insert(insertion_pos, component_type);
        }
    }

    m_identifier.reserve(m_component_types.size() * 20);

    for (auto component_type : m_component_types) {
        m_identifier.append(std::to_string(component_type));
        m_identifier.append("|");
    }

    m_component_types.shrink_to_fit();
    m_identifier.shrink_to_fit();

    m_hash = std::hash<std::string>{}(m_identifier);
}

bool EntityArchetype2::operator==(const EntityArchetype2& other) const
{
    return m_hash == other.m_hash && m_component_types == other.m_component_types;
}

std::size_t EntityArchetype2::size() const noexcept { return m_component_types.size(); }

std::size_t EntityArchetype2::hash() const noexcept { return m_hash; }

const std::string& EntityArchetype2::identifier() const noexcept { return m_identifier; }

std::span<const TypeId> EntityArchetype2::component_types() const noexcept
{
    return std::span<const TypeId>{ m_component_types.data(), size() };
}

EntityArchetype2 EntityArchetype2::with(TypeId component_type) const
{
    return with(std::span<const TypeId>{ &component_type, 1 });
}

EntityArchetype2 EntityArchetype2::with(std::span<const TypeId> component_types) const
{
    EntityArchetype2 archetype{};
    archetype.m_component_types = m_component_types;

    for (auto component_type : component_types) {
        auto insertion_pos{ std::upper_bound(
            archetype.m_component_types.begin(), archetype.m_component_types.end(), component_type) };
        if (insertion_pos == archetype.m_component_types.end() || *insertion_pos != component_type) {
            archetype.m_component_types.insert(insertion_pos, component_type);
        }
    }

    archetype.m_identifier.reserve(archetype.m_component_types.size() * 20);

    for (auto component_type : archetype.m_component_types) {
        archetype.m_identifier.append(std::to_string(component_type));
        archetype.m_identifier.append("|");
    }

    archetype.m_component_types.shrink_to_fit();
    archetype.m_identifier.shrink_to_fit();

    archetype.m_hash = std::hash<std::string>{}(archetype.m_identifier);

    return archetype;
}

EntityArchetype2 EntityArchetype2::without(TypeId component_type) const
{
    return without(std::span<const TypeId>{ &component_type, 1 });
}

EntityArchetype2 EntityArchetype2::without(std::span<const TypeId> component_types) const
{
    EntityArchetype2 archetype{};
    archetype.m_component_types = m_component_types;

    for (auto component_type : component_types) {
        auto deletion_pos{ std::upper_bound(
            archetype.m_component_types.begin(), archetype.m_component_types.end(), component_type) };
        if (deletion_pos != archetype.m_component_types.end()) {
            archetype.m_component_types.erase(deletion_pos);
        }
    }

    archetype.m_identifier.reserve(archetype.m_component_types.size() * 20);

    for (auto component_type : archetype.m_component_types) {
        archetype.m_identifier.append(std::to_string(component_type));
        archetype.m_identifier.append("|");
    }

    archetype.m_component_types.shrink_to_fit();
    archetype.m_identifier.shrink_to_fit();

    archetype.m_hash = std::hash<std::string>{}(archetype.m_identifier);

    return archetype;
}

/**************************************************************************************************
 ************************************* EntityArchetype2Hasher *************************************
 **************************************************************************************************/

std::size_t EntityArchetype2Hasher::operator()(const EntityArchetype2& k) const { return k.hash(); }
}
