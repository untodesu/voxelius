option(BUILD_CLIENT "Build client" ON)
option(BUILD_SERVER "Build server" OFF)

if(BUILD_CLIENT)
    add_executable(vgame "${CMAKE_CURRENT_LIST_DIR}/main.cc")
    target_compile_definitions(vgame PUBLIC VGAME_CLIENT)
    target_include_directories(vgame PUBLIC "${CMAKE_SOURCE_DIR}")
    target_link_libraries(vgame PUBLIC client physfs spdlog)
endif()

if(BUILD_SERVER)
    add_executable(vds "${CMAKE_CURRENT_LIST_DIR}/main.cc")
    target_compile_definitions(vds PUBLIC VGAME_SERVER)
    target_include_directories(vds PUBLIC "${CMAKE_SOURCE_DIR}")
    target_link_libraries(vds PUBLIC server physfs spdlog)
endif()
