add_library(shared STATIC
    "${CMAKE_CURRENT_LIST_DIR}/chunk_aabb.cc"
    "${CMAKE_CURRENT_LIST_DIR}/chunk_aabb.hh"
    "${CMAKE_CURRENT_LIST_DIR}/chunk.cc"
    "${CMAKE_CURRENT_LIST_DIR}/chunk.hh"
    "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt"
    "${CMAKE_CURRENT_LIST_DIR}/collision.cc"
    "${CMAKE_CURRENT_LIST_DIR}/collision.hh"
    "${CMAKE_CURRENT_LIST_DIR}/const.hh"
    "${CMAKE_CURRENT_LIST_DIR}/coord.hh"
    "${CMAKE_CURRENT_LIST_DIR}/dimension.cc"
    "${CMAKE_CURRENT_LIST_DIR}/dimension.hh"
    "${CMAKE_CURRENT_LIST_DIR}/factory.cc"
    "${CMAKE_CURRENT_LIST_DIR}/factory.hh"
    "${CMAKE_CURRENT_LIST_DIR}/feature.cc"
    "${CMAKE_CURRENT_LIST_DIR}/feature.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game_items.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game_items.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game_voxels.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game_voxels.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gravity.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gravity.hh"
    "${CMAKE_CURRENT_LIST_DIR}/grounded.hh"
    "${CMAKE_CURRENT_LIST_DIR}/head.hh"
    "${CMAKE_CURRENT_LIST_DIR}/item_registry.cc"
    "${CMAKE_CURRENT_LIST_DIR}/item_registry.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/player.hh"
    "${CMAKE_CURRENT_LIST_DIR}/protocol.cc"
    "${CMAKE_CURRENT_LIST_DIR}/protocol.hh"
    "${CMAKE_CURRENT_LIST_DIR}/ray_dda.cc"
    "${CMAKE_CURRENT_LIST_DIR}/ray_dda.hh"
    "${CMAKE_CURRENT_LIST_DIR}/splash.cc"
    "${CMAKE_CURRENT_LIST_DIR}/splash.hh"
    "${CMAKE_CURRENT_LIST_DIR}/stasis.cc"
    "${CMAKE_CURRENT_LIST_DIR}/stasis.hh"
    "${CMAKE_CURRENT_LIST_DIR}/threading.cc"
    "${CMAKE_CURRENT_LIST_DIR}/threading.hh"
    "${CMAKE_CURRENT_LIST_DIR}/transform.cc"
    "${CMAKE_CURRENT_LIST_DIR}/transform.hh"
    "${CMAKE_CURRENT_LIST_DIR}/types.hh"
    "${CMAKE_CURRENT_LIST_DIR}/velocity.cc"
    "${CMAKE_CURRENT_LIST_DIR}/velocity.hh"
    "${CMAKE_CURRENT_LIST_DIR}/voxel_registry.cc"
    "${CMAKE_CURRENT_LIST_DIR}/voxel_registry.hh"
    "${CMAKE_CURRENT_LIST_DIR}/voxel_storage.cc"
    "${CMAKE_CURRENT_LIST_DIR}/voxel_storage.hh")
target_compile_features(shared PUBLIC cxx_std_20)
target_include_directories(shared PUBLIC "${DEPS_INCLUDE_DIR}")
target_include_directories(shared PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_include_directories(shared PRIVATE "${PROJECT_SOURCE_DIR}/src/game")
target_precompile_headers(shared PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_link_libraries(shared PUBLIC core enet entt FNL miniz parson thread_pool)
