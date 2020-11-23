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

struct ComponentDescriptor {
    TypeId id;
    std::size_t size;
    std::size_t alignment;
    void (*createFunc)(void* ptr);
    void (*copyFunc)(const void* src, void* dst);
    void (*moveFunc)(void* src, void* dst);
    void (*copyUninitializedFunc)(const void* src, void* dst);
    void (*moveUninitializedFunc)(void* src, void* dst);
    void (*destructorFunc)(const void* ptr);

    bool operator==(const ComponentDescriptor& other);

    template <typename T> requires NoCVRefs<T> static ComponentDescriptor create_desc();
};

class EntityArchetype {
public:
    EntityArchetype() = default;
    EntityArchetype(const EntityArchetype&) = default;
    EntityArchetype(EntityArchetype&&) noexcept = default;

    EntityArchetype(TypeId component_type);
    EntityArchetype(std::span<const TypeId> component_types);

    EntityArchetype& operator=(const EntityArchetype&) = default;
    EntityArchetype& operator=(EntityArchetype&&) noexcept = default;

    bool operator==(const EntityArchetype& other) const;

    std::size_t size() const noexcept;
    std::size_t hash() const noexcept;
    const std::string& identifier() const noexcept;
    std::span<const TypeId> component_types() const noexcept;

    EntityArchetype with(TypeId component_type) const;
    EntityArchetype with(std::span<const TypeId> component_types) const;

    EntityArchetype without(TypeId component_type) const;
    EntityArchetype without(std::span<const TypeId> component_types) const;

    template <typename... Ts> requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> EntityArchetype with() const;
    template <typename... Ts> requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> EntityArchetype without() const;

private:
    std::size_t m_hash;
    std::string m_identifier;
    std::vector<TypeId> m_component_types;
};

struct EntityArchetypeHasher {
    std::size_t operator()(const EntityArchetype& k) const;
};

}

#include <visualizer/EntityArchetype.impl>