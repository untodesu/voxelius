add_library(parson STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/parson.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/parson.c")
target_include_directories(parson PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(parson PROPERTIES FOLDER Dependencies)
