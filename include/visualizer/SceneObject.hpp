#pragma once
#include <algorithm>
#include <glad/glad.h>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <visualizer/Mesh.hpp>

namespace Visualizer {

namespace detail {

    struct ComponentInfo {
        std::size_t index;
        std::size_t alignment;
        void (*destructorFunc)(const void*);
        void (*moveFunc)(void*, void*);
    };

    template <typename T> struct TypeIdPtr {
        static const T* const id;
    };

    template <typename T> const T* const TypeIdPtr<T>::id = nullptr;

    using TypeIdT = std::uintptr_t;

    template <typename T> TypeIdT typeId() noexcept
    {
        return reinterpret_cast<TypeIdT>(&TypeIdPtr<T>::id);
    }

}

/**
 * An object in a scene.
 */
class SceneObject {
public:
    template <typename T> bool hasComponent() const
    {
        auto id{ detail::typeId<T>() };
        auto pos{ m_componentInfos.find(id) };
        return !(pos == m_componentInfos.end());
    }

    template <typename T> std::optional<T> getComponent() const
    {
        auto ptr{ getComponentPtr<T>() };

        if (ptr == nullptr) {
            return std::nullopt;
        } else {
            return *ptr;
        }
    }

    template <typename T> void setComponent(const T& component)
    {
        auto ptr{ getComponentPtr<T>() };

        if (ptr == nullptr) {
            auto suitableIndex{ getSuitableIndex<T>() };

            if (!suitableIndex) {
                growComponentVector(sizeof(T), alignof(T));
                suitableIndex = getSuitableIndex<T>();
            }

            auto paddingSize{ *suitableIndex - m_components.size() };
            for (std::size_t i = 0; i < paddingSize + sizeof(T); ++i) {
                m_components.push_back({});
            }

            detail::ComponentInfo componentInfo{};
            componentInfo.index = m_components.size() - sizeof(T);
            componentInfo.alignment = alignof(T);
            componentInfo.moveFunc = [](void* src, void* dst) {
                auto tSrc{ static_cast<T*>(src) };
                auto tDst{ static_cast<T*>(dst) };

                if constexpr (std::is_trivially_copyable<T>::value) {
                    *tDst = *tSrc;
                } else {
                    *tDst = std::move(*tSrc);
                }
            };
            componentInfo.destructorFunc = [](const void* p) { static_cast<const T*>(p)->~T(); };

            auto id{ detail::typeId<T>() };
            m_componentInfos[id] = componentInfo;
            m_indexMap.push_back(id);

            ptr = reinterpret_cast<T*>(&m_components[*suitableIndex]);
        }

        *ptr = component;
    }

    template <typename T> void removeComponent()
    {
        auto ptr{ getComponentPtr<T>() };
        if (ptr != nullptr) {
            ptr->~T();

            auto id{ detail::typeId<T>() };
            m_componentInfos.erase(id);

            auto indexPos{ std::find(m_indexMap.begin(), m_indexMap.end(), id) };
            m_indexMap.erase(indexPos);
        }
    }

    template <typename T> T* getComponentPtr()
    {
        auto id{ detail::typeId<T>() };
        auto pos{ m_componentInfos.find(id) };
        if (pos == m_componentInfos.end()) {
            return nullptr;
        } else {
            detail::ComponentInfo info{ pos->second };
            return reinterpret_cast<T*>(&m_components[info.index]);
        }
    }

    template <typename T> const T* getComponentPtr() const
    {
        auto id{ detail::typeId<T>() };
        auto pos{ m_componentInfos.find(id) };
        if (pos == m_componentInfos.end()) {
            return nullptr;
        } else {
            detail::ComponentInfo info{ pos->second };
            return reinterpret_cast<const T*>(&m_components[info.index]);
        }
    }

private:
    void growComponentVector(std::size_t neededSize, std::size_t neededAlignment);

    template <typename T> std::optional<std::size_t> getSuitableIndex()
    {
        auto dataPtr{ m_components.data() };
        auto currentSize{ m_components.size() };
        auto remainingSize{ m_components.capacity() - currentSize };
        auto currentEnd{ &dataPtr[currentSize] };
        auto alignedEnd{ static_cast<void*>(currentEnd) };

        if (std::align(alignof(T), sizeof(T), alignedEnd, remainingSize) != nullptr) {
            auto difference{ static_cast<std::byte*>(alignedEnd) - currentEnd };
            return currentSize + difference;
        } else {
            return {};
        }
    }

    std::vector<std::byte> m_components;
    std::vector<detail::TypeIdT> m_indexMap;
    std::unordered_map<detail::TypeIdT, detail::ComponentInfo> m_componentInfos;
};

}
