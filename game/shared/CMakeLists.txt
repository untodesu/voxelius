add_library(shared STATIC
    "${CMAKE_CURRENT_LIST_DIR}/const.hh"
    "${CMAKE_CURRENT_LIST_DIR}/coord.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game_items.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game_items.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game_voxels.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game_voxels.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/protocol.cc"
    "${CMAKE_CURRENT_LIST_DIR}/protocol.hh"
    "${CMAKE_CURRENT_LIST_DIR}/splash.cc"
    "${CMAKE_CURRENT_LIST_DIR}/splash.hh"
    "${CMAKE_CURRENT_LIST_DIR}/threading.cc"
    "${CMAKE_CURRENT_LIST_DIR}/threading.hh"
    "${CMAKE_CURRENT_LIST_DIR}/types.hh")
target_compile_features(shared PUBLIC cxx_std_20)
target_include_directories(shared PRIVATE "${PROJECT_SOURCE_DIR}")
target_include_directories(shared PRIVATE "${PROJECT_SOURCE_DIR}/game")
target_precompile_headers(shared PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_link_libraries(shared PUBLIC core enet entt FNL miniz parson thread_pool)

add_subdirectory(entity)
add_subdirectory(world)
