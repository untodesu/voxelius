include("${CMAKE_CURRENT_LIST_DIR}/embed-cxx.cmake")

find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)

function(compile_spirv_shader target_name shader_source)
    get_filename_component(shader_source_name "${shader_source}" NAME)
    set(shader_binary_path "${PROJECT_BINARY_DIR}/${target_name}.generated.${shader_source_name}.spirv")

    # Create a valid C++ identifier for the shader binary
    set(shader_cxx_identifier "spirv_${shader_source_name}")
    string(REGEX REPLACE "\\.glsl$" "" shader_cxx_identifier "${shader_cxx_identifier}")
    string(REGEX REPLACE "\\.hlsl$" "" shader_cxx_identifier "${shader_cxx_identifier}")
    string(MAKE_C_IDENTIFIER "${shader_cxx_identifier}" shader_cxx_identifier)

    add_custom_command(OUTPUT "${shader_binary_path}"
        COMMAND ${GLSLANG_VALIDATOR} -e main -V -o "${shader_binary_path}" "${shader_source}"
        DEPENDS ${shader_source}
        COMMENT "Compiling SPIR-V shader: ${shader_source_name}"
        VERBATIM)

    embed_cxx_binary(${target_name} "${shader_binary_path}" "${shader_cxx_identifier}")
endfunction()
