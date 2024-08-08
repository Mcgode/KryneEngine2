
# Fetch shader compiler executable
if (LINUX)
    set(ShaderCompiler "${CMAKE_SOURCE_DIR}/External/DirectXCompiler/bin/dxc")
elseif (WIN32)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
        set(ShaderCompiler "${CMAKE_SOURCE_DIR}/External/DirectXCompiler/bin/x86/dxc.exe")
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86")
        set(ShaderCompiler "${CMAKE_SOURCE_DIR}/External/DirectXCompiler/bin/x86/dxc.exe")
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")
        set(ShaderCompiler "${CMAKE_SOURCE_DIR}/External/DirectXCompiler/bin/arm64/dxc.exe")
    else ()
        message(FATAL_ERROR "Unsupported processor ${CMAKE_SYSTEM_PROCESSOR}")
    endif ()
else ()
    message(FATAL_ERROR "Platform unsupported")
endif ()

# Check that we found the executable
if (EXISTS "${ShaderCompiler}")
    message(STATUS "Will compile shaders using '${ShaderCompiler}'")
else ()
    message(FATAL_ERROR "Unable to find '${ShaderCompiler}'")
endif ()

set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Shaders")
set(GENERATE_SCRIPT "${CMAKE_SOURCE_DIR}/CMake/ShaderListParser.py")
find_package(Python3 REQUIRED)

function(target_compile_shaders TARGET_NAME LOCAL_SHADERS_DIR OUTPUT_DIR_NAME)
    set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SHADERS_DIR}")
    set(OUTPUT_DIR "${SHADER_OUTPUT_DIR}/${OUTPUT_DIR_NAME}")
    message(STATUS "Target directory name: '${LOCAL_SHADERS_DIR}'")
    message(STATUS "Absolute target directory name: '${SHADER_DIR}'")
    message(STATUS "Output directory name: '${OUTPUT_DIR}'")

    if (GraphicsApi STREQUAL "DX12")
        set(OutputFormat "")
    else ()
        set(OutputFormat "spirv")
    endif()

    if (NOT ${CMAKE_GENERATOR} MATCHES "Ninja")
        message(FATAL_ERROR "System currently exclusively supports Ninja")
    endif ()
    message(STATUS "${CMAKE_MAKE_PROGRAM}")

    file(GLOB ShaderListFiles "${SHADER_DIR}/*.shader")
    message(STATUS "Shader list files list: ${ShaderListFiles}")

    set(COMMANDS_FILE "${OUTPUT_DIR}/build.ninja")
    set(WORKING_DIR ${OUTPUT_DIR})

    add_custom_command(
            OUTPUT "${COMMANDS_FILE}"
            COMMAND ${CMAKE_COMMAND} -E echo "Generating build commands..."
            COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}"
            COMMAND ${Python3_EXECUTABLE} "${GENERATE_SCRIPT}"
                ${TARGET_NAME}
                ${WORKING_DIR}
                ${OutputFormat}
                ${COMMANDS_FILE}
                ${ShaderCompiler}
                ${SHADER_DIR}
                ${CMAKE_SOURCE_DIR}
                ${ShaderListFiles}
            DEPENDS ${GENERATE_SCRIPT} ${ShaderListFiles}
            COMMENT "Parsing shader list"
    )

    message(STATUS "${CMAKE_MAKE_PROGRAM}")
    add_custom_target(${TARGET_NAME}_ShaderCommands
            COMMAND ${CMAKE_MAKE_PROGRAM}
                -d explain
            WORKING_DIRECTORY ${WORKING_DIR}
            DEPENDS ${COMMANDS_FILE}
            COMMENT "Shader Compilation [${TARGET_NAME}]"
    )

    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_ShaderCommands)
endfunction()
