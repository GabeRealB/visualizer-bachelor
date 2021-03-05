import json
import click
import uuid


@click.group()
def cli():
    pass


@click.command()
@click.argument('config', type=click.File('r'))
@click.argument('template', type=click.File('r'))
@click.argument('output', type=click.File('w'))
def generate(config, template, output):
    views = {}
    legend = {}
    groups = {}
    resources = {}
    variables = {"sequential": {}, "parallel": {}}

    mapping_template = """[=]() -> int {{
        return {};
    }}"""

    callable_template = """[]([[maybe_unused]] const VariableMap& variable_map) -> std::array<std::tuple<int, int>, 3> {{
        {}
        
        return {{
            std::tuple<int, int>{{ {}(), {}() }},
            std::tuple<int, int>{{ {}(), {}() }},
            std::tuple<int, int>{{ {}(), {}() }},
        }};
    }}"""

    cuboid_constr_template = """CuboidContainer {{
        .fill_color = {{ {} }},
        .out_of_bounds_color = {{ {} }},
        .unused_color = {{ {} }},
        .active_color = {{ {} }},
        .pos_size_callable = {},
    }}"""

    set_constr_template = """std::set<std::string> {{{}}}"""

    config_data = json.load(config)

    # Extract sequential variables
    for name, region in config_data["SEQ_CLOCK"].items():
        variables["sequential"][name] = region

    # Extract parallel variables
    for name, region in config_data["PAR_CLOCK"].items():
        variables["parallel"][name] = region

    # Extract legend
    legend = config_data["LEGEND"]
    groups = config_data["STRUCTURE"]
    resources = config_data["IMAGES"]

    # Extract views
    for name, view in config_data["CUBES"].items():
        cuboids = []
        view_zip = zip(view["mapping"], view["COLOR"])
        for mapping, color in view_zip:
            cuboids.append({"mapping": mapping, "color": color})
        views[name] = {"cuboids": cuboids, "arrangement": view["arrangement"]}
        if "heatmap" in view:
            views[name]["heatmap"] = view["heatmap"]

    # Generate cuboid mappings
    for view_name, view in views.items():
        for cuboid in view["cuboids"]:
            mappings = []
            mapping_info = cuboid["mapping"]

            for mapping in mapping_info[1:]:
                mappings.append(mapping_template.format(mapping[0]))
                mappings.append(mapping_template.format(mapping[1]))

            locals_str = ""
            for param in mapping_info[0]:
                locals_str = locals_str + """auto {0} = variable_map.get_variable_ref("{0}");""".format(param) + "\n"

            cuboid["callable"] = callable_template.format(locals_str, *mappings)

    # Generate the code
    code_str = "\n"
    for name, region in variables["sequential"].items():
        code_str = code_str + """\tconfig_instance.add_variable(VariableType::SEQUENTIAL, "{}", {}, {});\n""" \
            .format(name, *region) \
            .expandtabs(4)
    for name, region in variables["parallel"].items():
        code_str = code_str + """\tconfig_instance.add_variable(VariableType::PARALLEL, "{}", {}, {});\n""" \
            .format(name, *region) \
            .expandtabs(4)

    code_str = code_str + "\n"

    for view_name, view in views.items():
        container_name = "v_" + str(uuid.uuid4()).replace("-", "_")
        code_str = code_str + """\tauto {} = ViewContainer{{}};\n""" \
            .format(container_name) \
            .expandtabs(4)

        code_str = code_str + """\t{}.set_size({});\n""" \
            .format(container_name, view["arrangement"]["size"])

        code_str = code_str + """\t{}.set_position({}, {});\n""" \
            .format(container_name, view["arrangement"]["position"][0], view["arrangement"]["position"][1])

        for cuboid in view["cuboids"]:
            cuboid_name = "c_" + str(uuid.uuid4()).replace("-", "_")
            fill_color = ", ".join([str(c) for c in cuboid["color"][0]])
            out_of_bounds_color = ", ".join([str(c) for c in cuboid["color"][1]])
            unused_color = ", ".join([str(c) for c in cuboid["color"][2]])
            active_color = ", ".join([str(c) for c in cuboid["color"][3]])
            callable_func = cuboid["callable"]
            code_str = code_str + """\tauto {} = {};\n""" \
                .format(cuboid_name,
                        cuboid_constr_template.format(fill_color, out_of_bounds_color, unused_color, active_color,
                                                      callable_func)) \
                .expandtabs(4)

            requirements_name = "r_" + str(uuid.uuid4()).replace("-", "_")
            requirements = ",".join(map(lambda v: """ "{}" """.format(v), cuboid["mapping"][0]))
            code_str = code_str + """\tauto {} = {};\n""" \
                .format(requirements_name, set_constr_template.format(requirements)) \
                .expandtabs(4)

            code_str = code_str + """\t{}.add_cuboid({}, {});\n\n""" \
                .format(container_name, cuboid_name, requirements_name) \
                .expandtabs(4)

        if "heatmap" in view:
            heatmap = view["heatmap"]
            heatmap_cuboid = heatmap["cuboid"]
            heatmap_colors = heatmap["colors"]
            heatmap_colors_start = heatmap["colors_start"]

            code_str = code_str + """\t{}.add_heatmap({});\n""" \
                .format(container_name, heatmap_cuboid) \
                .expandtabs(4)

            for color, color_start in zip(heatmap_colors, heatmap_colors_start):
                color_str = ", ".join([str(c) for c in color])
                code_str = code_str + """\t{}.add_heatmap_color({}, {{ {} }});\n""" \
                    .format(container_name, color_start, color_str) \
                    .expandtabs(4)

        code_str = code_str + "\n"

        code_str = code_str + """\tconfig_instance.add_view_container("{}", {});\n\n""" \
            .format(view_name, container_name) \
            .expandtabs(4)

    # Generate legend
    for label, entry in legend.items():
        if entry["type"] == "color":
            entry_data = entry["data"]
            view_name = entry_data["color"][0]
            cuboid_idx = entry_data["color"][1]
            description = entry_data["description"]

            code_str = code_str + """\tconfig_instance.add_color_legend("{}", "{}", "{}", {});\n""" \
                .format(label, description, view_name, cuboid_idx) \
                .expandtabs(4)
        elif entry["type"] == "image":
            entry_data = entry["data"]
            name = entry_data["name"]
            path = entry_data["path"]
            scaling = entry_data["scaling"]
            description = entry_data["description"]
            absolute = "true" if entry_data["absolute"] else "false"

            code_str = code_str + """\tconfig_instance.add_image_legend("{}", "{}", "{}", {{ {}, {} }}, {});\n""" \
                .format(description, name, path, scaling[0], scaling[1], absolute) \
                .expandtabs(4)

    code_str = code_str + "\n"

    # Generate groups
    for group_name, group in groups["GROUPS"].items():
        for view_name in group:
            code_str = code_str + """\tconfig_instance.add_group("{}", "{}");\n""" \
                .format(group_name, view_name) \
                .expandtabs(4)
    for connection in groups["ARROWS"]:
        code_str = code_str + """\tconfig_instance.add_group_connection("{}", "{}");\n""" \
            .format(connection[0], connection[1]) \
            .expandtabs(4)

    code_str = code_str + "\n"

    # Generate resources
    for resource_name, resource, in resources.items():
        resource_size = resource["size"]
        resource_position = resource["position"]
        resource_path = resource["path"]

        code_str = code_str + """\tconfig_instance.add_image_resource({}, "{}", {{ {}, {} }}, "{}");\n""" \
            .format(resource_size, resource_name, resource_position[0], resource_position[1], resource_path) \
            .expandtabs(4)

    # Output the code
    generated_code = template.read().replace("@CONFIG_INIT_FUNC@", code_str)
    output.write(generated_code)


cli.add_command(generate)

if __name__ == '__main__':
    cli()
