cmake_minimum_required(VERSION 3.15)
project(VOXELIUS LANGUAGES CXX VERSION 0.0.1)
set(VOXELIUS_VERSION_META "DEV.2024-10")

set(BUILD_CLIENT ON CACHE BOOL "Build client executable")
set(BUILD_SERVER ON CACHE BOOL "Build server executable")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_FORCE_FETCHCONTENT OFF CACHE BOOL "Force FetchContent for GLFW")
set(PHYSFS_ARCHIVE_ZIP ON CACHE BOOL "" FORCE)

if(BUILD_CLIENT)
    add_subdirectory(external/glad)
    add_subdirectory(external/imgui)
    
    if(NOT GLFW_FORCE_FETCHCONTENT)
        find_package(glfw3 QUIET)
    endif()

    if(NOT glfw3_FOUND OR GLFW_FORCE_FETCHCONTENT)
        include(FetchContent)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(glfw3
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE)
        FetchContent_MakeAvailable(glfw3)
    endif()
endif()

add_subdirectory(external/emhash)
add_subdirectory(external/enet)
add_subdirectory(external/entt)
add_subdirectory(external/fastnoiselite)
add_subdirectory(external/miniz)
add_subdirectory(external/parson)
add_subdirectory(external/physfs)
add_subdirectory(external/spdlog)
add_subdirectory(external/stb)
add_subdirectory(external/thread_pool)

add_subdirectory(common)
add_subdirectory(config)
add_subdirectory(mathlib)

if(BUILD_CLIENT)
    add_subdirectory(game/client)
endif()

add_subdirectory(game/server)
add_subdirectory(game/shared)
add_subdirectory(launch)

set(CPACK_PACKAGE_NAME "Voxelius")
set(CPACK_PACKAGE_VENDOR "Voxelius Contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Block game on steroids")
set(CPACK_PACKAGE_CONTACT "https://bsky.app/profile/at.untode.su")

set(CPACK_PACKAGE_VERSION "${VOXELIUS_VERSION}-${VOXELIUS_VERSION_META}")
set(CPACK_PACKAGE_VERSION_MAJOR "${VOXELIUS_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VOXELIUS_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VOXELIUS_VERSION_PATCH}")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if(WIN32)
    # For Windows packages, assets should be placed in the
    # same directory as the client and server executables since
    # launch.cc, as a last resort, pulls data from current_directory()
    install(TARGETS vgame RUNTIME DESTINATION ".")
    install(TARGETS vgame-server RUNTIME DESTINATION ".")
    install(DIRECTORY assets/ DESTINATION ".")

    # Configure NSIS
    # UNDONE: make a dedicated server icon and actually
    # create a dedicated server executable in the start menu
    set(CPACK_GENERATOR NSIS)
    set(CPACK_NSIS_INSTALLED_ICON_NAME "vgame.exe")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/launch/resource/server_icon_1024x1024.ico")
    list(APPEND CPACK_NSIS_MENU_LINKS "vgame-server.exe" "${CPACK_PACKAGE_NAMAE} Server")
    list(APPEND CPACK_NSIS_MENU_LINKS "vgame.exe" "${CPACK_PACKAGE_NAME}")
elseif(UNIX AND NOT APPLE)
    # For Linux packages, assets should be placed in the FHS-compliant
    # directory /usr/share/voxelius; again, when launch.cc is compiled
    # for Linux, it will look for assets in that specific directory
    install(TARGETS vgame RUNTIME DESTINATION share/bin)
    install(TARGETS vgame-server RUNTIME DESTINATION share/bin)
    install(DIRECTORY assets/ DESTINATION share/voxelius)

    if(glfw3_FOUND AND NOT GLFW_FORCE_FETCHCONTENT)
        # We're building using system GLFW distribution
        # so we must include it as a dependency; this is really
        # the only external library we might link to dynamically
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libglfw3 (>= 3.3.8)")
    endif()

    # Configure DEB
    set(CPACK_GENERATOR DEB)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
endif()

include(CPack)
