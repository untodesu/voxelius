add_library(deps_parson STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/parson.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/parson.c")
target_include_directories(deps_parson PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::parson ALIAS deps_parson)
