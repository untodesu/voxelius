add_library(core STATIC
    "${CMAKE_CURRENT_LIST_DIR}/aabb.hh"
    "${CMAKE_CURRENT_LIST_DIR}/aabb.cc"
    "${CMAKE_CURRENT_LIST_DIR}/binfile.hh"
    "${CMAKE_CURRENT_LIST_DIR}/binfile.cc"
    "${CMAKE_CURRENT_LIST_DIR}/buffer.hh"
    "${CMAKE_CURRENT_LIST_DIR}/buffer.cc"
    "${CMAKE_CURRENT_LIST_DIR}/cmdline.hh"
    "${CMAKE_CURRENT_LIST_DIR}/cmdline.cc"
    "${CMAKE_CURRENT_LIST_DIR}/concepts.hh"
    "${CMAKE_CURRENT_LIST_DIR}/config.cc"
    "${CMAKE_CURRENT_LIST_DIR}/config.hh"
    "${CMAKE_CURRENT_LIST_DIR}/constexpr.hh"
    "${CMAKE_CURRENT_LIST_DIR}/crc64.cc"
    "${CMAKE_CURRENT_LIST_DIR}/crc64.hh"
    "${CMAKE_CURRENT_LIST_DIR}/epoch.cc"
    "${CMAKE_CURRENT_LIST_DIR}/epoch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/floathacks.hh"
    "${CMAKE_CURRENT_LIST_DIR}/image.cc"
    "${CMAKE_CURRENT_LIST_DIR}/image.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/resource.hh"
    "${CMAKE_CURRENT_LIST_DIR}/strtools.cc"
    "${CMAKE_CURRENT_LIST_DIR}/strtools.hh"
    "${CMAKE_CURRENT_LIST_DIR}/version.cc"
    "${CMAKE_CURRENT_LIST_DIR}/version.hh")
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC "${DEPS_INCLUDE_DIR}")
target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}")
target_precompile_headers(core PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_link_libraries(core PUBLIC enet emhash glm physfs spdlog stb)

if(WIN32)
    target_compile_definitions(core PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(core PUBLIC _USE_MATH_DEFINES)
    target_compile_definitions(core PUBLIC NOMINMAX)
endif()

if(MSVC)
    target_compile_options(core PUBLIC /utf-8)
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/version.cc.in" "${CMAKE_CURRENT_LIST_DIR}/version.cc")
