#pragma once

#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <visualizer/TupleUtils.hpp>
#include <visualizer/TypeId.hpp>
#include <visualizer/UniqueTypes.hpp>

namespace Visualizer {

struct EntityComponentData {
    TypeId id;
    std::size_t size;
    std::size_t alignment;
    void (*createFunc)(void*);
    void (*copyFunc)(const void*, void*);
    void (*moveFunc)(void*, void*);
    void (*destructorFunc)(const void*);

    bool operator==(const EntityComponentData& other);
};

class EntityArchetype {
public:
    EntityArchetype() = default;
    EntityArchetype(const EntityArchetype&) = default;
    EntityArchetype(EntityArchetype&&) noexcept = default;

    EntityArchetype& operator=(const EntityArchetype&) = default;
    EntityArchetype& operator=(EntityArchetype&&) noexcept = default;

    bool operator==(const EntityArchetype& other) const;

    std::size_t size() const;
    const std::string& identifier() const;
    std::span<const TypeId> types() const;
    std::span<const EntityComponentData> componentInfos() const;

    bool hasComponent(TypeId typeId) const;
    std::optional<EntityComponentData> componentInfo(TypeId typeId) const;

    template <typename... Ts> requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> static EntityArchetype create();
    template <typename... Ts>
    requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> static EntityArchetype with(const EntityArchetype& archetype);
    template <typename... Ts>
    requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> static EntityArchetype without(const EntityArchetype& archetype);

    template <typename T> requires NoCVRefs<T> bool hasComponent() const;
    template <typename T> requires NoCVRefs<T> std::optional<EntityComponentData> componentInfo() const;

private:
    static std::string generateArchetypeIdentifier(const std::vector<EntityComponentData>& components);

    template <typename T> requires NoCVRefs<T> static constexpr EntityComponentData generateEntityComponentData();

    std::string m_identifier;
    std::vector<TypeId> m_types;
    std::unordered_map<TypeId, std::size_t> m_components;
    std::vector<EntityComponentData> m_componentInfos;
};

struct EntityArchetypeHasher {
    std::size_t operator()(const EntityArchetype& k) const;
};

bool operator==(const EntityComponentData& lhs, const EntityComponentData& rhs);

}

#include <visualizer/EntityArchetype.impl>