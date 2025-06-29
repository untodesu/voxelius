cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(Voxelius LANGUAGES C CXX VERSION 0.0.1.2526)

## Declare build options
option(BUILD_VCLIENT "Build Voxelius client" ON)
option(BUILD_VSERVER "Build Voxelius server" ON)

## If possible, enable solution directories; this allows
## built-in pseudotargets like ALL_BUILD and ZERO_CHECK to
## be moved out of sight into a separate directory
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

## Ensure we are never building dependencies as dynamic
## libraries; we only use dynamic libraries for non-compiled
## third-party dependencies (such as SDL3 and maybe Discord-RPC);
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

## Microsoft-vendored STL has been permissively licensed for quite
## a while, so it makes all the sense to statically link with it to
## avoid pulling redistributable installers alongside the game package
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

## Output binaries into build root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

## Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(deps)
add_subdirectory(src)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(thirdpartylegalnotices ALL
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/thirdpartylegalnotices.py" "${CMAKE_CURRENT_LIST_DIR}/deps"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/thirdpartylegalnotices.txt"
    DEPENDS "${CMAKE_CURRENT_LIST_DIR}/thirdpartylegalnotices.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/thirdpartylegalnotices.txt" DESTINATION ".")

install(FILES "${CMAKE_CURRENT_LIST_DIR}/LICENSE" DESTINATION ".")
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/data" DESTINATION ".")

set(CPACK_PACKAGE_NAME "Voxelius")
set(CPACK_PACKAGE_VENDOR "untodesu")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Block game on steroids")
set(CPACK_PACKAGE_CONTACT "https://github.com/untodesu/voxelius/issues")

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION_TWEAK ${PROJECT_VERSION_TWEAK})

set(CPACK_PACKAGE_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")

include(CPack)
