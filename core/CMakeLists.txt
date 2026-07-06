add_library(core STATIC
    "${CMAKE_CURRENT_LIST_DIR}/config/map.cc"
    "${CMAKE_CURRENT_LIST_DIR}/config/map.hh"
    "${CMAKE_CURRENT_LIST_DIR}/config/ref.hh"
    "${CMAKE_CURRENT_LIST_DIR}/res/image.cc"
    "${CMAKE_CURRENT_LIST_DIR}/res/image.hh"
    "${CMAKE_CURRENT_LIST_DIR}/res/resource.cc"
    "${CMAKE_CURRENT_LIST_DIR}/res/resource.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/angles.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/angles.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/crc64.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/crc64.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/epoch.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/epoch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/json.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/json.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/modulo.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/physfs.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/physfs.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/raycast.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/raycast.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/string.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/string.hh"
    "${CMAKE_CURRENT_LIST_DIR}/buffer.cc"
    "${CMAKE_CURRENT_LIST_DIR}/buffer.hh"
    "${CMAKE_CURRENT_LIST_DIR}/camera.cc"
    "${CMAKE_CURRENT_LIST_DIR}/camera.hh"
    "${CMAKE_CURRENT_LIST_DIR}/cmdline.cc"
    "${CMAKE_CURRENT_LIST_DIR}/cmdline.hh"
    "${CMAKE_CURRENT_LIST_DIR}/concepts.hh"
    "${CMAKE_CURRENT_LIST_DIR}/constant.hh"
    "${CMAKE_CURRENT_LIST_DIR}/core.cc"
    "${CMAKE_CURRENT_LIST_DIR}/core.hh"
    "${CMAKE_CURRENT_LIST_DIR}/exception.cc"
    "${CMAKE_CURRENT_LIST_DIR}/exception.hh"
    "${CMAKE_CURRENT_LIST_DIR}/frustum.cc"
    "${CMAKE_CURRENT_LIST_DIR}/frustum.hh"
    "${CMAKE_CURRENT_LIST_DIR}/identifier.cc"
    "${CMAKE_CURRENT_LIST_DIR}/identifier.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/version.hh")
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(core PUBLIC
    deps::eigen
    deps::enet
    deps::parson
    deps::physfs
    deps::stb
    deps::thread_pool
    deps::uulog)
target_precompile_headers(core PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")

configure_file("${CMAKE_CURRENT_LIST_DIR}/version.cc.in" "${PROJECT_BINARY_DIR}/core.generated_version.cc")
target_sources(core PRIVATE "${PROJECT_BINARY_DIR}/core.generated_version.cc")

if(WIN32)
    target_compile_definitions(core PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(core PUBLIC _USE_MATH_DEFINES)
    target_compile_definitions(core PUBLIC NOMINMAX)
endif()
