#include <visualizer/SystemManager.hpp>

namespace Visualizer {

void* SystemParameterMap::retrieve(TypeId typeId) const
{
    if (auto pos{ m_parameters.find(typeId) }; pos != m_parameters.end()) {
        return pos->second;
    } else {
        return nullptr;
    }
}

void SystemParameterMap::insert(TypeId typeId, void* parameter) { m_parameters.insert_or_assign(typeId, parameter); }

SystemManager::~SystemManager()
{
    for (auto& pass : m_passes) {
        for (auto& system : pass.m_systems) {
            if (std::get<0>(system)) {
                std::get<0>(system)->terminate();
            }
        }
    }
}

bool SystemManager::hasSystem(std::string_view pass, TypeId typeId) const
{
    if (auto pos{ m_passesMap.find(pass) }; pos != m_passesMap.end()) {
        return m_passes[pos->second].m_systemMap.contains(typeId);
    } else {
        return false;
    }
}

std::shared_ptr<System> SystemManager::getSystem(std::string_view pass, TypeId typeId) const
{
    if (auto pos{ m_passesMap.find(pass) }; pos != m_passesMap.end()) {
        auto& systemPass{ m_passes[pos->second] };
        if (auto systemPos{ systemPass.m_systemMap.find(typeId) }; systemPos != systemPass.m_systemMap.end()) {
            return std::get<0>(systemPass.m_systems[systemPos->second]);
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

void SystemManager::setSystem(std::string_view pass, TypeId typeId, std::shared_ptr<System> system)
{
    auto existingSystem{ getSystem(pass, typeId) };
    if (existingSystem) {
        existingSystem->terminate();
    }

    system->setWorld(*m_world);
    system->initialize();

    if (auto pos{ m_passesMap.find(pass) }; pos != m_passesMap.end()) {
        auto& systemPass{ m_passes[pos->second] };
        systemPass.m_systems.emplace_back(std::move(system), typeId);
        systemPass.m_systemMap.insert_or_assign(typeId, systemPass.m_systems.size() - 1);
    } else {
        auto systemPass{ SystemPass{} };
        systemPass.m_systems.emplace_back(std::move(system), typeId);
        systemPass.m_systemMap.insert({ typeId, 0 });
        m_passes.push_back(std::move(systemPass));
        m_passesMap.insert({ std::string{ pass.data(), pass.size() }, m_passes.size() - 1 });
    }
}

void SystemManager::run(std::string_view pass, const SystemParameterMap& parameters)
{
    if (auto pos{ m_passesMap.find(pass) }; pos != m_passesMap.end()) {
        auto& systemPass{ m_passes[pos->second] };

        for (auto& system : systemPass.m_systems) {
            std::get<0>(system)->run(parameters.retrieve(std::get<1>(system)));
        }
    }
}

}