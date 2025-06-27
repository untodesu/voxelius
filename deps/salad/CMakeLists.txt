add_library(salad STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/AL/al.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/AL/alc.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/AL/efx.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/AL/salad.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/salad.c")
target_include_directories(salad PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(salad PROPERTIES FOLDER Dependencies)
target_compile_definitions(salad PRIVATE SALAD_PARANOID)
target_link_libraries(salad PUBLIC ${CMAKE_DL_LIBS})
