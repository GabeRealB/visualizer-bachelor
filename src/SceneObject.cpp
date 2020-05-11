#include <visualizer/SceneObject.hpp>

namespace Visualizer {

void SceneObject::growComponentVector(std::size_t neededSize)
{
    std::vector<std::byte> newVector{};
    if ((m_components.capacity() * 2) - m_components.size() < neededSize * 2) {
        newVector.reserve((m_components.capacity() + neededSize) * 2);
    } else {
        newVector.reserve(m_components.capacity() * 2);
    }
    for (std::size_t i = 0; i < m_components.size(); ++i) {
        newVector.push_back({});
    }

    for (auto& keys : m_componentInfos) {
        detail::ComponentInfo info{ keys.second };
        auto srcPtr{ &m_components[info.index] };
        auto destPtr{ &newVector[info.index] };
        info.moveFunc(srcPtr, destPtr);
        info.destructorFunc(srcPtr);
    }

    m_components = std::move(newVector);
}

}