add_executable(voxelius-client
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
    "${CMAKE_CURRENT_LIST_DIR}/constant.hh"
    "${CMAKE_CURRENT_LIST_DIR}/frame.cc"
    "${CMAKE_CURRENT_LIST_DIR}/frame.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
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
