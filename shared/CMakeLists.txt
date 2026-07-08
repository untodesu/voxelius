add_library(shared STATIC
    "${CMAKE_CURRENT_LIST_DIR}/scripting/blocks_library.cc"
    "${CMAKE_CURRENT_LIST_DIR}/scripting/blocks_library.hh"
    "${CMAKE_CURRENT_LIST_DIR}/scripting/core_library.cc"
    "${CMAKE_CURRENT_LIST_DIR}/scripting/core_library.hh"
    "${CMAKE_CURRENT_LIST_DIR}/scripting/lua_libraries.cc"
    "${CMAKE_CURRENT_LIST_DIR}/scripting/lua_libraries.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/coord.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/coord.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/lua.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/lua.hh"
    "${CMAKE_CURRENT_LIST_DIR}/block.hh"
    "${CMAKE_CURRENT_LIST_DIR}/block_registry.cc"
    "${CMAKE_CURRENT_LIST_DIR}/block_registry.hh"
    "${CMAKE_CURRENT_LIST_DIR}/block_storage.cc"
    "${CMAKE_CURRENT_LIST_DIR}/block_storage.hh"
    "${CMAKE_CURRENT_LIST_DIR}/chunk.cc"
    "${CMAKE_CURRENT_LIST_DIR}/chunk.hh"
    "${CMAKE_CURRENT_LIST_DIR}/constant.hh"
    "${CMAKE_CURRENT_LIST_DIR}/coord.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/mod_context.cc"
    "${CMAKE_CURRENT_LIST_DIR}/mod_context.hh"
    "${CMAKE_CURRENT_LIST_DIR}/mod_loader.cc"
    "${CMAKE_CURRENT_LIST_DIR}/mod_loader.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_compile_features(shared PUBLIC cxx_std_20)
target_include_directories(shared PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(shared PUBLIC core)
target_link_libraries(shared PUBLIC
    deps::entt
    deps::lua)
target_precompile_headers(shared PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
