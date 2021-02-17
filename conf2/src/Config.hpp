#pragma once

#include <array>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <span>
#include <string>
#include <tuple>
#include <vector>

namespace Config {

enum class VariableType { SEQUENTIAL, PARALLEL };

class Variables {
public:
    Variables() = default;
    Variables(const Variables& other) = default;
    Variables(Variables&& other) noexcept = default;
    ~Variables() = default;

    Variables& operator=(const Variables& other) = default;
    Variables& operator=(Variables&& other) noexcept = default;

    void reset()
    {
        for (std::size_t i = 0; i < m_values.size(); i++) {
            m_values[i] = m_regions[i].first;
        }
    }

    int& get_variable_ref(const std::string& name)
    {
        if (auto pos = std::find(m_names.begin(), m_names.end(), name); pos != m_names.end()) {
            auto index = std::distance(m_names.begin(), pos);
            return m_values[index];
        } else {
            std::cerr << "The variable does not exist." << std::endl;
            std::abort();
        }
    }

    const int& get_variable_ref(const std::string& name) const
    {
        if (auto pos = std::find(m_names.begin(), m_names.end(), name); pos != m_names.end()) {
            auto index = std::distance(m_names.begin(), pos);
            return m_values[index];
        } else {
            std::cerr << "The variable does not exist." << std::endl;
            std::abort();
        }
    }

    std::pair<int, int> get_variable_region(const std::string& name) const
    {
        if (auto pos = std::find(m_names.begin(), m_names.end(), name); pos != m_names.end()) {
            auto index = std::distance(m_names.begin(), pos);
            return m_regions[index];
        } else {
            std::cerr << "The variable does not exist." << std::endl;
            std::abort();
        }
    }

    void initialize_variable(const std::string& name, int start, int end)
    {
        if (std::find(m_names.begin(), m_names.end(), name) != m_names.end()) {
            std::cerr << "A variable with the same name already exists." << std::endl;
            std::abort();
        }

        m_names.push_back(name);
        m_values.emplace_back(start);
        m_regions.emplace_back(start, end);
    }

    std::size_t get_num_variables() const { return m_names.size(); }

    const std::string& get_variable_name(std::size_t index) const
    {
        if (index > m_names.size()) {
            std::cerr << "Out of bounds." << std::endl;
            std::abort();
        }

        return m_names[index];
    }

private:
    std::vector<int> m_values;
    std::vector<std::string> m_names;
    std::vector<std::pair<int, int>> m_regions;
};

class VariableMap {
public:
    VariableMap() = default;
    VariableMap(const VariableMap& other) = default;
    VariableMap(VariableMap&& other) noexcept = default;
    ~VariableMap() = default;

    VariableMap& operator=(const VariableMap& other) = default;
    VariableMap& operator=(VariableMap&& other) noexcept = default;

    void reset()
    {
        m_parallel.reset();
        m_sequential.reset();
    }

    int& get_variable_ref(const std::string& name)
    {
        if (!m_variable_types.contains(name)) {
            std::cerr << "The variable does not exist." << std::endl;
            std::abort();
        }

        auto type = m_variable_types[name];
        switch (type) {
        case VariableType::PARALLEL:
            return m_parallel.get_variable_ref(name);
        case VariableType::SEQUENTIAL:
            return m_sequential.get_variable_ref(name);
        default:
            std::cerr << "Wrong variable type provided." << std::endl;
            std::abort();
        }
    }

    const int& get_variable_ref(const std::string& name) const
    {
        if (!m_variable_types.contains(name)) {
            std::cerr << "The variable does not exist." << std::endl;
            std::abort();
        }

        auto type = m_variable_types.at(name);
        switch (type) {
        case VariableType::PARALLEL:
            return m_parallel.get_variable_ref(name);
        case VariableType::SEQUENTIAL:
            return m_sequential.get_variable_ref(name);
        default:
            std::cerr << "Wrong variable type provided." << std::endl;
            std::abort();
        }
    }

    std::pair<int, int> get_variable_region(const std::string& name) const
    {
        if (!m_variable_types.contains(name)) {
            std::cerr << "The variable does not exist." << std::endl;
            std::abort();
        }

        auto type = m_variable_types.at(name);
        switch (type) {
        case VariableType::PARALLEL:
            return m_parallel.get_variable_region(name);
        case VariableType::SEQUENTIAL:
            return m_sequential.get_variable_region(name);
        default:
            std::cerr << "Wrong variable type provided." << std::endl;
            std::abort();
        }
    }

    void initialize_variable(VariableType type, const std::string& name, int start, int end)
    {
        if (m_variable_types.contains(name)) {
            std::cerr << "A variable with the same name already exists." << std::endl;
            std::abort();
        }

        switch (type) {
        case VariableType::PARALLEL:
            m_parallel.initialize_variable(name, start, end);
            break;
        case VariableType::SEQUENTIAL:
            m_sequential.initialize_variable(name, start, end);
            break;
        default:
            std::cerr << "Wrong variable type provided." << std::endl;
            std::abort();
        }

        if (auto [pos, success] = m_variable_types.insert({ name, type }); !success) {
            (void)pos;
            std::cerr << "Unable to initialize the variable." << std::endl;
            std::abort();
        }
    }

