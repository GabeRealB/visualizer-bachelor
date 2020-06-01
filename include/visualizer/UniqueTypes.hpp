#pragma once
#include <type_traits>

namespace Visualizer {

template <typename... Ts> struct types_are_unique {
    static constexpr bool value{ false };
};

template <typename T> struct types_are_unique<T> {
    static constexpr bool value{ true };
};

template <typename T1, typename T2> struct types_are_unique<T1, T2> {
    static constexpr bool value{ !std::is_same<T1, T2>::value };
};

template <typename T1, typename T2, typename... TRest> struct types_are_unique<T1, T2, TRest...> {
    static constexpr bool value{ types_are_unique<T1, T2>::value && types_are_unique<T1, TRest...>::value
        && types_are_unique<T2, TRest...>::value };
};

template <typename T> struct type_is_cvref {
    static constexpr bool value{ std::disjunction_v<std::is_reference<T>, std::is_const<T>, std::is_volatile<T>> };
};

template <typename... Ts> concept UniqueTypes = types_are_unique<Ts...>::value;

template <typename T, typename... U> concept SameType = std::conjunction_v<std::is_same<T, U>...>;

template <typename... Ts> concept NoCVRefs = std::negation_v<std::conjunction<type_is_cvref<Ts>...>>;

}
