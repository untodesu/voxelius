add_library(deps_thread_pool INTERFACE)
target_include_directories(deps_thread_pool INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::thread_pool ALIAS deps_thread_pool)
