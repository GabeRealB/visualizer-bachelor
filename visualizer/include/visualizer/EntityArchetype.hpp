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
    void (*createFunc)(void* ptr);
    void (*copyFunc)(const void* src, void* dst);
    void (*moveFunc)(void* src, void* dst);
    void (*copyUninitializedFunc)(const void* src, void* dst);
    void (*moveUninitializedFunc)(void* src, void* dst);
    void (*destructorFunc)(const void* ptr);

    bool operator==(const EntityComponentData& other);

    template <typename T> requires NoCVRefs<T> static EntityComponentData create_desc();
};

class EntityArchetype2 {
public:
    EntityArchetype2() = default;
    EntityArchetype2(const EntityArchetype2&) = default;
    EntityArchetype2(EntityArchetype2&&) noexcept = default;

    EntityArchetype2(TypeId component_type);
    EntityArchetype2(std::span<const TypeId> component_types);

    EntityArchetype2& operator=(const EntityArchetype2&) = default;
    EntityArchetype2& operator=(EntityArchetype2&&) noexcept = default;

    bool operator==(const EntityArchetype2& other) const;

    std::size_t size() const noexcept;
    std::size_t hash() const noexcept;
    const std::string& identifier() const noexcept;
    std::span<const TypeId> component_types() const noexcept;

    EntityArchetype2 with(TypeId component_type) const;
    EntityArchetype2 with(std::span<const TypeId> component_types) const;

    EntityArchetype2 without(TypeId component_type) const;
    EntityArchetype2 without(std::span<const TypeId> component_types) const;

    template <typename... Ts> requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> EntityArchetype2 with() const;
    template <typename... Ts> requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> EntityArchetype2 without() const;

private:
    std::size_t m_hash;
    std::string m_identifier;
    std::vector<TypeId> m_component_types;
};

struct EntityArchetype2Hasher {
    std::size_t operator()(const EntityArchetype2& k) const;
};

bool operator==(const EntityComponentData& lhs, const EntityComponentData& rhs);

}

#include <visualizer/EntityArchetype.impl>