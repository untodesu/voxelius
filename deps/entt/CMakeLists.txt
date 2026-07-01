add_library(deps_entt INTERFACE)
target_include_directories(deps_entt INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::entt ALIAS deps_entt)
