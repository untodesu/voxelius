add_library(deps_uulog STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/uulog.hh"
    "${CMAKE_CURRENT_LIST_DIR}/src/uulog.cc")
target_compile_features(deps_uulog PUBLIC cxx_std_20)
target_include_directories(deps_uulog PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::uulog ALIAS deps_uulog)
