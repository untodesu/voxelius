add_library(glad STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/glad/gl.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/KHR/khrplatform.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/gl.c")
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(glad PROPERTIES FOLDER Dependencies)
