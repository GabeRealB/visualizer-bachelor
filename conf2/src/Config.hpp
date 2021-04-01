#pragma once

#include <array>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <tuple>
#include <variant>
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
    float line_width;
    std::array<std::size_t, 4> fill_active;
    std::array<std::size_t, 4> fill_inactive;
    std::array<std::size_t, 4> border_active;
    std::array<std::size_t, 4> border_inactive;
    std::array<std::size_t, 4> oob_active;
    std::array<std::size_t, 4> oob_inactive;
    ViewCuboidCallable pos_size_callable;
};

struct HeatmapInfo {
    std::size_t idx;
    std::vector<float> colors_start;
    std::vector<std::array<std::size_t, 4>> colors;
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

    float border_width() const { return m_border_width; }

    std::array<float, 2> position() const { return m_position; }

    const std::string& id() const { return m_id; }

    const std::string& name() const { return m_name; }

    const std::optional<HeatmapInfo>& heatmap() const { return m_heatmap; }

    const std::array<std::size_t, 4>& border_color() const { return m_border_color; }

    const std::array<std::size_t, 4>& caption_color() const { return m_caption_color; }

    void set_size(float size) { m_size = size; }

    void set_border_width(float border_width) { m_border_width = border_width; }

    void set_position(float x, float y)
    {
        m_position[0] = x;
        m_position[1] = y;
    }

    void set_id(const std::string& id) { m_id = id; }

    void set_name(const std::string& name) { m_name = name; }

    void set_border_color(const std::array<std::size_t, 4>& color) { m_border_color = color; }

    void set_caption_color(const std::array<std::size_t, 4>& color) { m_caption_color = color; }

    void add_cuboid(const CuboidContainer& cuboid, const std::set<std::string>& requirements)
    {
        m_cuboids.push_back(cuboid);
        m_variable_requirements.push_back(requirements);
    }

    void add_heatmap(std::size_t idx) { m_heatmap = { idx, {}, {} }; }

    void add_heatmap_color(float start, std::array<std::size_t, 4> color)
    {
        if (!m_heatmap.has_value()) {
            std::cerr << "No heatmap is defined." << std::endl;
            std::abort();
        } else {
            auto& heatmap = m_heatmap.value();
            assert(heatmap.colors_start.size() == 0 || heatmap.colors_start.back() < start);
            heatmap.colors.push_back(color);
            heatmap.colors_start.push_back(start);
        }
    }

    std::size_t get_num_cuboids() const { return m_cuboids.size(); }

    std::span<const CuboidContainer> get_cuboids() const { return { m_cuboids.data(), m_cuboids.size() }; }

    std::vector<std::pair<CuboidContainer, std::size_t>> find_matching(const std::set<std::string>& requirements) const
    {
        auto matches = std::vector<std::pair<CuboidContainer, std::size_t>>{};
        for (std::size_t i = 0; i < m_variable_requirements.size(); i++) {
            if (m_variable_requirements[i] == requirements) {
                matches.emplace_back(m_cuboids[i], i);
            }
        }
        return matches;
    }

private:
    float m_size;
    float m_border_width;
    std::string m_id;
    std::string m_name;
    std::array<float, 2> m_position;
    std::optional<HeatmapInfo> m_heatmap;
    std::vector<CuboidContainer> m_cuboids;
    std::array<std::size_t, 4> m_border_color;
    std::array<std::size_t, 4> m_caption_color;
    std::vector<std::set<std::string>> m_variable_requirements;
};

enum class LegendEntityType { Color, Image };

class LegendEntity {
public:
    LegendEntity(const std::string& label, LegendEntityType type)
        : m_label{ label }
        , m_type{ type }
    {
    }

    LegendEntity(const LegendEntity& other) = default;
    LegendEntity(LegendEntity&& other) noexcept = default;
    virtual ~LegendEntity() = default;

    LegendEntity& operator=(const LegendEntity& other) = default;
    LegendEntity& operator=(LegendEntity&& other) noexcept = default;

    LegendEntityType type() const { return m_type; }

    const std::string& label() const { return m_label; }

private:
    std::string m_label;
    LegendEntityType m_type;
};

class ImageLegend : public LegendEntity {
public:
    ImageLegend(const std::string& label, const std::string& image_name, const std::filesystem::path& image_path,
        const std::array<float, 2>& scaling, bool absolute)
        : LegendEntity{ label, LegendEntityType::Color }
        , m_absolute{ absolute }
        , m_image_name{ image_name }
        , m_scaling{ scaling }
        , m_image_path{ image_path }
    {
    }

    ImageLegend(const ImageLegend& other) = default;
    ImageLegend(ImageLegend&& other) noexcept = default;
    ~ImageLegend() override = default;

    ImageLegend& operator=(const ImageLegend& other) = default;
    ImageLegend& operator=(ImageLegend&& other) noexcept = default;

    bool absolute() const { return m_absolute; }

    const std::string& image_name() const { return m_image_name; }

    const std::string& description() const { return label(); }

    const std::array<float, 2>& scaling() const { return m_scaling; }

