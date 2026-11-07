add_library(deps_lz4 STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/lz4.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/lz4hc.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/lz4.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/lz4hc.c")
target_include_directories(deps_lz4 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::lz4 ALIAS deps_lz4)
