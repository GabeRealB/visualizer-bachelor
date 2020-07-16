#pragma once

#include <array>
#include <cstdint>

#include <visualizer/UniqueTypes.hpp>

namespace Visualizer {

using TypeId = std::uintptr_t;

template <typename T> struct TypeIdPtr {
    static const T* const id;
};

template <typename T> const T* const TypeIdPtr<T>::id = nullptr;

template <typename T> requires NoCVRefs<T> TypeId getTypeId() noexcept
{
    return reinterpret_cast<TypeId>(&TypeIdPtr<T>::id);
}

template <typename... Ts> requires NoCVRefs<Ts...> std::array<TypeId, sizeof...(Ts)> getTypeIds() noexcept
{
    return { getTypeId<Ts>()... };
}

}