    const std::filesystem::path& image_path() const { return m_image_path; }

private:
    bool m_absolute;
    std::string m_image_name;
    std::array<float, 2> m_scaling;
    std::filesystem::path m_image_path;
};

class ColorLegend : public LegendEntity {
public:
    ColorLegend(const std::string& label, const std::string& caption, const std::array<std::size_t, 4>& caption_color,
        const std::array<std::size_t, 4>& color)
        : LegendEntity{ label, LegendEntityType::Color }
        , m_caption{ caption }
        , m_color{ color }
        , m_caption_color{ caption_color }
    {
    }

    ColorLegend(const ColorLegend& other) = default;
    ColorLegend(ColorLegend&& other) noexcept = default;
    ~ColorLegend() override = default;

    ColorLegend& operator=(const ColorLegend& other) = default;
    ColorLegend& operator=(ColorLegend&& other) noexcept = default;

    const std::string& caption() const { return m_caption; }

    const std::array<std::size_t, 4>& color() const { return m_color; }

    const std::array<std::size_t, 4>& caption_color() const { return m_caption_color; }

private:
    std::string m_caption;
    std::array<std::size_t, 4> m_color;
    std::array<std::size_t, 4> m_caption_color;
};

class ImageResource {
public:
    ImageResource(float size, float border_width, const std::string& group, const std::string& name,
        const std::string& caption, const std::array<std::size_t, 4>& border_color,
        const std::array<std::size_t, 4>& caption_color, const std::string& id, const std::array<float, 2>& position,
        const std::filesystem::path& path)
        : m_size{ size }
        , m_border_width{ border_width }
        , m_name{ name }
        , m_group{ group }
        , m_caption{ caption }
        , m_id{ id }
        , m_path{ path }
        , m_position{ position }
        , m_border_color{ border_color }
        , m_caption_color{ caption_color }
    {
    }

    ImageResource(const ImageResource& other) = default;
    ImageResource(ImageResource&& other) noexcept = default;

    ImageResource& operator=(const ImageResource& other) = default;
    ImageResource& operator=(ImageResource&& other) noexcept = default;

    float size() const { return m_size; }

    float border_width() const { return m_border_width; }

    const std::string& name() const { return m_name; }

    const std::string& group() const { return m_group; }

    const std::string& caption() const { return m_caption; }

    const std::array<std::size_t, 4>& border_color() const { return m_border_color; }

    const std::array<std::size_t, 4>& caption_color() const { return m_caption_color; }

    const std::string& id() const { return m_id; }

    const std::filesystem::path& path() const { return m_path; }

    const std::array<float, 2>& position() const { return m_position; }

private:
    float m_size;
    float m_border_width;
    std::string m_name;
    std::string m_group;
    std::string m_caption;
    std::string m_id;
    std::filesystem::path m_path;
    std::array<float, 2> m_position;
    std::array<std::size_t, 4> m_border_color;
    std::array<std::size_t, 4> m_caption_color;
};

struct ConfigGroup {
    float line_width;
    std::string caption;
    std::string id;
    std::array<float, 2> position;
    std::array<std::size_t, 4> border_color;
    std::array<std::size_t, 4> caption_color;
};

enum class GroupConnectionPoint {
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
};

struct GroupConnection {
    float head_size;
    float line_width;
    std::string source;
    std::string destination;
    std::array<std::size_t, 4> color;
    GroupConnectionPoint source_point;
    GroupConnectionPoint destination_point;
};

class ConfigContainer {
public:
    using LegendVariant = std::variant<ColorLegend, ImageLegend>;

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

    const std::array<std::size_t, 4>& background_color() const { return m_background_color; }

    void set_background_color(const std::array<std::size_t, 4>& color) { m_background_color = color; }

    std::span<const LegendVariant> legend_entries() const { return { m_legend.begin(), m_legend.size() }; }

    void add_color_legend(const std::string& label, const std::string& caption,
        const std::array<std::size_t, 4>& caption_color, const std::array<std::size_t, 4>& color)
    {
        m_legend.push_back(ColorLegend{ label, caption, caption_color, color });
    }

    void add_image_legend(const std::string& label, const std::string& image_name,
        const std::filesystem::path& image_path, const std::array<float, 2>& scaling, bool absolute)
    {
        m_legend.push_back(ImageLegend{ label, image_name, image_path, scaling, absolute });
    }

    void add_image_resource(float size, float border_width, const std::string& group, const std::string& name,
        const std::string& caption, const std::array<std::size_t, 4>& border_color,
        const std::array<std::size_t, 4>& caption_color, const std::string& id, const std::array<float, 2>& position,
        const std::filesystem::path& path)
    {
        m_resources.push_back(ImageResource{
            size,
            border_width,
            group,
            name,
            caption,
            border_color,
            caption_color,
            id,
            position,
            path,
        });
    }

    std::span<const ImageResource> get_resources() const { return { m_resources.data(), m_resources.size() }; }

