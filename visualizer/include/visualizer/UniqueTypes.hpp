#pragma once
#include <type_traits>

namespace Visualizer {

template <typename... Ts> struct are_unique {
    static constexpr bool value{ false };
};

template <typename T> struct are_unique<T> {
    static constexpr bool value{ true };
};

template <typename T1, typename T2> struct are_unique<T1, T2> {
    static constexpr bool value{ !std::is_same<T1, T2>::value };
};

template <typename T1, typename T2, typename... TRest> struct are_unique<T1, T2, TRest...> {
    static constexpr bool value{ are_unique<T1, T2>::value && are_unique<T1, TRest...>::value
        && are_unique<T2, TRest...>::value };
};

template <typename T> struct is_vref {
    static constexpr bool value{ std::disjunction_v<std::is_reference<T>, std::is_volatile<T>> };
};

template <typename T> struct is_cvref {
    static constexpr bool value{ std::disjunction_v<is_vref<T>, std::is_const<T>> };
};

template <typename... Ts> concept UniqueTypes = are_unique<Ts...>::value;

template <typename T, typename... U> concept SameType = std::conjunction_v<std::is_same<T, U>...>;

template <typename... Ts> concept NoVRefs = std::negation_v<std::conjunction<is_vref<Ts>...>>;

template <typename... Ts> concept NoCVRefs = std::negation_v<std::conjunction<is_cvref<Ts>...>>;

template <typename... Ts> concept ComponentList = UniqueTypes<Ts...>&& NoVRefs<Ts...>;

template <typename... Ts> concept ConstComponentList = ComponentList<Ts...>&& std::conjunction_v<std::is_const<Ts>...>;

}
