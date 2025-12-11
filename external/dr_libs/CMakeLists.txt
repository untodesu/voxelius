add_library(dr_libs STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/dr_mp3.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/dr_wav.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/dr_impl.c")
target_compile_definitions(dr_libs PRIVATE _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(dr_libs PUBLIC DR_WAV_NO_STDIO DR_WAV_NO_WCHAR)
target_compile_definitions(dr_libs PUBLIC DR_MP3_NO_STDIO)
target_include_directories(dr_libs PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(dr_libs PROPERTIES FOLDER Dependencies)
