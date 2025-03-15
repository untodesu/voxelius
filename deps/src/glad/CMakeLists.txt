add_library(glad STATIC "${CMAKE_CURRENT_LIST_DIR}/gl.c")
target_include_directories(glad PUBLIC "${DEPS_INCLUDE_DIR}")
set_target_properties(glad PROPERTIES FOLDER DEPS)
