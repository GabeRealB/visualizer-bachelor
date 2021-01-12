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
    variables = {"sequential": {}, "parallel": {}}

    mapping_template = """[=]() -> int {{
        return {};
    }}"""

    callable_template = """[](const VariableMap& variable_map) -> std::array<std::tuple<std::size_t, std::size_t>> {{
        {}
        
        return {{
            {{ {}(), {}() }},
            {{ {}(), {}() }},
            {{ {}(), {}() }},
        }};
    }}"""

    cuboid_constr_template = """CuboidContainer {{
        .fill_color = {{ {} }},
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

    # Extract views
    for name, view in config_data["CUBES"].items():
        cuboids = []
        mapping_color_zip = zip(view["mapping"], view["COLOR"])
        for mapping, color in mapping_color_zip:
            cuboids.append({"mapping": mapping, "color": color})
        views[name] = cuboids

    # Generate cuboid mappings
    for view_name, view in views.items():
        for cuboid in view:
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
        code_str = code_str + "\tconfig_instance.add_variable(VariableType::SEQUENTIAL, {}, {}, {});\n" \
            .format(name, *region) \
            .expandtabs(4)
    for name, region in variables["parallel"].items():
        code_str = code_str + "\tconfig_instance.add_variable(VariableType::PARALLEL, {}, {}, {});\n" \
            .format(name, *region) \
            .expandtabs(4)

    code_str = code_str + "\n"

    for view_name, view in views.items():
        container_name = "v_" + str(uuid.uuid4()).replace("-", "_")
        code_str = code_str + """\tauto {} = ViewContainer{{}};\n""" \
            .format(container_name) \
            .expandtabs(4)

        for cuboid in view:
            cuboid_name = "c_" + str(uuid.uuid4()).replace("-", "_")
            fill_color = ", ".join([str(c) for c in cuboid["color"][0]])
            unused_color = ", ".join([str(c) for c in cuboid["color"][1]])
            active_color = ", ".join([str(c) for c in cuboid["color"][2]])
            callable_func = cuboid["callable"]
            code_str = code_str + """\tauto {} = {};\n""" \
                .format(cuboid_name,
                        cuboid_constr_template.format(fill_color, unused_color, active_color, callable_func)) \
                .expandtabs(4)

            requirements_name = "r_" + str(uuid.uuid4()).replace("-", "_")
            requirements = ",".join(map(lambda v:  """ "{}" """.format(v), cuboid["mapping"][0]))
            code_str = code_str + """\tauto {} = {};\n""" \
                .format(requirements_name, set_constr_template.format(requirements)) \
                .expandtabs(4)

            code_str = code_str + """\t{}.add_cuboid({}, {});\n\n""" \
                .format(container_name, cuboid_name, requirements_name) \
                .expandtabs(4)

        code_str = code_str + """\tconfig_instance.add_view_container("{}", {});\n\n""" \
            .format(view_name, container_name) \
            .expandtabs(4)

    # Output the code
    generated_code = template.read().replace("@CONFIG_INIT_FUNC@", code_str)
    output.write(generated_code)


cli.add_command(generate)

if __name__ == '__main__':
    cli()
