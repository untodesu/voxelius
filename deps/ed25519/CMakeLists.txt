add_library(deps_ed25519 STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/ed25519.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/add_scalar.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/fe.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/fe.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/fixedint.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/ge.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ge.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/key_exchange.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/keypair.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/precomp_data.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/sc.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/sc.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/seed.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/sha512.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/sha512.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/sign.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/verify.c")
target_include_directories(deps_ed25519 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(deps::ed25519 ALIAS deps_ed25519)
