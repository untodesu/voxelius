add_executable(voxelius-client
    "${CMAKE_CURRENT_LIST_DIR}/res/texture2D.cc"
    "${CMAKE_CURRENT_LIST_DIR}/res/texture2D.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/entity.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/entity.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/imgui.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/imgui.hh"
    "${CMAKE_CURRENT_LIST_DIR}/background.cc"
    "${CMAKE_CURRENT_LIST_DIR}/background.hh"
    "${CMAKE_CURRENT_LIST_DIR}/block_atlas.cc"
    "${CMAKE_CURRENT_LIST_DIR}/block_atlas.hh"
    "${CMAKE_CURRENT_LIST_DIR}/block_models.cc"
    "${CMAKE_CURRENT_LIST_DIR}/block_models.hh"
    "${CMAKE_CURRENT_LIST_DIR}/camera.cc"
    "${CMAKE_CURRENT_LIST_DIR}/camera.hh"
    "${CMAKE_CURRENT_LIST_DIR}/chunk_mesh.cc"
    "${CMAKE_CURRENT_LIST_DIR}/chunk_mesh.hh"
    "${CMAKE_CURRENT_LIST_DIR}/chunk_mesher.cc"
    "${CMAKE_CURRENT_LIST_DIR}/chunk_mesher.hh"
    "${CMAKE_CURRENT_LIST_DIR}/chunk_renderer.cc"
    "${CMAKE_CURRENT_LIST_DIR}/chunk_renderer.hh"
    "${CMAKE_CURRENT_LIST_DIR}/constant.hh"
    "${CMAKE_CURRENT_LIST_DIR}/fonts.cc"
    "${CMAKE_CURRENT_LIST_DIR}/fonts.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gui.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gui.hh"
    "${CMAKE_CURRENT_LIST_DIR}/head.cc"
    "${CMAKE_CURRENT_LIST_DIR}/head.hh"
    "${CMAKE_CURRENT_LIST_DIR}/interpolation.cc"
    "${CMAKE_CURRENT_LIST_DIR}/interpolation.hh"
    "${CMAKE_CURRENT_LIST_DIR}/language.cc"
    "${CMAKE_CURRENT_LIST_DIR}/language.hh"
    "${CMAKE_CURRENT_LIST_DIR}/main_menu.cc"
    "${CMAKE_CURRENT_LIST_DIR}/main_menu.hh"
    "${CMAKE_CURRENT_LIST_DIR}/outline.cc"
    "${CMAKE_CURRENT_LIST_DIR}/outline.hh"
    "${CMAKE_CURRENT_LIST_DIR}/main.cc"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/player_look.cc"
    "${CMAKE_CURRENT_LIST_DIR}/player_look.hh"
    "${CMAKE_CURRENT_LIST_DIR}/player_move.cc"
    "${CMAKE_CURRENT_LIST_DIR}/player_move.hh"
    "${CMAKE_CURRENT_LIST_DIR}/player_target.cc"
    "${CMAKE_CURRENT_LIST_DIR}/player_target.hh"
    "${CMAKE_CURRENT_LIST_DIR}/settings.cc"
    "${CMAKE_CURRENT_LIST_DIR}/settings.hh"
    "${CMAKE_CURRENT_LIST_DIR}/shader_program.cc"
    "${CMAKE_CURRENT_LIST_DIR}/shader_program.hh"
    "${CMAKE_CURRENT_LIST_DIR}/skybox.cc"
    "${CMAKE_CURRENT_LIST_DIR}/skybox.hh"
    "${CMAKE_CURRENT_LIST_DIR}/style.cc"
    "${CMAKE_CURRENT_LIST_DIR}/style.hh"
    "${CMAKE_CURRENT_LIST_DIR}/video.cc"
    "${CMAKE_CURRENT_LIST_DIR}/video.hh")
target_compile_features(voxelius-client PUBLIC cxx_std_20)
target_include_directories(voxelius-client PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(voxelius-client PUBLIC core shared)
target_link_libraries(voxelius-client PUBLIC
    deps::imgui
    deps::imgui-backend-sdl3
    deps::imgui-backend-opengl3
    deps::SDL3
    deps::glad)
target_precompile_headers(voxelius-client PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")

target_link_libraries(voxelius-client PUBLIC deps::FNL) # temporary
