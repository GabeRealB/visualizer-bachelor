import copy
import click
import uuid
import commentjson


@click.group()
def cli():
    pass


@click.command()
@click.argument('config', type=click.File('r'))
@click.argument('template', type=click.File('r'))
@click.argument('output', type=click.File('w'))
def generate(config, template, output):
    variables = {"sequential": {}, "parallel": {}}
    cubes = {}
    appearance = {}
    grouping = {}
    legend = {}

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
        .line_width = {},
        .fill_active = {{ {} }},
        .fill_inactive = {{ {} }},
        .border_active = {{ {} }},
        .border_inactive = {{ {} }},
        .oob_active = {{ {} }},
        .oob_inactive = {{ {} }},
        .pos_size_callable = {},
    }}"""

    set_constr_template = """std::set<std::string> {{{}}}"""

    config_data = commentjson.load(config)
    config_template = copy.deepcopy(config_data)

    # Extract sequential variables
    for name, region in config_data["seq_clock"].items():
        variables["sequential"][name] = region

    # Extract parallel variables
    for name, region in config_data["par_clock"].items():
        variables["parallel"][name] = region

    # Extract cubes
    for name, cube in config_data["cubes"].items():
        cubes[name] = cube

    # Extract appearance
    appearance = config_data["appearance"]
    legend = appearance["legend"]
    grouping = appearance["grouping"]

    for name, group in grouping["groups"].items():
        group["internal_id"] = "gr_" + str(uuid.uuid4()).replace("-", "_")
        config_template["appearance"]["grouping"]["groups"][name]["position"] = "{}_position".format(
            group["internal_id"])

        for idx, element in enumerate(group["elements"]):
            element["internal_id"] = "el_" + str(uuid.uuid4()).replace("-", "_")
            template_element = config_template["appearance"]["grouping"]["groups"][name]["elements"][idx]
            template_element["scale"] = "{}_scale".format(element["internal_id"])
            template_element["position"] = "{}_position".format(element["internal_id"])

            if "cube" in element:
                call = []
                requirements = []
                line_widths = element["line width"]
                mapping_infos = cubes[element["cube"]]

                for mapping_info in mapping_infos:
                    mapping = []

                    for info in mapping_info[1:]:
                        mapping.append(mapping_template.format(info[0]))
                        mapping.append(mapping_template.format(info[1]))

                    locals_str = ""
                    for param in mapping_info[0]:
                        locals_str = locals_str + \
                                     """[[maybe_unused]] auto {0} = variable_map.get_variable_ref("{0}");""".format(
                                         param) + "\n"

                    call.append(callable_template.format(locals_str, *mapping))
                    requirements.append(mapping_info[0])

                element["infos"] = []
                for col_idx, (color, func, req, line_width, temp_col) in enumerate(
                        zip(element["colors"], call, requirements, line_widths, template_element["colors"])):
                    temp_col["fill_active"] = "{}_colors_fill_active_{}".format(element["internal_id"], col_idx)
                    temp_col["fill_inactive"] = "{}_colors_fill_inactive_{}".format(element["internal_id"], col_idx)
                    temp_col["border_active"] = "{}_colors_border_active_{}".format(element["internal_id"], col_idx)
                    temp_col["border_inactive"] = "{}_colors_border_inactive_{}".format(element["internal_id"], col_idx)
                    temp_col["oob_active"] = "{}_colors_oob_active_{}".format(element["internal_id"], col_idx)
                    temp_col["oob_inactive"] = "{}_colors_oob_inactive_{}".format(element["internal_id"], col_idx)

                    element["infos"].append(
                        {"line width": line_width, "color": color, "callable": func, "requirements": req})

                if "heatmap" in element:
                    template_element["heatmap"]["cuboid"] = "{}_heatmap_cuboid".format(element["internal_id"])
                    template_element["heatmap"]["colors"] = "{}_heatmap_colors".format(element["internal_id"])
                    template_element["heatmap"]["colors_start"] = "{}_heatmap_colors_start".format(
                        element["internal_id"])

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

    for name, group in grouping["groups"].items():
        group_position = ", ".join([str(c) for c in group["position"]])
        code_str = code_str + """\tconfig_instance.add_group("{}", "{}", "{}", {{ {} }});\n""" \
            .format(name, group["caption"], group["internal_id"], group_position) \
            .expandtabs(4)

        for element in group["elements"]:
            if "cube" in element:
                container_name = "v_" + str(uuid.uuid4()).replace("-", "_")
                code_str = code_str + """\tauto {} = ViewContainer{{}};\n""" \
                    .format(container_name) \
                    .expandtabs(4)

                code_str = code_str + """\t{}.set_size({}f);\n""" \
                    .format(container_name, element["scale"])

                code_str = code_str + """\t{}.set_position({}f, {}f);\n""" \
                    .format(container_name, element["position"][0], element["position"][1])

                code_str = code_str + """\t{}.set_id("{}");\n""" \
                    .format(container_name, element["internal_id"])

                for cuboid in element["infos"]:
                    cuboid_name = "c_" + str(uuid.uuid4()).replace("-", "_")
                    line_width = "{}f".format(str(cuboid["line width"]))
                    fill_active = ", ".join([str(c) for c in cuboid["color"]["fill_active"]])
                    fill_inactive = ", ".join([str(c) for c in cuboid["color"]["fill_inactive"]])
                    border_active = ", ".join([str(c) for c in cuboid["color"]["border_active"]])
                    border_inactive = ", ".join([str(c) for c in cuboid["color"]["border_inactive"]])
                    oob_active = ", ".join([str(c) for c in cuboid["color"]["oob_active"]])
                    oob_inactive = ", ".join([str(c) for c in cuboid["color"]["oob_inactive"]])
                    callable_func = cuboid["callable"]
                    code_str = code_str + """\tauto {} = {};\n""" \
                        .format(cuboid_name,
                                cuboid_constr_template.format(line_width, fill_active, fill_inactive, border_active,
                                                              border_inactive, oob_active,
                                                              oob_inactive, callable_func)) \
                        .expandtabs(4)

                    requirements_name = "r_" + str(uuid.uuid4()).replace("-", "_")
                    requirements = ",".join(map(lambda v: """ "{}" """.format(v), cuboid["requirements"]))
                    code_str = code_str + """\tauto {} = {};\n""" \
                        .format(requirements_name, set_constr_template.format(requirements)) \
                        .expandtabs(4)

                    code_str = code_str + """\t{}.add_cuboid({}, {});\n\n""" \
                        .format(container_name, cuboid_name, requirements_name) \
                        .expandtabs(4)

                if "heatmap" in element:
                    heatmap = element["heatmap"]
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
                    .format(element["caption"], container_name) \
                    .expandtabs(4)
                code_str = code_str + """\tconfig_instance.add_group_view("{}", "{}");\n""" \
                    .format(name, element["caption"]) \
                    .expandtabs(4)

            if "image" in element:
                image = element["image"]
                caption = element["caption"]
                internal_id = element["internal_id"]
                scale = element["scale"]
                position = element["position"]

                image_name = "img_" + str(uuid.uuid4()).replace("-", "_")
                position_str = ", ".join([str(c) for c in position])

                code_str = code_str + """\tconfig_instance.add_image_resource({}, "{}", "{}", "{}", "{}", 
                {{ {} }}, "{}");\n\n""" \
                    .format(scale, name, image_name, caption, internal_id, position_str, image) \
                    .expandtabs(4)

    code_str = code_str + "\n"

    for connection in grouping["arrows"]:
        code_str = code_str + """\tconfig_instance.add_group_connection("{}", "{}");\n""" \
            .format(connection[0], connection[1]) \
            .expandtabs(4)

    code_str = code_str + "\n"

    background_color_str = ", ".join([str(c) for c in appearance["background color"]])
    code_str = code_str + """\tconfig_instance.set_background_color({{ {} }});\n""" \
        .format(background_color_str) \
        .expandtabs(4)

    code_str = code_str + "\n"

    for name, entry in legend.items():
        color = entry["color"]
        caption = entry["caption"]
        color_str = ", ".join([str(c) for c in color])

        code_str = code_str + """\tconfig_instance.add_color_legend("{}", "{}",  {{ {} }});\n""" \
            .format(name, caption, color_str) \
            .expandtabs(4)

    code_str = code_str + """\tconfig_instance.add_config_template(R"config({})config");\n""" \
        .format(commentjson.dumps(config_template, indent=2)) \
        .expandtabs(4)

    # Output the code
    generated_code = template.read().replace("@CONFIG_INIT_FUNC@", code_str)
    output.write(generated_code)


cli.add_command(generate)

if __name__ == '__main__':
    cli()
