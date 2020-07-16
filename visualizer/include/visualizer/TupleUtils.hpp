#pragma once

#include <cstddef>
#include <utility>

namespace Visualizer {

template <typename T> struct TNullptr {
    static constexpr T* value{ nullptr };
};

template <std::size_t, typename T> struct TypeWrapper {
    using type = T;
};

template <typename T, std::size_t N> struct TupleTypeN {
    template <std::size_t... I> static auto getType(std::index_sequence<I...>)
    {
        return std::tuple<typename TypeWrapper<I, T>::type...>{};
    }

    using type = decltype(getType(std::make_index_sequence<N>{}));
};

}