    const ViewContainer& get_view_container(const std::string& id) const
    {
        if (auto pos = std::find(m_view_ids.begin(), m_view_ids.end(), id); pos != m_view_ids.end()) {
            auto index = std::distance(m_view_ids.begin(), pos);
            return m_views[index];
        } else {
            std::cerr << "The view does not exist." << std::endl;
            std::abort();
        }
    }

    void add_view_container(const std::string& id, const ViewContainer& view_container)
    {
        if (std::find(m_view_ids.begin(), m_view_ids.end(), id) == m_view_ids.end()) {
            m_views.push_back(view_container);
            m_view_ids.push_back(id);
        } else {
            std::cerr << "The view already exists." << std::endl;
            std::abort();
        }
    }

    std::span<const std::string> get_view_ids() const { return { m_view_ids.data(), m_view_ids.size() }; }

    const std::string& get_group_association(const std::string& view_id) const
    {
        if (!m_group_associations.contains(view_id)) {
            std::cerr << "The group does not exist." << std::endl;
            std::abort();
        } else {
            return m_group_associations.at(view_id);
        }
    }

    const ConfigGroup& get_group(const std::string& group_name) const
    {
        if (!m_groups.contains(group_name)) {
            std::cerr << "The group does not exist." << std::endl;
            std::abort();
        } else {
            return m_groups.at(group_name);
        }
    }

    void add_group(const std::string& group_name, const std::string& caption, float border_width,
        const std::array<std::size_t, 4>& border_color, const std::array<std::size_t, 4>& caption_color,
        const std::string& group_id, const std::array<float, 2>& position)
    {
        if (m_groups.contains(group_name)) {
            std::cerr << "The view already exists." << std::endl;
            std::abort();
        } else {
            m_groups.insert({
                group_name,
                {
                    border_width,
                    caption,
                    group_id,
                    position,
                    border_color,
                    caption_color,
                },
            });
        }
    }

    void add_group_view(const std::string& group_name, const std::string& view_id)
    {
        if (auto pos = std::find(m_view_ids.begin(), m_view_ids.end(), view_id); pos != m_view_ids.end()) {
            if (m_group_associations.contains(view_id)) {
                std::cerr << "The view is already associated to a group." << std::endl;
                std::abort();
            } else {
                if (!m_groups.contains(group_name)) {
                    std::cerr << "The group does not exist." << std::endl;
                    std::abort();
                }
                m_group_associations.insert({ view_id, group_name });
            }
        } else {
            std::cerr << "The view does not exist." << std::endl;
            std::abort();
        }
    }

    std::span<const GroupConnection> get_group_connections() const
    {
        return { m_group_connections.data(), m_group_connections.size() };
    }

    void add_group_connection(const std::string& source, const std::string& source_point,
        const std::string& destination, const std::string& destination_point, const std::array<std::size_t, 4> color,
        float head_size, float line_width)
    {
        if (!m_groups.contains(source) || !m_groups.contains(destination)) {
            std::cerr << "The group does not exist." << std::endl;
            std::abort();
        } else {
            auto to_enum = [](const std::string& point) -> auto
            {
                if (point == "left") {
                    return GroupConnectionPoint::Left;
                } else if (point == "right") {
                    return GroupConnectionPoint::Right;
                } else if (point == "top") {
                    return GroupConnectionPoint::Top;
                } else if (point == "bottom") {
                    return GroupConnectionPoint::Bottom;
                } else if (point == "top-left") {
                    return GroupConnectionPoint::TopLeft;
                } else if (point == "top-right") {
                    return GroupConnectionPoint::TopRight;
                } else if (point == "bottom-left") {
                    return GroupConnectionPoint::BottomLeft;
                } else {
                    return GroupConnectionPoint::BottomRight;
                }
            };

            m_group_connections.push_back({
                head_size,
                line_width,
                source,
                destination,
                color,
                to_enum(source_point),
                to_enum(destination_point),
            });
        }
    }

    void add_variable(VariableType type, const std::string& name, std::size_t start, std::size_t end)
    {
        m_variables.emplace_back(type, name, start, end);
    }

    const std::string& get_config_template() const { return m_config_template; }

    void add_config_template(const std::string& config) { m_config_template = config; }

    VariableMap construct_variable_map() const
    {
        auto var_map = VariableMap{};
        for (const auto& var : m_variables) {
            var_map.initialize_variable(std::get<0>(var), std::get<1>(var), static_cast<int>(std::get<2>(var)),
                static_cast<int>(std::get<3>(var)));
        }
        return var_map;
    }

private:
    std::string m_config_template;
    std::vector<ViewContainer> m_views;
    std::vector<LegendVariant> m_legend;
    std::vector<std::string> m_view_ids;
    std::vector<ImageResource> m_resources;
    std::array<std::size_t, 4> m_background_color;
    std::map<std::string, std::string> m_group_associations;
    std::map<std::string, ConfigGroup> m_groups;
    std::vector<GroupConnection> m_group_connections;
    std::vector<std::tuple<VariableType, std::string, std::size_t, std::size_t>> m_variables;
};

}
