#include <visualizer/Entity.hpp>

#include <functional>

namespace Visualizer {

bool operator==(const Entity& lhs, const Entity& rhs) noexcept
{
    return (lhs.generation == rhs.generation && lhs.id == rhs.id);
}

/**************************************************************************************************
 ****************************************** EntityHasher ******************************************
 **************************************************************************************************/

std::size_t EntityHasher::operator()(const Entity& k) const
{
    return std::hash<std::size_t>{}(k.id) ^ std::hash<std::size_t>{}(k.generation);
}
}