    std::size_t get_num_variables(VariableType type) const
    {
        switch (type) {
        case VariableType::PARALLEL:
            return m_parallel.get_num_variables();
        case VariableType::SEQUENTIAL:
            return m_sequential.get_num_variables();
        default:
            std::cerr << "Wrong variable type provided." << std::endl;
            std::abort();
        }
    }

    const std::string& get_variable_name(VariableType type, std::size_t index) const
    {
        switch (type) {
        case VariableType::PARALLEL:
            return m_parallel.get_variable_name(index);
        case VariableType::SEQUENTIAL:
            return m_sequential.get_variable_name(index);
        default:
            std::cerr << "Wrong variable type provided." << std::endl;
            std::abort();
        }
    }

private:
    Variables m_parallel;
    Variables m_sequential;
    std::map<std::string, VariableType> m_variable_types;
};

using ViewCuboidCallable = std::array<std::tuple<int, int>, 3> (*)(const VariableMap& variable_map);

struct CuboidContainer {
    std::array<std::size_t, 4> fill_color;
    std::array<std::size_t, 4> unused_color;
    std::array<std::size_t, 4> active_color;
    ViewCuboidCallable pos_size_callable;
};

class ViewContainer {
public:
    ViewContainer() = default;
    ViewContainer(const ViewContainer& other) = default;
    ViewContainer(ViewContainer&& other) noexcept = default;
    ~ViewContainer() = default;

    ViewContainer& operator=(const ViewContainer& other) = default;
    ViewContainer& operator=(ViewContainer&& other) noexcept = default;

    float size() const { return m_size; }

    bool movable() const { return m_movable; }

    std::array<float, 2> position() const { return m_position; }

    void set_size(float size) { m_size = size; }

    void set_movable(bool movable) { m_movable = movable; }

    void set_position(float x, float y)
    {
        m_position[0] = x;
        m_position[1] = y;
    }

    void add_cuboid(const CuboidContainer& cuboid, const std::set<std::string>& requirements)
    {
        m_cuboids.push_back(cuboid);
        m_variable_requirements.push_back(requirements);
    }

    std::size_t get_num_cuboids() const { return m_cuboids.size(); }

    std::span<const CuboidContainer> get_cuboids() const { return { m_cuboids.data(), m_cuboids.size() }; }

    std::vector<CuboidContainer> find_matching(const std::set<std::string>& requirements) const
    {
        auto result = std::vector<CuboidContainer>{};
        for (std::size_t i = 0; i < m_variable_requirements.size(); i++) {
            if (m_variable_requirements[i] == requirements) {
                result.push_back(m_cuboids[i]);
            }
        }
        return result;
    }

private:
    float m_size;
    bool m_movable;
    std::array<float, 2> m_position;
    std::vector<CuboidContainer> m_cuboids;
    std::vector<std::set<std::string>> m_variable_requirements;
};

class ConfigContainer {
public:
    ConfigContainer() = default;
    ConfigContainer(const ConfigContainer& other) = default;
    ConfigContainer(ConfigContainer&& other) noexcept = default;
    ~ConfigContainer() = default;

    ConfigContainer& operator=(const ConfigContainer& other) = default;
    ConfigContainer& operator=(ConfigContainer&& other) noexcept = default;

    static ConfigContainer& get_instance()
    {
        static ConfigContainer container{};
        return container;
    }

    const ViewContainer& get_view_container(const std::string& name) const
    {
        if (auto pos = std::find(m_view_names.begin(), m_view_names.end(), name); pos != m_view_names.end()) {
            auto index = std::distance(m_view_names.begin(), pos);
            return m_views[index];
        } else {
            std::cerr << "The view does not exist." << std::endl;
            std::abort();
        }
    }

    void add_view_container(const std::string& name, const ViewContainer& view_container)
    {
        if (std::find(m_view_names.begin(), m_view_names.end(), name) == m_view_names.end()) {
            m_views.push_back(view_container);
            m_view_names.push_back(name);
        } else {
            std::cerr << "The view already exists." << std::endl;
            std::abort();
        }
    }

    std::span<const std::string> get_view_names() const { return { m_view_names.data(), m_view_names.size() }; }

    void add_variable(VariableType type, const std::string& name, std::size_t start, std::size_t end)
    {
        m_variables.emplace_back(type, name, start, end);
    }

    VariableMap construct_variable_map() const
    {
        auto var_map = VariableMap{};
        for (const auto& var : m_variables) {
            var_map.initialize_variable(std::get<0>(var), std::get<1>(var), std::get<2>(var), std::get<3>(var));
        }
        return var_map;
    }

private:
    std::vector<ViewContainer> m_views;
    std::vector<std::string> m_view_names;
    std::vector<std::tuple<VariableType, std::string, std::size_t, std::size_t>> m_variables;
};

}
