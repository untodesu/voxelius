function(embed_cxx_binary target_name input_path cxx_identifier)
    get_filename_component(input_name "${input_path}" NAME)
    set(cxxres_path "${PROJECT_BINARY_DIR}/${target_name}.generated.${input_name}.cc")

    string(MAKE_C_IDENTIFIER "${cxx_identifier}" cxx_identifier)

    add_custom_command(OUTPUT "${cxxres_path}"
        COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/scripts/generate-cxx-binarray.py" "${input_path}" "${cxxres_path}" "${cxx_identifier}"
        DEPENDS "${input_path}"
        COMMENT "Generating C++ source file for binary: ${input_name} [${cxx_identifier}]"
        VERBATIM)

    target_sources(${target_name} PRIVATE "${cxxres_path}")
    set_source_files_properties("${cxxres_path}" PROPERTIES GENERATED TRUE)
endfunction()
