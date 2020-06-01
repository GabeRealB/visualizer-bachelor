#include <visualizer/EntityArchetype.hpp>

namespace Visualizer {

bool EntityComponentData::operator==(const EntityComponentData& other)
{
    return (size == other.size && alignment == other.alignment && createFunc == other.createFunc
        && copyFunc == other.copyFunc && moveFunc == other.moveFunc && destructorFunc == other.destructorFunc);
}

/**************************************************************************************************
 **************************************** EntityArchetype ****************************************
 **************************************************************************************************/

bool EntityArchetype::operator==(const EntityArchetype& other) const { return (m_identifier == other.m_identifier); }

std::size_t EntityArchetype::size() const { return m_componentInfos.size(); }

const std::string& EntityArchetype::identifier() const { return m_identifier; }

std::span<const TypeId> EntityArchetype::types() const { return { m_types.data(), m_types.size() }; }

std::span<const EntityComponentData> EntityArchetype::componentInfos() const
{
    return { m_componentInfos.data(), m_componentInfos.size() };
}

bool EntityArchetype::hasComponent(TypeId typeId) const { return m_components.contains(typeId); }

std::optional<EntityComponentData> EntityArchetype::componentInfo(TypeId typeId) const
{
    auto pos{ m_components.find(typeId) };
    if (pos == m_components.end()) {
        return std::nullopt;
    } else {
        return m_componentInfos[pos->second];
    }
}

std::string EntityArchetype::generateArchetypeIdentifier(const std::vector<EntityComponentData>& components)
{
    std::string res{};
    res.reserve(components.size() * 20);

    for (std::size_t i = 0; i < components.size(); ++i) {
        auto& component{ components[i] };
        res.append(std::to_string(component.id));
        res.append("-");
        res.append(std::to_string(component.size));
        res.append("|");
    }

    return res;
}

/**************************************************************************************************
 ************************************* EntityArchetypeHasher *************************************
 **************************************************************************************************/

std::size_t EntityArchetypeHasher::operator()(const EntityArchetype& k) const
{
    return std::hash<std::string>{}(k.identifier());
}

}
