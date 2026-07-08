add_library(deps_emhash INTERFACE)
target_include_directories(deps_emhash INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::emhash ALIAS deps_emhash)
