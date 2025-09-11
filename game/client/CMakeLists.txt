add_executable(vclient
    "${CMAKE_CURRENT_LIST_DIR}/const.hh"
    "${CMAKE_CURRENT_LIST_DIR}/experiments.cc"
    "${CMAKE_CURRENT_LIST_DIR}/experiments.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/main.cc"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/program.cc"
    "${CMAKE_CURRENT_LIST_DIR}/program.hh"
    "${CMAKE_CURRENT_LIST_DIR}/receive.cc"
    "${CMAKE_CURRENT_LIST_DIR}/receive.hh"
    "${CMAKE_CURRENT_LIST_DIR}/screenshot.cc"
    "${CMAKE_CURRENT_LIST_DIR}/screenshot.hh"
    "${CMAKE_CURRENT_LIST_DIR}/session.cc"
    "${CMAKE_CURRENT_LIST_DIR}/session.hh"
    "${CMAKE_CURRENT_LIST_DIR}/toggles.cc"
    "${CMAKE_CURRENT_LIST_DIR}/toggles.hh")
target_compile_features(vclient PUBLIC cxx_std_20)
target_compile_definitions(vclient PUBLIC GLFW_INCLUDE_NONE)
target_include_directories(vclient PRIVATE "${PROJECT_SOURCE_DIR}")
target_include_directories(vclient PRIVATE "${PROJECT_SOURCE_DIR}/game")
target_precompile_headers(vclient PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_link_libraries(vclient PUBLIC shared dr_libs glad glfw imgui imgui_glfw imgui_opengl3 salad)

add_subdirectory(config)
add_subdirectory(entity)
add_subdirectory(gui)
add_subdirectory(io)
add_subdirectory(resource)
add_subdirectory(sound)
add_subdirectory(world)

if(WIN32 AND MSVC)
    # GLFW defines APIENTRY and ENet includes
    # Windows API headers which also define APIENTRY
    target_compile_options(vclient PRIVATE /wd4005)
endif()

if(WIN32)
    enable_language(RC)
    target_sources(vclient PRIVATE "${CMAKE_CURRENT_LIST_DIR}/vclient.rc")
endif()

install(TARGETS vclient RUNTIME DESTINATION ".")
