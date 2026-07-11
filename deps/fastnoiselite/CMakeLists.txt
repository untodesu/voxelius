add_library(deps_FNL STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/fastnoiselite.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/fastnoiselite.c")
target_compile_definitions(deps_FNL PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(deps_FNL PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(deps_FNL PROPERTIES FOLDER Dependencies)

add_library(deps::FNL ALIAS deps_FNL)
