add_library(deps_eigen INTERFACE)
target_include_directories(deps_eigen INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::eigen ALIAS deps_eigen)
