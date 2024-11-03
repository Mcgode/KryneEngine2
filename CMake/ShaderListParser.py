#!/usr/bin/env python3

import sys
import json
from pathlib import Path, PurePath
import os
from ninja import ninja_syntax


def main():
    target_name = sys.argv[1]
    shader_output_dir = Path(sys.argv[2])
    shader_format = sys.argv[3]
    output_file = Path(sys.argv[4])
    shader_compiler = Path(sys.argv[5])
    shaders_dir = Path(sys.argv[6])
    source_dir = Path(sys.argv[7])
    include_list = sys.argv[8]
    converter = sys.argv[9]
    convert_format = sys.argv[10]
    shader_list_files = sys.argv[11:]

    working_dir = output_file.parent

    python_script = source_dir / "CMake/ShaderBuildCommand.py"

    with open(output_file, 'w') as f:
        writer = ninja_syntax.Writer(f, 150)

        writer.comment("####################################################################")
        writer.comment(f" {target_name} shaders compilation ninja file")
        writer.comment("####################################################################")
        writer.newline()

        writer.comment("--------------------------------------------------------------------")
        writer.comment(f" Variables")
        writer.comment("--------------------------------------------------------------------")
        writer.newline()

        python_name = "python"
        writer.variable(python_name, f"\"{sys.executable}\"")

        include_variable = ''
        if include_list != "None":
            for directory in include_list.split(';'):
                include_variable += f'-I "{os.path.relpath(directory, working_dir)}" '
        writer.variable("includes", include_variable)

        build_shader_script_name = "build_shader_script"
        writer.variable(build_shader_script_name, os.path.relpath(python_script, working_dir))

        shader_compiler_name = "shader_compiler"
        writer.variable(shader_compiler_name, os.path.relpath(shader_compiler, working_dir))

        shader_input_dir_name = "input_dir"
        writer.variable(shader_input_dir_name, os.path.relpath(shaders_dir, working_dir))

        shader_output_dir_name = "output_dir"
        writer.variable(shader_output_dir_name, os.path.relpath(shader_output_dir, working_dir))

        converter_name = "converter"
        if converter != "none":
            writer.variable(converter_name, converter)

        writer.newline()

        base_command = f"${shader_compiler_name} "
        if shader_format == "spirv":
            base_command += "-spirv "
        base_command += "$in "
        base_command += "-T $shader_type "
        base_command += "-E $entry_point "
        base_command += "$includes "

        command = f"${python_name} ${build_shader_script_name} $out {base_command}"

        format_extension = ".cso"
        if shader_format == "spirv":
            format_extension = ".spv"

        writer.comment("--------------------------------------------------------------------")
        writer.comment(f" Build rules")
        writer.comment("--------------------------------------------------------------------")
        writer.newline()

        writer.rule("shader_compile",
                    command,
                    depfile="$out.d",
                    deps="gcc")

        if converter != "none":
            writer.newline()
            writer.rule("shader_convert", f"${converter_name} $in -o $out")

        writer.newline()
        writer.comment("--------------------------------------------------------------------")
        writer.comment(f" Build commands")
        writer.comment("--------------------------------------------------------------------")

        for input_file_path in shader_list_files:
            with open(input_file_path, 'r') as input_file:
                current_dir = Path(input_file_path).parent
                input_json = json.load(input_file)

                if "Path" not in input_json or "Configurations" not in input_json:
                    continue

                shader_file = current_dir / input_json["Path"]
                configuration_count = len(input_json["Configurations"])
                print(f"Creating commands for {shader_file.relative_to(shaders_dir)}, "
                      f"with {configuration_count} different configurations")

                for configuration in input_json["Configurations"]:
                    entry_point = configuration["EntryPoint"]
                    shader_type = configuration["ShaderType"]

                    # Output location
                    output_shader = shader_file.relative_to(shaders_dir)
                    output_shader = output_shader.with_name(f"{output_shader.stem}_{entry_point}{format_extension}")
                    output_shader = PurePath(f"${shader_output_dir_name}") / output_shader

                    input_path = PurePath(f'${shader_input_dir_name}') / shader_file.relative_to(shaders_dir)
                    print(input_path)

                    writer.newline()
                    writer.build(
                        [str(output_shader)],
                        "shader_compile",
                        [str(input_path)],
                        implicit=[
                            f"${build_shader_script_name}"
                        ],
                        variables={
                            "shader_type": shader_type,
                            "entry_point": entry_point
                        },
                    )

                    if converter != "none":
                        final_shader = shader_file.relative_to(shaders_dir)
                        final_shader = final_shader.with_name(f"{final_shader.stem}_{entry_point}.{convert_format}")
                        final_shader = PurePath(f"${shader_output_dir_name}") / final_shader
                        writer.build(
                            [str(final_shader)],
                            "shader_convert",
                            str(output_shader))

                    print(f" - Shader type [{shader_type}], Entry Point [{entry_point}]")
        writer.close()


if __name__ == "__main__":
    main()

