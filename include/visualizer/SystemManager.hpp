#pragma once

#include <concepts>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <visualizer/System.hpp>
#include <visualizer/TypeId.hpp>
#include <visualizer/UniqueTypes.hpp>

namespace Visualizer {

class SystemParameterMap {
public:
    SystemParameterMap() = default;

    void* retrieve(TypeId typeId) const;
    void insert(TypeId typeId, void* parameter);

    template <typename T> requires NoCVRefs<T>&& std::derived_from<T, System> void* retrieve() const;
    template <typename T> requires NoCVRefs<T>&& std::derived_from<T, System> void insert(void* parameter);

private:
    std::unordered_map<TypeId, void*> m_parameters;
};

class SystemManager : public GenericManager {
public:
    SystemManager() = default;
    SystemManager(const SystemManager& other) = delete;
    SystemManager(SystemManager&& other) noexcept = default;
    ~SystemManager();

    SystemManager& operator=(const SystemManager& other) = delete;
    SystemManager& operator=(SystemManager&& other) noexcept = default;

    bool hasSystem(std::string_view pass, TypeId typeId) const;
    std::shared_ptr<System> getSystem(std::string_view pass, TypeId typeId) const;
    void setSystem(std::string_view pass, TypeId typeId, std::shared_ptr<System> system);

    void run(std::string_view pass, const SystemParameterMap& parameters = {});

    template <typename T>
    requires NoCVRefs<T>&& std::derived_from<T, System> bool hasSystem(std::string_view pass) const;
    template <typename T>
    requires NoCVRefs<T>&& std::derived_from<T, System> std::shared_ptr<T> getSystem(std::string_view pass) const;
    template <typename T>
    requires NoCVRefs<T>&& std::derived_from<T, System> void setSystem(
        std::string_view pass, std::shared_ptr<T> system);
    template <typename T, typename... Args>
    requires NoCVRefs<T>&& std::derived_from<T, System>&& std::constructible_from<T, Args...> std::shared_ptr<T>
    addSystem(std::string_view pass, Args&&... args);

private:
    struct SystemPass {
        std::unordered_map<TypeId, std::size_t> m_systemMap;
        std::vector<std::tuple<std::shared_ptr<System>, TypeId>> m_systems;
    };

    struct StringCmp {
        using is_transparent = void;
        bool operator()(std::string_view a, std::string_view b) const { return a < b; }
    };

    std::vector<SystemPass> m_passes;
    std::map<std::string, std::size_t, StringCmp> m_passesMap;
};

}

#include <visualizer/SystemManager.impl>