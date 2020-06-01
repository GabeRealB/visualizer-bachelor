#pragma once

#include <memory>
#include <type_traits>
#include <unordered_map>

#include <visualizer/TypeId.hpp>

namespace Visualizer {

class World;

class GenericManager {
public:
    GenericManager(const GenericManager& other) = delete;
    GenericManager(GenericManager&& other) = default;
    virtual ~GenericManager() = default;

    GenericManager& operator=(const GenericManager& other) = delete;
    GenericManager& operator=(GenericManager&& other) = default;

    bool hasWorld() const;
    void setWorld(const World& world);
    void freeWorld();

protected:
    GenericManager() = default;

    const World* m_world;
};

class World {
public:
    World() = default;
    World(const World& other) = delete;
    World(World&& other) noexcept;
    ~World();

    void operator=(const World& other) = delete;
    void operator=(World&& other) noexcept;

    bool hasManager(TypeId typeId) const;
    std::shared_ptr<GenericManager> getManager(TypeId typeId) const;
    void setManager(TypeId typeId, std::shared_ptr<GenericManager> manager);

    template <typename T> bool hasManager() const
    {
        static_assert(std::is_base_of<GenericManager, T>::value, "T must derive from GenericManager");
        auto id{ getTypeId<T>() };
        return hasManager(id);
    }

    template <typename T> std::shared_ptr<T> getManager() const
    {
        static_assert(std::is_base_of<GenericManager, T>::value, "T must derive from GenericManager");
        auto id{ getTypeId<T>() };
        return std::static_pointer_cast<T>(getManager(id));
    }

    template <typename T> void setManager(std::shared_ptr<T> manager)
    {
        static_assert(std::is_base_of<GenericManager, T>::value, "T must derive from GenericManager");
        auto id{ getTypeId<T>() };
        setManager(id, std::static_pointer_cast<GenericManager>(manager));
    }

    template <typename T, typename... Args> std::shared_ptr<T> addManager(Args&&... args)
    {
        static_assert(std::is_base_of<GenericManager, T>::value, "T must derive from GenericManager");
        static_assert(std::is_constructible<T, Args...>::value, "T is not constructible with the given parameters");
        auto id{ getTypeId<T>() };
        auto manager{ std::make_shared<T>(std::forward<Args>(args)...) };
        setManager(id, std::static_pointer_cast<GenericManager>(manager));
        return manager;
    }

private:
    std::unordered_map<TypeId, std::shared_ptr<GenericManager>> m_managerMap;
};

}
