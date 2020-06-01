#include <visualizer/World.hpp>

#include <utility>

namespace Visualizer {

bool GenericManager::hasWorld() const { return m_world != nullptr; }
void GenericManager::setWorld(const World& world) { m_world = &world; }
void GenericManager::freeWorld() { m_world = nullptr; }

World::~World()
{
    for (auto& val : m_managerMap) {
        val.second->freeWorld();
    }
}

World::World(World&& other) noexcept
    : m_managerMap{ std::exchange(other.m_managerMap, {}) }
{
    for (auto& val : m_managerMap) {
        val.second->setWorld(*this);
    }
}

void World::operator=(World&& other) noexcept
{
    for (auto& val : m_managerMap) {
        val.second->freeWorld();
    }

    m_managerMap = std::exchange(other.m_managerMap, {});
    for (auto& val : m_managerMap) {
        val.second->setWorld(*this);
    }
}

bool World::hasManager(TypeId typeId) const { return m_managerMap.contains(typeId); }

std::shared_ptr<GenericManager> World::getManager(TypeId typeId) const
{
    auto pos{ m_managerMap.find(typeId) };
    if (pos == m_managerMap.end()) {
        return nullptr;
    } else {
        return pos->second;
    }
}

void World::setManager(TypeId typeId, std::shared_ptr<GenericManager> manager)
{
    manager->setWorld(*this);
    m_managerMap.insert_or_assign(typeId, std::move(manager));
}

}
