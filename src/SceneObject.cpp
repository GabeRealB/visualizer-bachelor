#include <visualizer/SceneObject.hpp>

namespace Visualizer {

void SceneObject::growComponentVector(std::size_t neededSize, std::size_t neededAlignment)
{
    std::vector<std::byte> newVector{};
    if ((m_components.capacity() * 2) - m_components.size() < neededSize + neededAlignment) {
        newVector.reserve((m_components.capacity() * 2) + neededSize + neededAlignment);
    } else {
        newVector.reserve(m_components.capacity() * 2);
    }

    if (m_indexMap.size() > 0) {
        auto componentInfo{ m_componentInfos[m_indexMap[0]] };
        auto alignmentOffset{ reinterpret_cast<std::uintptr_t>(newVector.data())
            % componentInfo.alignment };

        auto indexOffset{ static_cast<std::intmax_t>(componentInfo.index)
            - static_cast<std::intmax_t>(alignmentOffset) };

        for (std::size_t i = 0; i < m_components.size() + indexOffset; ++i) {
            newVector.push_back({});
        }

        for (auto& keys : m_componentInfos) {
            detail::ComponentInfo info{ keys.second };
            auto srcPtr{ &m_components[info.index] };
            auto destPtr{ &newVector[info.index + indexOffset] };
            info.index += indexOffset;
            info.moveFunc(srcPtr, destPtr);
            info.destructorFunc(srcPtr);
        }
    }

    m_components = std::move(newVector);
}

}