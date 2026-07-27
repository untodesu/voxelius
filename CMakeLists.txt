cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(Voxelius LANGUAGES C CXX VERSION 17.0.0)

option(ENABLE_PROFILING "Enable profiling" ON)

option(BUILD_CLIENT "Build game client" ON)
option(BUILD_SERVER "Build game server" ON)

if(NOT BUILD_CLIENT AND NOT BUILD_SERVER)
    message(FATAL_ERROR "Neither BUILD_CLIENT nor BUILD_SERVER is enabled. I don't know what to do!!")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

if(MSVC)
    add_compile_options(/MP)
    add_compile_options(/W3)
endif()

if(ENABLE_PROFILING)
    add_compile_definitions(TRACY_ENABLE)
    add_compile_definitions(TRACY_ON_DEMAND)
    add_compile_definitions(TRACY_NO_FRAME_IMAGE)
endif()

execute_process(COMMAND git rev-parse --short=8 HEAD WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
execute_process(COMMAND git rev-parse --abbrev-ref HEAD WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

add_subdirectory(core)
add_subdirectory(deps)
add_subdirectory(shared)

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(BUILD_SERVER)
    add_subdirectory(server)
endif()

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "doc/voxelius")

set(CPACK_PACKAGE_NAME "Voxelius")
set(CPACK_PACKAGE_VENDOR "untodesu")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}-${GIT_BRANCH}-${GIT_COMMIT}")
set(CPACK_PACKAGE_DIRECTORY "${PROJECT_SOURCE_DIR}/dist")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")

set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}\\\\media\\\\nsis-installer.ico")
set(CPACK_NSIS_MUI_HEADERIMAGE "${PROJECT_SOURCE_DIR}\\\\media\\\\nsis-header.bmp")
set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_SOURCE_DIR}\\\\media\\\\nsis-splash.bmp")

unset(CPACK_NSIS_MENU_LINKS)
list(APPEND CPACK_NSIS_MENU_LINKS "bin/voxelius-client.exe" "Voxelius")
list(APPEND CPACK_NSIS_MENU_LINKS "bin/voxelius-server.exe" "Voxelius server")

include(CPack)
