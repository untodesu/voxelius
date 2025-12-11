add_library(FNL STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/fastnoiselite.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/fastnoiselite.c")
target_compile_definitions(FNL PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(FNL PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(FNL PROPERTIES FOLDER Dependencies)
