add_library(miniz STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/miniz.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/miniz.c")
target_compile_definitions(miniz PUBLIC MINIZ_NO_STDIO MINIZ_NO_ZLIB_COMPATIBLE_NAMES)
target_include_directories(miniz PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(miniz PROPERTIES FOLDER Dependencies)
