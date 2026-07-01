add_library(deps_imgui STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/imconfig.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_internal.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_stdlib.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imstb_rectpack.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imstb_textedit.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imstb_truetype.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_demo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_draw.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_stdlib.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_tables.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_widgets.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui.cpp")
target_include_directories(deps_imgui PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(deps_imgui PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps_imgui_sdl3 STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_impl_sdl3.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_impl_sdl3.cpp")
target_link_libraries(deps_imgui_sdl3 PUBLIC deps_imgui)
target_link_libraries(deps_imgui_sdl3 PUBLIC deps_SDL3)

add_library(deps_imgui_sdlgpu3 STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_impl_sdlgpu3.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_impl_sdlgpu3_shaders.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_impl_sdlgpu3.cpp")
target_link_libraries(deps_imgui_sdlgpu3 PUBLIC deps_imgui)
target_link_libraries(deps_imgui_sdlgpu3 PUBLIC deps_SDL3)

add_library(deps::imgui ALIAS deps_imgui)
add_library(deps::imgui-backend-sdl3 ALIAS deps_imgui_sdl3)
add_library(deps::imgui-backend-sdlgpu3 ALIAS deps_imgui_sdlgpu3)
