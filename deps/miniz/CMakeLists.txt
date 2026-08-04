add_library(deps_miniz STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/miniz.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/miniz.c")
target_compile_definitions(deps_miniz PUBLIC MINIZ_NO_STDIO MINIZ_NO_ZLIB_COMPATIBLE_NAMES)
target_include_directories(deps_miniz PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::miniz ALIAS deps_miniz)
