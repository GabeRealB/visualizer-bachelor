#ifndef VISUALIZER_MDHOPS_HPP
#define VISUALIZER_MDHOPS_HPP

#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace MDH2Vis {

using Operation = std::uint32_t (*)(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3);

struct OperationContainer {
    std::vector<Operation> x;
    std::vector<Operation> y;
    std::vector<Operation> z;
};

enum class Component { X, Y, Z };

class OperationMap {
public:
    static const OperationContainer& getOperations(const std::string& name) { return getInstance().m_operations[name]; }

    static void addOperation(const std::string& name, Component component, Operation operation)
    {
        auto& instance{ getInstance() };
        OperationContainer* container{ nullptr };

        if (!instance.m_operations.contains(name)) {
            container = &instance.m_operations.insert_or_assign(name, OperationContainer{}).first->second;
        } else {
            container = &instance.m_operations.at(name);
        }

        switch (component) {
        case Component::X:
            container->x.emplace_back(operation);
            break;
        case Component::Y:
            container->y.emplace_back(operation);
            break;
        case Component::Z:
            container->z.emplace_back(operation);
            break;
        }
    }

private:
    OperationMap()
        : m_operations{}
    {
    }

    static OperationMap& getInstance()
    {
        static OperationMap operation_map{};
        return operation_map;
    }

    std::unordered_map<std::string, OperationContainer> m_operations;
};

class CombineOperations {
public:
    static std::span<const std::string> operations()
    {
        auto& ops{ getInstance() };
        return { ops.m_operations.data(), ops.m_operations.size() };
    }

    static void addOperations(const std::initializer_list<std::string>& list)
    {
        getInstance().m_operations.insert(getInstance().m_operations.begin(), list.begin(), list.end());
    }

private:
    CombineOperations()
        : m_operations{}
    {
    }

    static CombineOperations& getInstance()
    {
        static CombineOperations operation_vec{};
        return operation_vec;
    }

    std::vector<std::string> m_operations;
};

template <Component Comp, Operation Op> struct RegisterOperation {
    RegisterOperation(const char* Name) { OperationMap::addOperation(Name, Comp, Op); }
};

struct RegisterCombineOperations {
    RegisterCombineOperations(std::initializer_list<std::string> l) { CombineOperations::addOperations(l); }
};

}

#endif // VISUALIZER_MDHOPS_HPP
