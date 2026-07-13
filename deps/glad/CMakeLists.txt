add_library(deps_glad STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/glad/gl.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/KHR/khrplatform.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/gl.c")
target_include_directories(deps_glad PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(deps_glad PROPERTIES FOLDER Dependencies)

add_library(deps::glad ALIAS deps_glad)
