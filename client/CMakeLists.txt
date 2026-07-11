include("${PROJECT_SOURCE_DIR}/cmake/embed-cxx-spv.cmake")

add_executable(voxelius-client
    "${CMAKE_CURRENT_LIST_DIR}/gui/fonts.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gui/fonts.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gui/gui.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gui/gui.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gui/language.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gui/language.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gui/style.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gui/style.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/buffer.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/buffer.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/handle.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/ring.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/ring.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/sampler.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/sampler.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/texture.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/texture.hh"
    "${CMAKE_CURRENT_LIST_DIR}/res/texture2D.cc"
    "${CMAKE_CURRENT_LIST_DIR}/res/texture2D.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/entity.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/entity.hh"
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
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/head.cc"
    "${CMAKE_CURRENT_LIST_DIR}/head.hh"
    "${CMAKE_CURRENT_LIST_DIR}/main.cc"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/video.cc"
    "${CMAKE_CURRENT_LIST_DIR}/video.hh")
target_compile_features(voxelius-client PUBLIC cxx_std_20)
target_include_directories(voxelius-client PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(voxelius-client PUBLIC core shared)
target_link_libraries(voxelius-client PUBLIC
    deps::imgui
    deps::imgui-backend-sdl3
    deps::imgui-backend-sdlgpu3
    deps::SDL3)
target_precompile_headers(voxelius-client PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")

compile_spirv_shader(voxelius-client "${CMAKE_CURRENT_LIST_DIR}/hlsl/chunk.vert.hlsl")
compile_spirv_shader(voxelius-client "${CMAKE_CURRENT_LIST_DIR}/hlsl/chunk.frag.hlsl")
