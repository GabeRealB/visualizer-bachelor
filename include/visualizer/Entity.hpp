#pragma once

#include <cstddef>

namespace Visualizer {

struct Entity {
    std::size_t id;
    std::size_t generation;
};

bool operator==(const Entity& lhs, const Entity& rhs) noexcept;

struct EntityHasher {
    std::size_t operator()(const Entity& k) const;
};

}
