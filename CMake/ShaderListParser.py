#!/usr/bin/env python3

import sys
import json
from pathlib import Path
import os
from ninja import ninja_syntax


def main():
    target_name = sys.argv[1]
    working_dir = Path(sys.argv[2])
    shader_format = sys.argv[3]
    output_file = Path(sys.argv[4])
    shader_compiler = Path(sys.argv[5])
    shaders_dir = Path(sys.argv[6])
    source_dir = Path(sys.argv[7])
    include_list = sys.argv[8]
    shader_list_files = sys.argv[9:]

    output_dir = output_file.parent

    with open(output_file, 'w') as f:
        writer = ninja_syntax.Writer(f, 150)

        writer.comment("####################################################################")
        writer.comment(f" {target_name} shaders compilation ninja file")
        writer.comment("####################################################################")
        writer.newline()

        writer.variable("python", f"\"{sys.executable}\"")

        include_variable = ''
        if include_list != "None":
            for directory in include_list.split(';'):
                include_variable += f'-I "{os.path.relpath(directory, working_dir)}" '
        writer.variable("includes", include_variable)

        writer.newline()

        base_command = f"{os.path.relpath(shader_compiler, working_dir)} "
        if shader_format == "spirv":
            base_command += "-spirv "
        base_command += "$in "
        base_command += "-T $shader_type "
        base_command += "-E $entry_point "
        base_command += "$includes "

        python_script = source_dir / "CMake/ShaderBuildCommand.py"
        command = f"$python {os.path.relpath(python_script, working_dir)} $out {base_command}"

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
                    output_shader = output_dir / shader_file.relative_to(shaders_dir)
                    output_shader = output_shader.with_name(f"{output_shader.stem}_{entry_point}{format_extension}")

                    writer.newline()
                    writer.build(
                        [os.path.relpath(output_shader, working_dir)],
                        "shader_compile",
                        [os.path.relpath(shader_file, working_dir)],
                        implicit=[
                            os.path.relpath(python_script, working_dir)
                        ],
                        variables={
                            "shader_type": shader_type,
                            "entry_point": entry_point
                        },
                    )

                    print(f" - Shader type [{shader_type}], Entry Point [{entry_point}]")
        writer.close()


if __name__ == "__main__":
    main()

