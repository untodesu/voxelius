add_executable(vserver
    "${CMAKE_CURRENT_LIST_DIR}/world/climate.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/climate.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/climate_noise.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/climate_noise.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/entropy_cache.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/entropy_cache.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/feature_placer.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/feature_placer.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/feature_scatter.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/feature_scatter.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/heightmap.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/heightmap.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/noise_cache_2D.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/noise_cache_2D.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/noise_cache_3D.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/noise_cache_3D.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/realm_sky.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/realm_sky.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/realm_surface.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/realm_surface.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/terrain.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/terrain.hh"
    "${CMAKE_CURRENT_LIST_DIR}/world/worldgen.cc"
    "${CMAKE_CURRENT_LIST_DIR}/world/worldgen.hh"
    "${CMAKE_CURRENT_LIST_DIR}/constant.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/main.cc"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_compile_features(vserver PUBLIC cxx_std_20)
target_include_directories(vserver PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(vserver PUBLIC core shared)
target_link_libraries(vserver PUBLIC
    deps::FNL)
target_precompile_headers(vserver PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")

install(TARGETS vserver RUNTIME DESTINATION ".")
