add_library(deps_stb STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/stb_image.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/stb_image_write.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/stb_impl.c")
target_compile_definitions(deps_stb PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(deps_stb PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::stb ALIAS deps_stb)
