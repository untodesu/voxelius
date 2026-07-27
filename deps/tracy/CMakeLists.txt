add_library(deps_tracy STATIC
    "${CMAKE_CURRENT_LIST_DIR}/public/tracy/Tracy.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/public/TracyClient.cpp")
target_compile_features(deps_tracy PUBLIC cxx_std_20)
target_include_directories(deps_tracy PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public/tracy")

if(WIN32)
    target_link_libraries(deps_tracy PUBLIC ws2_32 dbghelp advapi32)
endif()

add_library(deps::tracy ALIAS deps_tracy)
