#pragma once

#include <concepts>
#include <optional>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

#include <visualizer/Entity.hpp>
#include <visualizer/EntityArchetype.hpp>
#include <visualizer/TupleUtils.hpp>
#include <visualizer/TypeId.hpp>
#include <visualizer/UniqueTypes.hpp>
#include <visualizer/World.hpp>

namespace Visualizer {

class EntityQueryResult;
class ComponentManager;
class EntityDatabaseContext;
class EntityDatabaseLazyContext;

class EntityQuery {
public:
    EntityQuery() = default;
    EntityQuery(const EntityArchetype& archetype);

    std::span<const TypeId> withTypes() const;
    std::span<const TypeId> withoutTypes() const;

    EntityQuery& with(TypeId typeId);
    EntityQuery& without(TypeId typeId);

    EntityQuery& with(std::span<const TypeId> types);
    EntityQuery& without(std::span<const TypeId> types);

    EntityQueryResult query(World& world);
    EntityQueryResult query(ComponentManager& componentManager);
    EntityQueryResult query(EntityDatabaseContext& database_context);
    EntityQueryResult query(EntityDatabaseLazyContext& database_context);

    template <typename... Ts> requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> EntityQuery& with();
    template <typename... Ts> requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> EntityQuery& without();

private:
    std::vector<TypeId> m_withTypes;
    std::vector<TypeId> m_withoutTypes;
};

class EntityQueryResult {
public:
    EntityQueryResult() = default;
    EntityQueryResult(const EntityQueryResult& other) = default;
    EntityQueryResult(EntityQueryResult&& other) noexcept = default;
    EntityQueryResult(std::vector<Entity> entities, std::vector<void*> components, const std::vector<TypeId>& types);

    EntityQueryResult& operator=(const EntityQueryResult& other) = default;
    EntityQueryResult& operator=(EntityQueryResult&& other) noexcept = default;

    std::size_t count() const;
    std::size_t componentCount() const;
    std::span<const Entity> entities() const;
    std::span<void*> components() const;
    std::optional<std::size_t> typeIndex(TypeId typeId) const;

    bool contains(TypeId typeId) const;
    bool contains(std::span<const TypeId> types) const;

    std::optional<std::tuple<Entity, std::span<void*>>> at(std::size_t idx) const;
    std::optional<std::tuple<Entity, std::span<void*>>> operator[](std::size_t idx) const;

    template <typename... Ts> requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> bool contains() const;
    template <typename... Ts>
    requires NoCVRefs<Ts...>&& UniqueTypes<Ts...> typename TupleTypeN<std::optional<std::size_t>, sizeof...(Ts)>::type
    typeIndex() const;
    template <typename... Ts>
    requires UniqueTypes<Ts...> std::optional<std::tuple<const Entity, Ts*...>> at(std::size_t idx) const;

    template <typename... Ts, typename Pred>
    requires NoCVRefs<Ts...>&& UniqueTypes<Ts...>&& std::predicate<Pred, const Ts*...> EntityQueryResult& filter(
        Pred&& fn);

    template <typename... Ts, typename Fn>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, Ts*...> void forEach(Fn&& fn) const;
    template <typename... Ts, typename Fn>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, Entity, Ts*...> void forEachWithEntity(Fn&& fn) const;

    template <typename... Ts, typename Fn>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, std::size_t, Ts*...> void iterate(Fn&& fn) const;
    template <typename... Ts, typename Fn>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, std::size_t, Entity, Ts*...> void iterateWithEntity(Fn&& fn) const;

    template <typename... Ts, typename Fn, typename Pred>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, Ts*...>&&
        std::predicate<Pred, const typename std::remove_cvref_t<Ts*>...> void
        forEach(Fn&& fn, Pred&& pred) const;
    template <typename... Ts, typename Fn, typename Pred>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, Entity, Ts*...>&&
        std::predicate<Pred, Entity, const typename std::remove_cvref_t<Ts*>...> void
        forEachWithEntity(Fn&& fn, Pred&& pred) const;

private:
    template <typename... Ts, typename Pred, std::size_t... Is>
    requires NoCVRefs<Ts...>&& UniqueTypes<Ts...>&& std::predicate<Pred, const Ts*...> EntityQueryResult& filter(
        Pred&& fn, std::index_sequence<Is...>);

    template <typename... Ts, typename Fn, std::size_t... Is>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, Ts*...> void forEach(Fn&& fn, std::index_sequence<Is...>) const;
    template <typename... Ts, typename Fn, std::size_t... Is>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, Entity, Ts*...> void forEachWithEntity(
        Fn&& fn, std::index_sequence<Is...>) const;

    template <typename... Ts, typename Fn, std::size_t... Is>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, std::size_t, Ts*...> void iterate(
        Fn&& fn, std::index_sequence<Is...>) const;
    template <typename... Ts, typename Fn, std::size_t... Is>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, std::size_t, Entity, Ts*...> void iterateWithEntity(
        Fn&& fn, std::index_sequence<Is...>) const;

    template <typename... Ts, typename Fn, typename Pred, std::size_t... Is>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, Ts*...>&&
        std::predicate<Pred, const typename std::remove_cvref_t<Ts*>...> void
        forEach(Fn&& fn, Pred&& pred, std::index_sequence<Is...>) const;
    template <typename... Ts, typename Fn, typename Pred, std::size_t... Is>
    requires UniqueTypes<Ts...>&& std::invocable<Fn, Entity, Ts*...>&&
        std::predicate<Pred, Entity, const typename std::remove_cvref_t<Ts*>...> void
        forEachWithEntity(Fn&& fn, Pred&& pred, std::index_sequence<Is...>) const;

    std::vector<Entity> m_entities;
    std::vector<void*> m_components;
    std::unordered_map<TypeId, std::size_t> m_typeIndexMap;
};

}

#include <visualizer/EntityQuery.impl>