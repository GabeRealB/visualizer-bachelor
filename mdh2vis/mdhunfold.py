import json
import sys
import os
import uuid

mdh_path_file_path = sys.argv[1]
mdh_in_path = sys.argv[2]
mdh_out_path = sys.argv[3]

combine_ops = []
views_in = {}
views_out = {}

mdh_path_found = False

if os.path.exists(mdh_path_file_path):
    with open(mdh_path_file_path) as mdh_path_file:
        mdh_path = mdh_path_file.read()
        mdh_path_found = True

if os.path.exists(mdh_path) and mdh_path_found:
    with open(mdh_path) as mdh_file:
        data = json.load(mdh_file)
    for op in data["MDH"]["combine operators"]:
        combine_ops.append(op)
    for name, input_view in data["views"]["input"].items():
        views_in[name] = { "x": [], "y": [], "z": [] }
        for op in input_view:
            if op[0] not in views_in[name]["x"]:
                views_in[name]["x"].append(op[0])
            if op[1] not in views_in[name]["y"]:
                views_in[name]["y"].append(op[1])
            if op[2] not in views_in[name]["z"]:
                views_in[name]["z"].append(op[2])
    for name, output_view in data["views"]["output"].items():
        views_out[name] = { "x": [], "y": [], "z": [] }
        for op in output_view:
            if op[0] not in views_out[name]["x"]:
                views_out[name]["x"].append(op[0])
            if op[1] not in views_out[name]["y"]:
                views_out[name]["y"].append(op[1])
            if op[2] not in views_out[name]["z"]:
                views_out[name]["z"].append(op[2])

    function_template = """std::uint32_t {}(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3) {{
        (void)i1;
        (void)i2;
        (void)i3;
        return {};
    }}"""

    register_operation_template = "RegisterOperation<{}, {}> {} {{ \"{}\" }};"

    mdh_output = "RegisterCombineOperations sCombineOperations { "
    for op in combine_ops:
        mdh_output += "\"{}\", ".format(op)
    mdh_output = mdh_output[:-2] + " }; \n"

    for name, view in views_in.items():
        for op in view["x"]:
            random_name = str(uuid.uuid4()).replace("-", "_")
            op_name = "f_" + random_name
            reg_name = "s_" + random_name
            mdh_output += function_template.format(op_name, op) + "\n"
            mdh_output += register_operation_template.format("Component::X", op_name, reg_name, name) + "\n"
        for op in view["y"]:
            random_name = str(uuid.uuid4()).replace("-", "_")
            op_name = "f_" + random_name
            reg_name = "s_" + random_name
            mdh_output += function_template.format(op_name, op) + "\n"
            mdh_output += register_operation_template.format("Component::Y", op_name, reg_name, name) + "\n"
        for op in view["z"]:
            random_name = str(uuid.uuid4()).replace("-", "_")
            op_name = "f_" + random_name
            reg_name = "s_" + random_name
            mdh_output += function_template.format(op_name, op) + "\n"
            mdh_output += register_operation_template.format("Component::Z", op_name, reg_name, name) + "\n"

    for name, view in views_out.items():
        for op in view["x"]:
            random_name = str(uuid.uuid4()).replace("-", "_")
            op_name = "f_" + random_name
            reg_name = "s_" + random_name
            mdh_output += function_template.format(op_name, op) + "\n"
            mdh_output += register_operation_template.format("Component::X", op_name, reg_name, name) + "\n"
        for op in view["y"]:
            random_name = str(uuid.uuid4()).replace("-", "_")
            op_name = "f_" + random_name
            reg_name = "s_" + random_name
            mdh_output += function_template.format(op_name, op) + "\n"
            mdh_output += register_operation_template.format("Component::Y", op_name, reg_name, name) + "\n"
        for op in view["z"]:
            random_name = str(uuid.uuid4()).replace("-", "_")
            op_name = "f_" + random_name
            reg_name = "s_" + random_name
            mdh_output += function_template.format(op_name, op) + "\n"
            mdh_output += register_operation_template.format("Component::Z", op_name, reg_name, name) + "\n"
else:
    mdh_output = ""

with open(mdh_in_path) as mdh_in_file, open(mdh_out_path, "w") as mdh_out_file:
    contents = mdh_in_file.read().replace("/*<Code>*/", mdh_output)
    mdh_out_file.write(contents)
