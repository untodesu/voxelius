add_executable(voxelius-client
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/main.cc"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_compile_features(voxelius-client PUBLIC cxx_std_20)
target_include_directories(voxelius-client PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(voxelius-client PUBLIC core shared)
target_link_libraries(voxelius-client PUBLIC
    deps::imgui
    deps::imgui-backend-sdl3
    deps::imgui-backend-sdlgpu3
    deps::SDL3)
target_precompile_headers(voxelius-client PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
