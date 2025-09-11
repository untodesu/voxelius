execute_process(COMMAND git rev-parse --short=8 HEAD
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file("${CMAKE_CURRENT_LIST_DIR}/version.cc.in" "${CMAKE_CURRENT_LIST_DIR}/version.cc")

add_library(core STATIC
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/version.cc"
    "${CMAKE_CURRENT_LIST_DIR}/version.hh")
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}")
target_precompile_headers(core PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_link_libraries(core PUBLIC enet emhash glm physfs spdlog stb)

add_subdirectory(config)
add_subdirectory(io)
add_subdirectory(math)
add_subdirectory(resource)
add_subdirectory(utils)

if(WIN32)
    target_compile_definitions(core PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(core PUBLIC _USE_MATH_DEFINES)
    target_compile_definitions(core PUBLIC NOMINMAX)
endif()
