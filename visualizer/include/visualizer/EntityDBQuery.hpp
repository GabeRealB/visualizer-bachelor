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

using ComponentType = TypeId;

class EntityDBQuery;
class EntityDBWindow;
class EntityDatabaseContext;
class EntityDatabaseLazyContext;

template <typename Pred, typename... Ts>
concept EntityDBWindowPred = std::predicate<Pred, const Ts*...> || std::predicate<Pred, Entity, const Ts*...>;

template <typename Fn, typename... Ts>
concept EntityDBWindowIterateFn
    = std::invocable<Fn, std::size_t, Ts*...> || std::invocable<Fn, std::size_t, Entity, Ts*...>;

template <typename Fn, typename... Ts>
concept EntityDBWindowForEachFn = std::invocable<Fn, Ts*...> || std::invocable<Fn, Entity, Ts*...>;

class EntityDBQuery {
public:
    EntityDBQuery() = default;
    EntityDBQuery(const EntityDBQuery& other) = default;
    EntityDBQuery(EntityDBQuery&& other) noexcept = default;
    EntityDBQuery(const EntityArchetype& archetype);

    EntityDBQuery& operator=(const EntityDBQuery& other) = default;
    EntityDBQuery& operator=(EntityDBQuery&& other) noexcept = default;

    EntityDBQuery& with_component(ComponentType component_type);
    EntityDBQuery& without_component(ComponentType component_type);
    EntityDBQuery& with_optional_component(ComponentType component_type);

    EntityDBQuery& with_component(std::span<const ComponentType> component_types);
    EntityDBQuery& without_component(std::span<const ComponentType> component_types);
    EntityDBQuery& with_optional_component(std::span<const ComponentType> component_types);

    std::span<const ComponentType> required_components() const;
    std::span<const ComponentType> prohibited_components() const;
    std::span<const ComponentType> optional_components() const;

    EntityDBWindow query_db_window(EntityDatabaseContext& database_context);

    template <typename... Ts> requires ComponentList<Ts...>&& NoCVRefs<Ts...> EntityDBQuery& with_component();
    template <typename... Ts> requires ComponentList<Ts...>&& NoCVRefs<Ts...> EntityDBQuery& without_component();
    template <typename... Ts> requires ComponentList<Ts...>&& NoCVRefs<Ts...> EntityDBQuery& with_optional_component();

private:
    std::vector<ComponentType> m_required_components;
    std::vector<ComponentType> m_prohibited_components;
    std::vector<ComponentType> m_optional_components;
};

class EntityDBWindow {
public:
    EntityDBWindow() = default;
    EntityDBWindow(std::vector<Entity>&& entities, std::vector<std::vector<void*>>&& components,
        std::unordered_map<ComponentType, std::size_t>&& component_type_map);

    std::size_t size() const;
    std::size_t component_size() const;

    std::size_t entity_idx(Entity entity) const;
    std::size_t component_idx(ComponentType component_type) const;

    bool has_entity(Entity entity) const;
    bool has_component(ComponentType component_type) const;
    bool entity_has_component(Entity entity, ComponentType component_type) const;

    void* fetch_component_unchecked(std::size_t entity_idx, std::size_t component_idx);
    const void* fetch_component_unchecked(std::size_t entity_idx, std::size_t component_idx) const;

    std::span<const Entity> entity_span() const;
    std::span<void*> component_span(std::size_t component_idx);
    std::span<void* const> component_span(std::size_t component_idx) const;

    template <typename... Ts, typename Pred>
    requires ComponentList<Ts...>&& EntityDBWindowPred<Pred, Ts...> EntityDBWindow filter(Pred&& pred);

    template <typename... Ts, typename Fn>
    requires ComponentList<Ts...>&& EntityDBWindowIterateFn<Fn, Ts...> void iterate(Fn&& fn);

    template <typename... Ts, typename Fn>
    requires ComponentList<Ts...>&& EntityDBWindowForEachFn<Fn, Ts...> void for_each(Fn&& fn);

    template <typename... Ts, typename Fn, typename Pred>
    requires ComponentList<Ts...>&& EntityDBWindowIterateFn<Fn, Ts...>&& EntityDBWindowPred<Pred, Ts...> void iterate(
        Fn&& fn, Pred&& pred);

    template <typename... Ts, typename Fn, typename Pred>
    requires ComponentList<Ts...>&& EntityDBWindowForEachFn<Fn, Ts...>&& EntityDBWindowPred<Pred, Ts...> void for_each(
        Fn&& fn, Pred&& pred);

private:
    template <typename... Ts, typename Pred, std::size_t... Is>
    requires ComponentList<Ts...>&& EntityDBWindowPred<Pred, Ts...> EntityDBWindow filter(
        Pred&& pred, std::index_sequence<Is...>);

    template <typename... Ts, typename Fn, std::size_t... Is>
    requires ComponentList<Ts...>&& EntityDBWindowIterateFn<Fn, Ts...> void iterate(
        Fn&& fn, std::index_sequence<Is...>);

    template <typename... Ts, typename Fn, std::size_t... Is>
    requires ComponentList<Ts...>&& EntityDBWindowForEachFn<Fn, Ts...> void for_each(
        Fn&& fn, std::index_sequence<Is...>);

    template <typename... Ts, typename Fn, typename Pred, std::size_t... Is>
    requires ComponentList<Ts...>&& EntityDBWindowIterateFn<Fn, Ts...>&& EntityDBWindowPred<Pred, Ts...> void iterate(
        Fn&& fn, Pred&& pred, std::index_sequence<Is...>);

    template <typename... Ts, typename Fn, typename Pred, std::size_t... Is>
    requires ComponentList<Ts...>&& EntityDBWindowForEachFn<Fn, Ts...>&& EntityDBWindowPred<Pred, Ts...> void for_each(
        Fn&& fn, Pred&& pred, std::index_sequence<Is...>);

    std::vector<Entity> m_entities;
    std::vector<std::vector<void*>> m_components;
    std::unordered_map<ComponentType, std::size_t> m_component_type_map;
    std::unordered_map<Entity, std::size_t, EntityHasher> m_entity_index_map;
};

}

#include <visualizer/EntityDBQuery.impl>