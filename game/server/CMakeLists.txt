add_executable(vserver
    "${CMAKE_CURRENT_LIST_DIR}/chat.cc"
    "${CMAKE_CURRENT_LIST_DIR}/chat.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/main.cc"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/receive.cc"
    "${CMAKE_CURRENT_LIST_DIR}/receive.hh"
    "${CMAKE_CURRENT_LIST_DIR}/sessions.cc"
    "${CMAKE_CURRENT_LIST_DIR}/sessions.hh"
    "${CMAKE_CURRENT_LIST_DIR}/status.cc"
    "${CMAKE_CURRENT_LIST_DIR}/status.hh"
    "${CMAKE_CURRENT_LIST_DIR}/whitelist.cc"
    "${CMAKE_CURRENT_LIST_DIR}/whitelist.hh")
target_compile_features(vserver PUBLIC cxx_std_20)
target_include_directories(vserver PRIVATE "${PROJECT_SOURCE_DIR}")
target_include_directories(vserver PRIVATE "${PROJECT_SOURCE_DIR}/game")
target_precompile_headers(vserver PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_link_libraries(vserver PUBLIC shared)

add_subdirectory(world)

if(WIN32)
    enable_language(RC)
    target_sources(vserver PRIVATE "${CMAKE_CURRENT_LIST_DIR}/vserver.rc")
endif()

install(TARGETS vserver RUNTIME DESTINATION ".")
